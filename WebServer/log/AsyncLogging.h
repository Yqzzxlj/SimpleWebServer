#pragma once

#include "../noncopyable.h"
#include "LogStream.h"

#include <string>
#include <vector>
#include <thread>
#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>


class AsyncLogging : noncopyable {
 public:
  AsyncLogging(const std::string& basename, int flushInterval = 2);
  ~AsyncLogging() {
    if (running_) stop();
  }

  void append(const char* logline, int len);

  void start() {
    running_ = true;
    thread_.reset(new std::thread(threadFunc_));
  }
  void stop() {
    running_ = false;
    cond_.notify_one();
    thread_->join();
  }

 private:
  void threadFunc();
  typedef FixedBuffer<kLargeBuffer> Buffer;
  typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
  typedef std::shared_ptr<Buffer> BufferPtr;

  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;

  const int flushInterval_;
  std::string basename_;

  bool running_;
  std::function<void()> threadFunc_;
  std::unique_ptr<std::thread> thread_;

  std::mutex mutex_;
  std::condition_variable cond_;

};