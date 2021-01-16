#include "Socket.h"
#include "config.h"
#include "util.h"

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
  if (fd >= 0) {
    ::close(fd);
    std::cout << "client socket closed: " << fd << std::endl;
    fd = -1;
  }
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
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd == -1) {
    std::cout << "create socket failed in file <" << __FILE__ << "> at " << __LINE__ << std::endl;
    exit(0);
  }

  setReusePort(listen_fd);
  util::setnonblocking(listen_fd); 
}

void net::ServerSocket::bind() {
  int ret = ::bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
  if (ret == -1) {
    std::cout << "bind socket failed in file <" << __FILE__ << "> at " << __LINE__ << std::endl;
    exit(0);
  }
}

void net::ServerSocket::listen() {
  int ret = ::listen(listen_fd, LINSTENQ);
  if (ret == -1) {
    std::cout << "listen socket failed in file <" << __FILE__ << "> at " << __LINE__ << std::endl;
    exit(0);
  }
}

int net::ServerSocket::accept(ClientSocket& client_socket) const {
  int clientfd = ::accept(listen_fd, (struct sockaddr*)&client_socket.addr, &client_socket.addr_len);
  if (clientfd == -1) {
    if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
      return clientfd;
    }
    std::cout << errno << std::endl;
    std::cout << "accept socket failed in file <" << __FILE__ << "> at " << __LINE__ << std::endl;
    exit(0);
  }
  client_socket.fd = clientfd;
  std::cout << "accept a client: " << clientfd << std::endl;
  return clientfd;
}

net::ServerSocket::~ServerSocket() {
  if (listen_fd >= 0) {
    std::cout << "server socket closed" << std::endl;
    ::close(listen_fd);
    listen_fd = -1;
  }
}