#include "util.h"

#include <unistd.h>     // read(), write(), close()
#include <errno.h>      // errno, EINTR

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

} // namespace util
