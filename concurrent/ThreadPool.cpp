/**
 * @file ThreadPool.cpp
 * @brief 线程池实现 - 工作线程管理
 *
 * 实现线程池的核心逻辑：工作线程创建、任务分发、线程同步。
 * 使用条件变量实现生产者-消费者模式。
 *
 * @author PoorWebServer Team
 * @date 2026-04-24
 * @version 1.0
 */

#include "ThreadPool.hpp"

/**
 * @brief ThreadPool构造函数
 *
 * 创建指定数量的工作线程，每个线程运行工作循环：
 * 1. 等待任务到达（条件变量）
 * 2. 获取任务并执行
 * 3. 重复直到停止
 *
 * @param num_threads 工作线程数量
 */
ThreadPool::ThreadPool(size_t num_threads) : stop_(false) {
    for (size_t i = 0; i < num_threads; ++i) {
        // 为每个工作线程创建执行函数
        workers_.emplace_back([this] {
            while (true) {
                std::function<void()> task;

                {
                    // 获取队列锁，等待任务或停止信号
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    condition_.wait(lock, [this] {
                        return stop_ || !tasks_.empty();
                    });

                    // 如果停止且队列为空，退出线程
                    if (stop_ && tasks_.empty()) {
                        return;
                    }

                    // 从队列获取任务
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }

                // 在锁外执行任务，提高并发性
                task();
            }
        });
    }
}

/**
 * @brief ThreadPool析构函数
 *
 * 停止所有工作线程并等待它们完成当前任务
 */
ThreadPool::~ThreadPool() {
    shutdown();
}

/**
 * @brief 停止线程池
 *
 * 设置停止标志，通知所有等待线程，然后等待线程结束
 */
void ThreadPool::shutdown() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }

    // 通知所有等待的工作线程
    condition_.notify_all();

    // 等待所有工作线程结束
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}