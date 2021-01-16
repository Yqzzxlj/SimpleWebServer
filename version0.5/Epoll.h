#pragma once
#include "HttpData.h"
#include "Timer.h"

#include <sys/epoll.h>
#include <unordered_map>
#include <memory>
#include <vector>

class Epoll {
public:
  static int init(int max_events);
  static int addfd(int epoll_fd, int fd, __uint32_t events,
                   std::shared_ptr<http::HttpData>);
  static int modfd(int epoll_fd, int fd, __uint32_t events,
                   std::shared_ptr<http::HttpData>);
  static int delfd(int epoll_fd, int fd, __uint32_t events);
  static std::vector<std::shared_ptr<http::HttpData>>
  poll(const net::ServerSocket& server_socket, int max_event, int timeout);
  static void handleConnection(const net::ServerSocket& server_socket);

public:
  static const int MAX_EVENTS;
  static struct epoll_event* events;
  static const __uint32_t DEFAULT_EVENTS;
  static std::unordered_map<int, std::shared_ptr<http::HttpData>> http_data_map;
  static TimerManager timer_manager;
};