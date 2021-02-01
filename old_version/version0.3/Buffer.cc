#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <algorithm>

const char util::Buffer::CRLF[] = "\r\n";
const size_t util::Buffer::INITIAL_SIZE = 1024;

ssize_t util::Buffer::readFd(int fd, int* saved_errno) {
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writeable = writable_bytes();

  vec[0].iov_base = begin() + write_index;
  vec[0].iov_len = writeable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);

  const int iovcnt = (writeable < sizeof (extrabuf) ? 2 : 1);
  const ssize_t n = ::readv(fd, vec, iovcnt);

  if (n < 0) {
    *saved_errno = errno;
  } else if (static_cast<size_t>(n) <= writeable) {
    write_index += n;
  } else {
    write_index = buffer.size();
    append(extrabuf, n - writeable);
  }
  return n;

}