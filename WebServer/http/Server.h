#pragma once
#include "Socket.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpData.h"
#include <memory>
#include <string>
#include <iostream>

namespace net {

const int BUFFERSIZE = 1024;

class HttpServer {
public:
  explicit HttpServer(int port = 80, const std::string& ip = "")
      : serverSocket(port, ip) {
    serverSocket.bind();
    serverSocket.listen();
    std::cout << "server socket fd: " << serverSocket.listen_fd << std::endl;
  }

  void run();

private:
  void handle_request(std::shared_ptr<http::HttpData>);
  
  void header(std::shared_ptr<http::HttpData>);
  void static_file(std::shared_ptr<http::HttpData>, const std::string&);
  void send(std::shared_ptr<http::HttpData>);
  void getMime(std::shared_ptr<http::HttpData>);


  ServerSocket serverSocket;
  
};


} // namespace net