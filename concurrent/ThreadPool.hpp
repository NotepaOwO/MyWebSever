/**
 * @file ThreadPool.hpp
 * @brief 线程池实现 - 避免"每连接一线程"问题
 *
 * 提供固定数量的工作线程，异步执行提交的任务。
 * 支持任务的提交和结果获取，提高并发性能。
 *
 * 设计模式：
 * - 生产者-消费者模式：任务队列和工作线程
 * - 命令模式：封装任务为可执行对象
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

/**
 * @brief 线程池类
 *
 * 管理固定数量的工作线程，提供任务提交接口。
 * 自动分配任务到空闲线程执行，提高资源利用率。
 */
class ThreadPool {
public:
    /**
     * @brief 构造函数
     *
     * 创建指定数量的工作线程，所有线程开始等待任务
     *
     * @param num_threads 工作线程数量
     */
    explicit ThreadPool(size_t num_threads);

    /**
     * @brief 析构函数
     *
     * 停止所有工作线程，等待任务完成
     */
    ~ThreadPool();

    /**
     * @brief 提交任务到线程池
     *
     * 将可调用对象打包成任务，异步执行并返回结果
     *
     * @tparam F 可调用对象类型（函数、lambda等）
     * @tparam Args 参数类型
     * @param f 可调用对象
     * @param args 函数参数
     * @return std::future<typename std::result_of<F(Args...)>::type>
     *         任务执行结果的future对象
     */
    template<class F, class... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    /**
     * @brief 停止线程池
     *
     * 通知所有工作线程停止，等待当前任务完成
     */
    void shutdown();

private:
    std::vector<std::thread> workers_;              ///< 工作线程集合
    std::queue<std::function<void()>> tasks_;       ///< 任务队列

    std::mutex queue_mutex_;                        ///< 队列互斥锁
    std::condition_variable condition_;             ///< 条件变量（任务通知）
    bool stop_;                                     ///< 停止标志
};

// 模板函数定义必须在头文件中
template<class F, class... Args>
auto ThreadPool::submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    // 将任务打包成packaged_task，便于获取结果
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();

    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("submit on stopped ThreadPool");
        }
        // 将任务添加到队列
        tasks_.emplace([task]() { (*task)(); });
    }

    // 通知一个等待的工作线程
    condition_.notify_one();
    return res;
}

#endif // THREAD_POOL_HPP