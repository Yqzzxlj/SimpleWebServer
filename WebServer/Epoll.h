#pragma once
#include "Timer.h"

#include <sys/epoll.h>
#include <memory>
#include <vector>

class Channel;

namespace http {
  class HttpData;
} // namespace http

class Epoll {

 public:
  Epoll();
  ~Epoll();

  int getEpollFd() { return epollFd_; }

  void epoll_add(std::shared_ptr<Channel> channel, int timeout);
  void epoll_mod(std::shared_ptr<Channel> channel, int timeout);
  void epoll_del(std::shared_ptr<Channel> channel);
  std::vector<std::shared_ptr<Channel>> poll();

  void addTimer(std::shared_ptr<Channel> channel, int timeout);
  void hanleExpired();

 private:
  static const int MAXFDS = 100000;
  int epollFd_;

  std::vector<struct epoll_event> events; // epoll_wait传回的活跃事件。
  std::shared_ptr<Channel> fd2chann_[MAXFDS];
  TimerManager timerManager_;
};