#include "HttpData.h"
#include "../Timer.h"
#include "../log/Logging.h"

void HttpData::close_timer() {
  if (timer_.lock()) {
    LOG_DEBUG << "Closing timer";
    std::shared_ptr<TimerNode> temp_timer(timer_.lock());
    temp_timer->deleted();
    timer_.reset();
  }
}

void HttpData::set_timer(std::shared_ptr<TimerNode> timer) {
  timer_ = timer;
}