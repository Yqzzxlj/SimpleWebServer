#include "HttpData.h"
#include "../Timer.h"

void HttpData::close_timer() {
  if (timer.lock()) {
    std::shared_ptr<TimerNode> temp_timer(timer.lock());
    temp_timer->deleted();
    timer.reset();
  }
}

void HttpData::set_timer(std::shared_ptr<TimerNode> timer) {
  this->timer = timer;
}