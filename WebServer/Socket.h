#pragma once
#include <arpa/inet.h>
#include <string>

void setReusePort(int fd);

class ClientSocket {
public:
  ClientSocket() : fd_(-1), addr_{0}, addr_len_(0) {}
  ~ClientSocket();

  socklen_t addr_len_;
  sockaddr_in addr_;
  int fd_;
};

class ServerSocket {
public:
  ServerSocket(int port = 8080, const std::string& p = "");
  ~ServerSocket();
  void bind();
  void listen();
  int accept(ClientSocket&) const;

  sockaddr_in addr_;
  int listen_fd_;
  int epoll_fd_;
  int port_;
  std::string ip_;
};
