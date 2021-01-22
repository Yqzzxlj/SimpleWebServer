#include "Logging.h"
#include "AsyncLogging.h"

#include <string>
#include <sys/time.h>

std::once_flag init_flag;
static AsyncLogging* AsyncLogger_;
std::string Logger::logFileName_ = "./WebServer.log";

void once_init() {
  AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
  AsyncLogger_->start();
}

void output(const char* msg, int len) {
  std::call_once(init_flag, once_init);
  AsyncLogger_->append(msg, len);
}

Logger::Impl::Impl(const char* fileName, int line)
  : stream_(), line_(line), basename_(fileName) {
  formatTime();
}

void Logger::Impl::formatTime() {
  struct timeval tv;
  time_t time;
  char str_t[26] = {0};
  gettimeofday(&tv, NULL);
  time = tv.tv_sec;
  struct tm* p_time = localtime(&time);
  strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
  stream_ << str_t;
}

Logger::Logger(const char* fileName, int line)
  : impl_(fileName, line) {}

Logger::~Logger() {
  impl_.stream_ << " -- " << impl_.basename_ << ':' << impl_.line_ << '\n';
  const LogStream::Buffer& buf(stream().buffer());
  output(buf.data(), buf.length());
}



