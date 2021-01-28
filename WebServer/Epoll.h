#pragma once

#include <sys/epoll.h>
#include <unordered_map>
#include <memory>
#include <vector>

class HttpData;
class ServerSocket;
class TimerManager;


class Epoll {
public:
  static int init(int max_events);
  static int addfd(int epoll_fd, int fd, __uint32_t events,
                   std::shared_ptr<HttpData>);
  static int modfd(int epoll_fd, int fd, __uint32_t events,
                   std::shared_ptr<HttpData>);
  static int delfd(int epoll_fd, int fd, __uint32_t events);
  static std::vector<std::shared_ptr<HttpData>>
  poll(const ServerSocket& server_socket, int max_event, int timeout);
  static void handleConnection(const ServerSocket& server_socket);

public:
  static const int MAX_EVENTS;
  static struct epoll_event* events_;
  static const __uint32_t DEFAULT_EVENTS;
  static std::unordered_map<int, std::shared_ptr<HttpData>> fd2httpData_;
  static TimerManager timer_manager_;
};