#include "AsyncLogging.h"
#include "LogFile.h"

#include <assert.h>
#include <chrono>

#include <iostream>

AsyncLogging::AsyncLogging(const std::string& basename,
                           int flushInterval)
  : basename_(basename),
    flushInterval_(flushInterval),
    running_(false),
    threadFunc_(std::bind(&AsyncLogging::threadFunc, this)),
    currentBuffer_(new Buffer()),
    nextBuffer_(new Buffer()) {
  assert(basename.size() > 1);
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len) {
  std::unique_lock<std::mutex> lock;
  if (currentBuffer_->avail() > len) {
    currentBuffer_->append(logline, len);
  } else {
    buffers_.push_back(std::move(currentBuffer_));
    if (nextBuffer_ != nullptr) {
      currentBuffer_ = std::move(nextBuffer_);
    } else {
      currentBuffer_.reset(new Buffer());
    }
    currentBuffer_->append(logline, len);
    cond_.notify_one();
  }
}

void AsyncLogging::threadFunc() {
  assert(running_);
  LogFile output(basename_);
  BufferPtr newBuffer1(new Buffer());
  BufferPtr newBuffer2(new Buffer());
  newBuffer1->bzero();
  newBuffer2->bzero();
  BufferVector buffersToWrite;
  buffersToWrite.reserve(16);
  while (running_) {
    assert(newBuffer1 != nullptr && newBuffer1->length() == 0);
    assert(newBuffer2 != nullptr && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (buffers_.empty()) {
        std::chrono::seconds dura(flushInterval_);
        cond_.wait_for(lock, dura);
      }
      buffers_.push_back(std::move(currentBuffer_));
      currentBuffer_ = std::move(newBuffer1);
      buffersToWrite.swap(buffers_);
      if (nextBuffer_ == nullptr) {
        nextBuffer_ = std::move(newBuffer2);
      }
    }


    assert(!buffersToWrite.empty());

    if(buffersToWrite.size() > 25) {
      buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
    }

    for (const auto& buffer : buffersToWrite) {
      output.append(buffer->data(), buffer->length());
    }

    if (buffersToWrite.size() > 2) {
      buffersToWrite.resize(2);
    }

    if (newBuffer1 == nullptr) {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (newBuffer2 == nullptr) {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output.flush();
  }
  output.flush();
}