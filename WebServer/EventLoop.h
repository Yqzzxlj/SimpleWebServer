#pragma once
#include "noncopyable.h"
#include "Epoll.h"
#include <thread>
#include <assert.h>
#include <functional>
#include <mutex>

class EventLoop : noncopyable {
public:
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop();

  void loop();
  void quit();

  void runInLoop(const Functor& cb);
  void queueInLoop (const Functor& cb);

  void assertInLoopThread() {
    assert(isInLoopThread());
  }

  bool isInLoopThread() const {
    return threadId_ == std::this_thread::get_id();
  }

  EventLoop* getEventLoopOfCurrentThread();

private:

  bool looping_; // atomic
  const std::thread::id threadId_;
  bool quit_;
  bool eventHandling_;
  std::shared_ptr<Epoll> poller_;

  
  int weakupFd_;
  std::shared_ptr<Channel> pweakupChannel_;

  std::mutex mutex_;
  std::vector<Functor> pendingFunctors_;
  bool callingPendingFunctors;

  void weakup();
  void handleRead();
  void doPendingFunctors();
};