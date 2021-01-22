#pragma once

#include <string>
#include <memory>
#include <mutex>


class AppendFile;

class LogFile {

 public:
  LogFile(const std::string& basename,
          int checkEveryN = 1024);
  
  ~LogFile();

  void append(const char* logline, int len);

  void flush();

 private:
  
  void append_unlocked(const char* logline, int len);

  const std::string __basename;
  const int __checkEveryN;

  int count_;
  std::unique_ptr<AppendFile> file_;
  std::mutex mutex_;
};