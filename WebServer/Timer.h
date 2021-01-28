#pragma once
#include "HttpData.h"

#include <mutex>
#include <queue>
#include <vector>
#include <memory>

namespace http {
  class HttpData;
} // namespace http

class TimerNode {
public:
  TimerNode(std::shared_ptr<http::HttpData> http_data, size_t timeout);
  ~TimerNode();

  bool is_deleted() const {return __deleted;}

  size_t get_expired_time() {return expired_time;}

  bool is_expire() {return expired_time < current_msec;}

  void deleted();

  std::shared_ptr<http::HttpData> get_http_data() {return http_data;}

  static void current_time();

  static size_t current_msec;

private:
  bool __deleted;
  size_t expired_time;
  std::shared_ptr<http::HttpData> http_data;
};

struct TimerCmp {
  bool operator()(const std::shared_ptr<TimerNode>& a, const std::shared_ptr<TimerNode>& b) const {
    return a->get_expired_time() > b->get_expired_time();
  }
};

class TimerManager {
public:
  typedef std::shared_ptr<TimerNode> TimerNodePtr;

  void add_timer(std::shared_ptr<http::HttpData> http_data, size_t timeout);

  void handle_expired_event();

  static const size_t DEFAULT_TIME_OUT;

private:

  std::priority_queue<TimerNodePtr, std::vector<TimerNodePtr>, TimerCmp> timer_queue;
  
  std::mutex heap_mutex;
};