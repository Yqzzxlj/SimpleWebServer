#include "Util.h"

#include <unistd.h>
#include <errno.h>

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