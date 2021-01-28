#pragma once
#include "../Socket.h"
#include "../log/Logging.h"

#include <memory>
#include <string>

class HttpData;

const int BUFFERSIZE = 1024;

class HttpServer {
public:
  explicit HttpServer(int port = 80, const std::string& ip = "")
      : serverSocket_(port, ip) {
    serverSocket_.bind();
    serverSocket_.listen();
    LOG_ERROR << "server socket fd: " << serverSocket_.listen_fd_;
  }

  void run();

private:
  void handle_request(std::shared_ptr<HttpData>);
  
  void header(std::shared_ptr<HttpData>);
  void static_file(std::shared_ptr<HttpData>, const std::string&);
  void send(std::shared_ptr<HttpData>);
  void getMime(std::shared_ptr<HttpData>);


  ServerSocket serverSocket_;
};