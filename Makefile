# Demo version
# all: myhttp

# myhttp: httpd.c
# 	gcc -W -Wall -o myhttp httpd.c -lpthread

# clean:
# 	rm myhttp

# stage 1
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread -lssl -lcrypto
TARGET = myhttp
SRCS = core/main.cpp core/Server.cpp http/HttpHandler.cpp network/Socket.cpp http/HttpRequest.cpp http/HttpResponse.cpp concurrent/ThreadPool.cpp ssl/SSLWrapper.cpp cgi/CGIHandler.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) -lssl -lcrypto

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
