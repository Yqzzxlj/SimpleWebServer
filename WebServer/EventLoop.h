#pragma once
#include <noncopyable.h>
#include <thread>

class EventLoop {
public:

  EventLoop();
  ~EventLoop();

  void loop();

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      this->assertInLoopThread();
    }
  }

  bool isInLoopThread() const {
    return __threadId == std::this_thread::get_id();
  }

private:
  void abortNotInLoopThread();

  bool __looping; // atomic
  const std::thread::id __threadId;
};