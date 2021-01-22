#pragma once
#include "LogStream.h"

#include <string>

class Logger {

 public:
  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };

  Logger(const char* fileName, int line);
  Logger(const char* fileName, int line, LogLevel level);
  ~Logger();

  LogStream& stream() { return impl_.stream_; }

  static void setLogFileName(const std::string& fileName) {
    logFileName_ = fileName;
  }
  static std::string getLogFileName() { return logFileName_; }

  static LogLevel getLogLevel();
  static void setLogLevel(LogLevel level);

 private:
  class Impl {
   public:
    Impl(const char* fileName, int line, LogLevel level);
    void formatTime();

    LogStream stream_;
    int line_;
    LogLevel level_;
    std::string basename_;
  };

  Impl impl_;
  static std::string logFileName_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::getLogLevel() {
  return g_logLevel;
}

//
// CAUTION: do not write:
//
// if (good)
//   LOG_INFO << "Good news";
// else
//   LOG_WARN << "Bad news";
//
// this expends to
//
// if (good)
//   if (logging_INFO)
//     logInfoStream << "Good news";
//   else
//     logWarnStream << "Bad news";
//
#define LOG_TRACE if (Logger::getLogLevel() <= Logger::TRACE) \
  Logger(__FILE__, __LINE__, Logger::TRACE).stream()
#define LOG_DEBUG if (Logger::getLogLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG).stream()
#define LOG_INFO if (Logger::getLogLevel() <= Logger::INFO) \
  Logger(__FILE__, __LINE__, Logger::INFO).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, LINE__, Logger::FATAL).stream()