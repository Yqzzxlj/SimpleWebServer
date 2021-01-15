#pragma once
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Socket.h"
#include <memory>

namespace http {

class HttpData : public std::enable_shared_from_this<HttpData> {

public:
  HttpData() : epoll_fd(-1) {}

public:
  std::shared_ptr<http::HttpRequest> request;
  std::shared_ptr<http::HttpResponse> response;
  std::shared_ptr<net::ClientSocket> client_socket;
  int epoll_fd;
};

} // namespace http
