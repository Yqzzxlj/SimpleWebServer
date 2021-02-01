#include "Server.h"
#include "config.h"
#include "Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HtteRequestParser.h"

#include <sys/stat.h> // stat
#include <sys/mman.h> // mmap, munmap
#include <unistd.h> // close
#include <fcntl.h> // open
#include <iostream>

void net::HttpServer::run() {
  while (true) {
    ClientSocket client_socket;
    serverSocket.accept(client_socket);
    handle_request(client_socket);
  }
}

void net::HttpServer::handle_request(const ClientSocket & clientSocket) {
  util::Buffer buffer;

  http::HttpRequestParser::PARSE_STATE  parse_state = http::HttpRequestParser::PARSE_REQUESTLINE;

  while (true) {

      ssize_t recv_data = buffer.readFd(clientSocket.fd, NULL);
      if (recv_data == -1) {
          std::cout << "reading failed" << std::endl;
          exit(0);
      }
      if (recv_data == 0) {
          std::cout << "connection closed by peer" << std::endl;
          break;
      }

      http::HttpRequest  request;

      http::HttpRequestParser::HTTP_CODE  retcode = http::HttpRequestParser::parse_content(
              buffer, parse_state, request);

      if (retcode == http::HttpRequestParser::NO_REQUEST) {
          continue;
      }

      if (retcode == http::HttpRequestParser::GET_REQUEST) {
          http::HttpResponse response(true);
          header(request, response);
          getMime(request, response);
          static_file(response, STATIC_PATH);
          send(response, clientSocket);
      } else {
          std::cout << "Bad Request" << std::endl;
      }
  }
}

void net::HttpServer::header(const http::HttpRequest& request,  http::HttpResponse& response) {
  if (request.version == http::HttpRequest::HTTP_11) {
    response.version = http::HttpRequest::HTTP_11;
  } else {
    response.version = http::HttpRequest::HTTP_10;
  }
  response.headers.insert(std::make_pair("Server", "Tiny Web Server"));
}

void net::HttpServer::getMime(const http::HttpRequest &request, http::HttpResponse &response) {
  std::string filepath = request.uri;
  std::string mime;
  int pos;
  if ((pos = filepath.rfind('?')) != std::string::npos) {
    filepath.erase(pos);
  }

  if (filepath.back() == '/') {
    filepath += "index.html";
  }

  if ((pos = filepath.rfind('.')) != std::string::npos) {
    mime = filepath.substr(pos);
  }

  auto it = http::mime_map.find(mime);
  if (it != http::mime_map.end()) {
    response.mime = it->second;
  } else {
    response.mime = http::mime_map.find("default")->second;
  }

  response.filepath = filepath;
}

void net::HttpServer::static_file(http::HttpResponse &response, const std::string& basepath) {
  struct stat file_stat;
  std::string filepath = basepath + response.filepath;
  if (stat(filepath.c_str(), &file_stat) < 0) {
    response.state_code = http::HttpResponse::k404NotFound;
    response.short_msg = "Not Found";
    response.filepath = basepath + "/404.html";
    return;
  }

  if(!S_ISREG(file_stat.st_mode)){
    response.state_code = http::HttpResponse::k403forbiden;
    response.short_msg = "ForBidden";
    response.filepath = basepath + "/403.html";
    return;
  }

  response.state_code = http::HttpResponse::k200Ok;
  response.short_msg = "OK";
  response.filepath = filepath;
  return;
}

void net::HttpServer::send(const http::HttpResponse& response, const net::ClientSocket& clientSocket) {
  // char header[BUFFERSIZE];
  // bzero(header, '\0');
  std::string header;
  response.appendBuffer(header);
  std::string internal_error = "Internal Error";

  std::string filepath = response.filepath;

  struct stat file_stat;
  if (stat(filepath.c_str(), &file_stat) < 0) {
    header += "Content-length: " + std::to_string(internal_error.size()) + "\r\n\r\n";
    header += internal_error;
    ::send(clientSocket.fd, header.c_str(), header.size(), 0);
  }


  int filefd = ::open(filepath.c_str(), O_RDONLY, 0);
  if (filefd < 0) {
    header += "Content-length: " + std::to_string(internal_error.size()) + "\r\n\r\n";
    header += internal_error;
    ::send(clientSocket.fd, header.c_str(), header.size(), 0);
  }

  header += "Content-length: " + std::to_string(file_stat.st_size) + "\r\n\r\n";

  ::send(clientSocket.fd, header.c_str(), header.size(), 0);
  void *mapbuf = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, filefd, 0);
  ::send(clientSocket.fd, mapbuf, file_stat.st_size, 0);
  munmap(mapbuf, file_stat.st_size);
  close(filefd);
  return;
}