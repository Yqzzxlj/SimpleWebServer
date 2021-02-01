#pragma once
#include <arpa/inet.h>
#include <string>

namespace net {

void setReusePort(int fd);

class ClientSocket {
public:
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
  void accept(ClientSocket&);

  sockaddr_in addr;
  int fd;
  int port;
  std::string ip;
};

} // namespace net