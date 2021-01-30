#pragma once

#include <mutex>
#include <queue>
#include <vector>
#include <memory>

class HttpData;

class TimerNode : public std::enable_shared_from_this<TimerNode> {
public:
  TimerNode(std::shared_ptr<HttpData> http_data, size_t timeout);
  ~TimerNode();

  bool is_deleted() const {return deleted_;}

  size_t get_expired_time() {return expired_time_;}

  bool is_expire() {return expired_time_ < current_msec;}

  void deleted();

  std::shared_ptr<HttpData> getHttpData() {return http_data_;}

  static void current_time();

  static size_t current_msec;

private:
  bool deleted_;
  size_t expired_time_;
  std::shared_ptr<HttpData> http_data_;
};

struct TimerCmp {
  bool operator()(const std::shared_ptr<TimerNode>& a, const std::shared_ptr<TimerNode>& b) const {
    return a->get_expired_time() > b->get_expired_time();
  }
};

class TimerManager {
public:
  typedef std::shared_ptr<TimerNode> TimerNodePtr;

  void addTimer(std::shared_ptr<HttpData> http_data, size_t timeout);

  void handleExpiredEvent();

  static const size_t DEFAULT_TIME_OUT;

private:

  std::priority_queue<TimerNodePtr, std::vector<TimerNodePtr>, TimerCmp> timer_queue_;
  
  std::mutex heap_mutex_;
};