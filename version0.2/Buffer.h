#pragma once

#include <vector>
#include <string>

namespace util {

class Buffer {
public:
  static const size_t INITIAL_SIZE;

  explicit Buffer(size_t initial_size = INITIAL_SIZE)
      : buffer(initial_size),
        reader_index(0),
        write_index(0) {}

  size_t readable_bytes() const {
    return write_index - reader_index;
  }

  size_t writable_bytes() const {
    return buffer.size() - write_index;
  }

  const char* peek() const {
    return begin() + reader_index;
  }

  const char* findCRLF() const {
    const char* crlf = std::search(peek(), begin_write(), CRLF, CRLF + 2);
    return crlf == begin_write() ? NULL : crlf;
  }

  std::string retrieve_all() {
    std::string ret(peek(), peek() + readable_bytes());
    reader_index = 0;
    write_index = 0;
    return ret;
  }

  std::string retrieve_line() {
    const char* end = std::find(peek(), peek() + readable_bytes(), '\n');
    std::string ret(peek(), end + 1);
    reader_index += ret.size();
    if (reader_index == write_index) {
      reader_index = 0;
      write_index = 0;
    }
    return ret;
  }

  void append(const char* data, size_t len) {
    ensure_writable_bytes(len);
    std::copy(data, data + len, begin_write());
    has_written(len);
  }

  void append(const std::string& str) {
    append(str.c_str(), str.size());
  }

  char* begin_write() {
    return begin() + write_index;
  }

  const char* begin_write() const {
    return begin() + write_index;
  }

  void ensure_writable_bytes(size_t len) {
    if (writable_bytes() < len) {
      makeSpace(len);
    }
    assert(writable_bytes() >= len);
  }

  void has_written(size_t len) {
    assert(len <= writable_bytes());
    write_index += len;
  }

  ssize_t readFd(int fd, int* saved_errno);

private:
  char* begin() {
    return &*buffer.begin();
  }
  const char* begin() const {
    return &*buffer.begin();
  }

  void makeSpace(size_t len) {
    if (writable_bytes() + reader_index < len) {
      buffer.resize(write_index + len);
    } else {
      size_t readable = readable_bytes();
      std::copy(begin() + reader_index,
                begin() + write_index,
                begin());
      reader_index = 0;
      write_index = reader_index + readable;
    }
  }

  std::vector<char> buffer;
  size_t reader_index;
  size_t write_index;

  static const char CRLF[];
};

} // namespace util