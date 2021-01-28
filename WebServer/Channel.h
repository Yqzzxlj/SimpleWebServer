#pragma once
#include "noncopyable.h"
#include "http/HttpData.h"

#include <functional>

class EventLoop;

class Channel : noncopyable {
 public:
  typedef std::function<void()> EventCallBack;
  
  Channel(EventLoop* loop);
  Channel(EventLoop* loop, int fd);
  ~Channel();

  void handleEvent();

  void setReadCallBack(const EventCallBack& cb) {
    readCallBack_ = cb;
  }
  void setWriteCallBack(const EventCallBack& cb) {
    writeCallBack_ = cb;
  }
  void setErrorCallBack(const EventCallBack& cb) {
    errorCallBack_ = cb;
  }

  void setHttpData(const std::shared_ptr<http::HttpData>& hd) { httpData_ = hd; }
  std::shared_ptr<http::HttpData> getHttpData() {
    if (httpData_.lock()) {
      return std::shared_ptr<http::HttpData>(httpData_);
    }
  }
  int fd() const { return fd_; }
  void setFd(int fd) { fd_ = fd; }
  int events() const { return events_; }
  void setEvent(__uint32_t evt) { events_ = evt; }
  void setRevents(__uint32_t revt) { revents_ = revt; }
  EventLoop* ownerLoop() { return loop_; }
  

 private:
  EventLoop* loop_; 
  int fd_;
  __uint32_t events_; // 关心的IO事件， 由用户设置
  __uint32_t revents_; // 活动的事件， 由EventLoop/Poller设置。

  std::weak_ptr<http::HttpData> httpData_;
  EventCallBack readCallBack_;
  EventCallBack writeCallBack_;
  EventCallBack errorCallBack_;
};