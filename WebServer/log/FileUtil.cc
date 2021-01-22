#include "FileUtil.h"

AppendFile::AppendFile(const std::string& filename)
  : fp_(::fopen(filename.c_str(), "ae")) { // 'e' for O_CLOEXEC,  即exec()函数调用成功后，自动关闭文件描述符
    setbuffer(fp_, buffer_, sizeof(buffer_)); // 指定缓冲区
}

AppendFile::~AppendFile() {
  ::fclose(fp_);
}

void AppendFile::append(const char* logline, const size_t len) {
  size_t n = write(logline, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = write(logline + n, remain);
    if (x == 0) {
      int err = ferror(fp_);
      if (err) {
        fprintf(stderr, "AppendFile::append() failed!\n");
      }
      break;
    }
    n += x;
    remain -= x;
  }
}

void AppendFile::flush() {
  ::fflush(fp_);
}

size_t AppendFile::write(const char* logline, size_t len) {
  // fwite 的线程不安全版本
  return ::fwrite_unlocked(logline, 1, len, fp_);
}