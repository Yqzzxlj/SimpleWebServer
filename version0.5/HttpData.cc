#include "HttpData.h"

void http::HttpData::close_timer() {
  if (timer.lock()) {
    std::shared_ptr<TimerNode> temp_timer(timer.lock());
    temp_timer->deleted();
    timer.reset();
  }
}

void http::HttpData::set_timer(std::shared_ptr<TimerNode> timer) {
  this->timer = timer;
}