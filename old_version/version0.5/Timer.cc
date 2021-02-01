#include "Timer.h"
#include "Epoll.h"
#include "HttpData.h"

#include <sys/time.h>
#include <unistd.h>
#include <iostream>

// TODO 使用weak_ptr
size_t TimerNode::current_msec = 0; // 当前时间
const size_t TimerManager::DEFAULT_TIME_OUT = 5 * 1000; // 5s

TimerNode::TimerNode(std::shared_ptr<http::HttpData> http_data, size_t timeout) 
  : __deleted(false), http_data(http_data) {
  current_time();
  expired_time = current_msec + timeout;
}

TimerNode::~TimerNode() {
  if (http_data) {
    auto it = Epoll::http_data_map.find(http_data->client_socket->fd);
    if (it != Epoll::http_data_map.end()) {
      std::cout << "connection " << http_data->client_socket->fd << " expired" << std::endl;
      Epoll::http_data_map.erase(it);
    }
  }
}

inline void TimerNode::current_time() {
  struct timeval cur;
  gettimeofday(&cur, NULL);
  current_msec = (cur.tv_sec * 1000) + (cur.tv_usec / 1000);
}

void TimerNode::deleted() {
  http_data.reset();
  __deleted = true;
}

void TimerManager::add_timer(std::shared_ptr<http::HttpData> http_data, size_t timeout) {
  TimerNodePtr timer_node(new TimerNode(http_data, timeout));
  {
    std::unique_lock<std::mutex> lock;
    timer_queue.push(timer_node);
  }
}

void TimerManager::handle_expired_event() {
  std::unique_lock<std::mutex> lock;
  TimerNode::current_time();
  std::cout << "handing expired event" << std::endl;
  while (!timer_queue.empty()) {
    TimerNodePtr timer_node = timer_queue.top();
    if (timer_node->is_deleted()) {
      timer_queue.pop();
    } else if (timer_node->is_expire()){
      timer_queue.pop();
    } else {
      break;
    }
  }
}