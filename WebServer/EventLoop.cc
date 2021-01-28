#include "EventLoop.h"
#include "log/Logging.h"
#include "Util.h"
#include "Channel.h"

#include <assert.h>
#include <sys/poll.h>
#include <sys/unistd.h>
#include <sys/eventfd.h>

thread_local EventLoop* t_loopInThisThread = 0;

int createEventFd() {
  int evtFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtFd < 0) {
    LOG_ERROR << "eventfd error";
    abort();
  }
  return evtFd;
}

EventLoop::EventLoop()
  : looping_(false),
    threadId_(std::this_thread::get_id()),
    weakupFd_(createEventFd()),
    quit_(false),
    eventHandling_(false),
    callingPendingFunctors(false),
    poller_(new Epoll()),
    pweakupChannel_(new Channel(this, weakupFd_)) {
  LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << "exist in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }

  pweakupChannel_->setEvent(EPOLLIN | EPOLLET);
  pweakupChannel_->setReadCallBack(std::bind(&EventLoop::handleRead, this));
  poller_->epoll_add(pweakupChannel_, 0);
}

EventLoop::~EventLoop() {
  ::close(weakupFd_);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;

  LOG_TRACE << "EventLoop " << this << " start looping";

  std::vector<std::shared_ptr<Channel>> activateChannels;
  while (!quit_) {
    activateChannels.clear();
    activateChannels = poller_->poll();
    eventHandling_ = true;
    for (auto& chann : activateChannels) {
      chann->handleEvent();
    }
    eventHandling_ = false;
    doPendingFunctors();
    poller_->hanleExpired();
  }  

  LOG_TRACE << "EventLoop " << this << " stop looping.";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = false;
  if (!isInLoopThread()) {
    weakup();
  }
}

void EventLoop::runInLoop(const Functor& cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(cb);
  }
}

void EventLoop::queueInLoop(const Functor& cb) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    pendingFunctors_.push_back(cb);
  }
  if (!isInLoopThread() || callingPendingFunctors) {
    weakup();
  }
}

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

void EventLoop::weakup() {
  uint64_t one = 1;
  ssize_t n = writen(weakupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG_ERROR << "EventLoop::weakup writen " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = readn(weakupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG_ERROR << "EventLoop::handleRead read " << n << " bytes instead of 8";
  }
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors = true;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (const Functor& functor : functors) functor(); 
  callingPendingFunctors = false;
}