#pragma once
#include "Socket.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <memory>
#include <string>

namespace net {

const int BUFFERSIZE = 1024;

class HttpServer {
public:
  explicit HttpServer(int port = 80, const std::string& ip = "")
      : serverSocket(port, ip) {
    serverSocket.bind();
    serverSocket.listen();
  }

  void run();

private:
  void handle_request(const ClientSocket &);
  
  void header(const http::HttpRequest&, http::HttpResponse&);
  void static_file(http::HttpResponse&, const std::string&);
  void send(const http::HttpResponse&, const net::ClientSocket&);
  void getMime(const http::HttpRequest&, http::HttpResponse&);


  ServerSocket serverSocket;
  
};


} // namespace net