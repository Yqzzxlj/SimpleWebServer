#include "Socket.h"
#include "config.h"

#include <stdlib.h> // exit()
#include <unistd.h> // close()
#include <sys/socket.h> // bind, listen

#include <iostream> // cout

void net::setReusePort(int fd) {
  int optval = 1;
  int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(optval));
  if (ret < 0) {
    std::cout << "set reuse prot failed in file <" << __FILE__ << "> at " << __LINE__ << std::endl;
    exit(0);
  }
}

net::ClientSocket::~ClientSocket() {
  std::cout << "client socket closed" << std::endl;
  ::close(fd);
}

net::ServerSocket::ServerSocket(int _port, const std::string& _ip)
    : port(_port), ip(_ip) {
  
  // initial addr
  addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (!ip.empty()) {
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
  } else {
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
  }

  // create socket
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    std::cout << "create socket failed in file <" << __FILE__ << "> at " << __LINE__ << std::endl;
    exit(0);
  }

  setReusePort(fd);
}

void net::ServerSocket::bind() {
  int ret = ::bind(fd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == -1) {
    std::cout << "bind socket failed in file <" << __FILE__ << "> at " << __LINE__ << std::endl;
    exit(0);
  }
}

void net::ServerSocket::listen() {
  int ret = ::listen(fd, LINSTENQ);
  if (ret == -1) {
    std::cout << "listen socket failed in file <" << __FILE__ << "> at " << __LINE__ << std::endl;
    exit(0);
  }
}

void net::ServerSocket::accept(ClientSocket& client_socket) {
  int clientfd = ::accept(fd, (struct sockaddr*)&client_socket.addr, &client_socket.addr_len);
  if (clientfd == -1) {
    std::cout << "accept socket failed in file <" << __FILE__ << "> at " << __LINE__ << std::endl;
    exit(0);
  }
  client_socket.fd = clientfd;
  std::cout << "accept a client" << std::endl;
}

net::ServerSocket::~ServerSocket() {
  std::cout << "server socket closed" << std::endl;
  ::close(fd);
}