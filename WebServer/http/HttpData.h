#pragma once
#include <memory>
#include "../log/Logging.h"


class TimerNode;
class ClientSocket;
class HttpRequest;
class HttpResponse;

class HttpData : public std::enable_shared_from_this<HttpData> {

public:
  HttpData() : epoll_fd_(-1) {}
  ~HttpData() {}

public:
  std::shared_ptr<HttpRequest> request_;
  std::shared_ptr<HttpResponse> response_;
  std::shared_ptr<ClientSocket> client_socket_;
  int epoll_fd_;

  void close_timer();
  void set_timer(std::shared_ptr<TimerNode>);

private:
  std::weak_ptr<TimerNode> timer_;
};