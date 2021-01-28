#include "Socket.h"
#include "config.h"
#include "Util.h"
#include "log/Logging.h"

#include <stdlib.h> // exit()
#include <unistd.h> // close()
#include <sys/socket.h> // bind, listen

void setReusePort(int fd) {
  int optval = 1;
  int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(optval));
  if (ret < 0) {
    LOG_ERROR << "set reuse prot failed in file";
    exit(0);
  }
}

ClientSocket::~ClientSocket() {
  if (fd_ >= 0) {
    ::close(fd_);
    LOG_INFO << "client socket " << fd_ << " closed.";
    fd_ = -1;
  }
}

ServerSocket::ServerSocket(int port, const std::string& ip)
    : port_(port), ip_(ip) {
  
  // initial addr
  addr_ = {0};
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  if (!ip.empty()) {
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
  } else {
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
  }

  // create socket
  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ == -1) {
    LOG_ERROR << "create socket failed";
    exit(0);
  }

  setReusePort(listen_fd_);
  setNonblocking(listen_fd_); 
}

void ServerSocket::bind() {
  int ret = ::bind(listen_fd_, (struct sockaddr*)&addr_, sizeof(addr_));
  if (ret == -1) {
    LOG_ERROR << "bind socket failed";
    exit(0);
  }
}

void ServerSocket::listen() {
  int ret = ::listen(listen_fd_, LINSTENQ);
  if (ret == -1) {
    LOG_ERROR << "listen socket failed";
    exit(0);
  }
}

int ServerSocket::accept(ClientSocket& client_socket) const {
  int clientfd = ::accept(listen_fd_, (struct sockaddr*)&client_socket.addr_, &client_socket.addr_len_);
  if (clientfd == -1) {
    if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
      return clientfd;
    }
    LOG_DEBUG << "accept socket failed, errno: " << errno;
    exit(0);
  }
  client_socket.fd_ = clientfd;
  LOG_INFO << "accept a client: " << clientfd;
  return clientfd;
}

ServerSocket::~ServerSocket() {
  if (listen_fd_ >= 0) {
    LOG_INFO << "server socket closed";
    ::close(listen_fd_);
    listen_fd_ = -1;
  }
}