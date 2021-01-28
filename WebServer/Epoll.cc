#include "Epoll.h"
#include "Channel.h"
#include "Logging.h"

#include <assert.h>


const int MAXFDS = 100 * 1000;
const int EVENTSNUM = 4096;
const int EPOLLWAITTIME = 10 * 1000;

Epoll::Epoll()
  : epollFd_(::epoll_create1(EPOLL_CLOEXEC)),
    events(EVENTSNUM) {
  assert(epollFd_ > 0);
}

Epoll::~Epoll() {}

void Epoll::epoll_add(std::shared_ptr<Channel> channel,
                      int timeout = TimerManager::DEFAULT_TIME_OUT) {
  if (timeout > 0) {
    timerManager_.add_timer(channel->getHttpData(), timeout);
  }

  int fd = channel->fd();

  struct epoll_event event;
  event.events = channel->events();
  event.data.fd = fd;

  fd2chann_[fd] = channel;
  if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0) {
    LOG_ERROR << "epoll add error";
    fd2chann_[fd].reset();
  }
}

void Epoll::epoll_mod(std::shared_ptr<Channel> channel,
                      int timeout = TimerManager::DEFAULT_TIME_OUT) {
  if (timeout > 0) {
    timerManager_.add_timer(channel->getHttpData(), timeout);
  }

  int fd = channel->fd();

  struct epoll_event event;
  event.events = channel->events();
  event.data.fd = fd;

  if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0) {
    LOG_ERROR << "epoll mod error";
    fd2chann_[fd].reset();
  }
}

void Epoll::epoll_del(std::shared_ptr<Channel> channel) {
  int fd = channel->fd();

  struct epoll_event event;
  event.events = channel->events();
  event.data.fd = fd;

  if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event)) {
    LOG_ERROR << "epoll del error";
  }
  fd2chann_[fd].reset();
}

std::vector<std::shared_ptr<Channel>> Epoll::poll() {
  while (true) {
    int event_num = epoll_wait(epollFd_, &*events.begin(), EVENTSNUM, EPOLLWAITTIME);
    if (event_num < 0) LOG_ERROR << "epoll wait error";
    if (event_num > 0) {
      std::vector<std::shared_ptr<Channel>> ret_data;
      for (int i = 0; i < event_num; ++i) {
        int fd = events[i].data.fd;
        std::shared_ptr<Channel> chann_ = fd2chann_[fd];
        if (chann_ != nullptr) {
          chann_->setRevents(events[i].events);
          chann_->setEvent(0);
          ret_data.push_back(chann_);
        } else {
          LOG_WARN << "Invalid channel ptr";
        }
      }
      return ret_data;
    }
  }
}

void Epoll::addTimer(std::shared_ptr<Channel> channel, int timeout) {
  std::shared_ptr<http::HttpData> httpdata = channel->getHttpData();
  if (httpdata != nullptr) {
    timerManager_.add_timer(httpdata, timeout);
  } else {
    LOG_INFO << "add timer failed.";
  }
}

void Epoll::hanleExpired() {
  timerManager_.handle_expired_event();
}
