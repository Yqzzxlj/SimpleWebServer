#pragma once
#include <noncopyable.h>
#include <thread>

class EventLoop : noncopyable {
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
    return threadId_ == std::this_thread::get_id();
  }

private:
  void abortNotInLoopThread();

  bool looping_; // atomic
  const std::thread::id threadId_;
};