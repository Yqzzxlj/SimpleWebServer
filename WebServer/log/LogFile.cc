#include "LogFile.h"
#include "FileUtil.h"
#include <mutex>

LogFile::LogFile(const std::string& basename,
                 int checkEveryN)
  : __basename(basename),
    __checkEveryN(checkEveryN),
    count_(0) {
      file_.reset(new AppendFile(basename));
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len) {
  std::unique_lock<std::mutex> lock(mutex_);
  append_unlocked(logline, len);
}

void LogFile::flush() {
  std::unique_lock<std::mutex> lock(mutex_);
  file_->flush();
}

void LogFile::append_unlocked(const char* logline, int len) {
  file_->append(logline, len);
  ++count_;
  if (count_ >= __checkEveryN) {
    count_ = 0;
    file_->flush();
  }
}