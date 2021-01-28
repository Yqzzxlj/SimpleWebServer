#include "Channel.h"
#include "EventLoop.h"
#include "Logging.h"

#include <sys/epoll.h>

Channel::Channel(EventLoop* loop)
  : loop_(loop),
    fd_(0),
    events_(0),
    revents_(0) {}

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0) {}

Channel::~Channel() {}

void Channel::handleEvent() {
  if (revents_ & (EPOLLERR | EPOLLHUP | EPOLLWRBAND)) {
    if (errorCallBack_) errorCallBack_();
  }
  if (revents_ & (EPOLLIN | EPOLLPRI)) {
    if (readCallBack_) readCallBack_();
  }
  if (revents_ & EPOLLOUT) {
    if (writeCallBack_) writeCallBack_();
  }
}

