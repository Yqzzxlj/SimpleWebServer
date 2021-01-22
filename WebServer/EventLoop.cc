#include "EventLoop.h"

thread_local EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop()
  : looping_(false),
    threadId_(std::this_thread::get_id()) {

}