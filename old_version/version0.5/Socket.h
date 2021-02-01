#pragma once
#include <arpa/inet.h>
#include <string>

namespace net {

void setReusePort(int fd);

class ClientSocket {
public:
  ClientSocket() : fd(-1), addr{0}, addr_len(0) {}
  ~ClientSocket();

  socklen_t addr_len;
  sockaddr_in addr;
  int fd;
};

class ServerSocket {
public:
  ServerSocket(int _port = 8080, const std::string& _p = "");
  ~ServerSocket();
  void bind();
  void listen();
  int accept(ClientSocket&) const;

  sockaddr_in addr;
  int listen_fd;
  int epoll_fd;
  int port;
  std::string ip;
};

} // namespace net