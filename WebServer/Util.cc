#include "Util.h"
#include "log/Logging.h"

#include <unistd.h>     // read(), write(), close()
#include <errno.h>      // errno, EINTR
#include <fcntl.h>      // fcntl()
#include <sys/socket.h>
#include <signal.h>
#include <cstring>
#include <netinet/in.h>  // IPPROTO_TCP
#include <netinet/tcp.h> // TCP_NODELAY

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


int setNonblocking(int fd) {
    int old_option = ::fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    if(::fcntl(fd, F_SETFL, new_option) == -1) {
      LOG_ERROR << "set nonblock failed";
    }
    return old_option;
}

void handle_for_sigpipe() {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, NULL)) {
    return;
  }
}

void setNoDelay(int fd) {
  int enable = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
}

ssize_t readn(int fd, void* buff, int n) {
  int nleft = n;
  int nread = 0;
  char* cur = (char*)buff;

  while (nleft > 0) {
    ssize_t nread = ::read(fd, cur, nleft);
    if (nread < 0) {
      if (errno == EINTR) {
        nread = 0;
      } else if (errno == EAGAIN) {
        return n - nleft;
      } else {
        return -1;
      }
    } else if (nread == 0) {
      break;
    }
    nleft -= nread;
    cur += nread;
  }
  return n - nleft;
}

ssize_t writen(int fd, void* buff, int n) {
  int nleft = n;
  int nwritten = 0;
  char* cur = (char*)buff;

  while (nleft > 0) {
    ssize_t nwritten = ::write(fd, cur, nleft);
    if (nwritten <= 0) {
      if (errno == EINTR) {
        nwritten = 0;
      } else if (errno == EAGAIN) {
        return n - nleft;
      } else {
        return -1;
      }
    }
    nleft -= nwritten;
    cur += nwritten;
  }
  return n;
}