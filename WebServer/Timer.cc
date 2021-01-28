#include "Timer.h"
#include "Epoll.h"
#include "http/HttpData.h"
#include "Socket.h"
#include "log/Logging.h"

#include <sys/time.h>
#include <unistd.h>

size_t TimerNode::current_msec = 0; // 当前时间
const size_t TimerManager::DEFAULT_TIME_OUT = 5 * 1000; // 5s

TimerNode::TimerNode(std::shared_ptr<HttpData> http_data, size_t timeout) 
  : deleted_(false), http_data_(http_data) {
  current_time();
  expired_time_ = current_msec + timeout;
}

TimerNode::~TimerNode() {
  if (http_data_ != nullptr) {
    auto it = Epoll::fd2httpData_.find(http_data_->client_socket_->fd_);
    if (it != Epoll::fd2httpData_.end()) {
      LOG_ERROR << "connection " << http_data_->client_socket_->fd_ << " expired";
      Epoll::fd2httpData_.erase(it);
    }
  }
}

inline void TimerNode::current_time() {
  struct timeval cur;
  gettimeofday(&cur, NULL);
  current_msec = (cur.tv_sec * 1000) + (cur.tv_usec / 1000);
}

void TimerNode::deleted() {
  http_data_.reset();
  deleted_ = true;
}

void TimerManager::addTimer(std::shared_ptr<HttpData> http_data, size_t timeout) {
  TimerNodePtr timer_node(new TimerNode(http_data, timeout));
  {
    std::unique_lock<std::mutex> lock;
    timer_queue_.push(timer_node);
  }
}

void TimerManager::handleExpiredEvent() {
  std::unique_lock<std::mutex> lock;
  TimerNode::current_time();
  LOG_INFO << "handing expired event";
  while (!timer_queue_.empty()) {
    TimerNodePtr timer_node = timer_queue_.top();
    if (timer_node->is_deleted()) {
      timer_queue_.pop();
    } else if (timer_node->is_expire()){
      timer_queue_.pop();
    } else {
      break;
    }
  }
}