#include "util.h"

#include <unistd.h>     // read(), write(), close()
#include <errno.h>      // errno, EINTR
#include <fcntl.h>      // fcntl()
#include <iostream>

namespace util {

std::string& ltrim(std::string& str) {
  if (!str.empty()) {
    str.erase(0, str.find_first_not_of(" \t\r\n"));
  }
  return str;
}

std::string& rtrim(std::string& str) {
  if (!str.empty()) {
    str.erase(str.find_last_not_of(" \t\r\n") + 1);
  }
  return str;
}

std::string& trim(std::string& str) {
  if (!str.empty()) {
    ltrim(str);
    rtrim(str);
  }
  return str;
}


int setnonblocking(int fd) {
    int old_option = ::fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    if(::fcntl(fd, F_SETFL, new_option) == -1) {
      std::cout << "set nonblock failed" << std::endl;
    }
    return old_option;
}
} // namespace util
