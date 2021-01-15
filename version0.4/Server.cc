#include "Server.h"
#include "config.h"
#include "Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HtteRequestParser.h"
#include "HttpData.h"
#include "ThreadPool.h"
#include "Epoll.h"

#include <sys/stat.h> // stat
#include <sys/mman.h> // mmap, munmap
#include <unistd.h> // close
#include <fcntl.h> // open
#include <iostream>
#include <memory>

void net::HttpServer::run() {
  ThreadPool thread_pool(5);
  int epoll_fd = Epoll::init(1024);
  
  std::shared_ptr<http::HttpData> http_data(new http::HttpData());
  http_data->epoll_fd = epoll_fd;
  serverSocket.epoll_fd = epoll_fd;

  __uint32_t event = (EPOLLIN | EPOLLET);
  Epoll::addfd(epoll_fd, serverSocket.listen_fd, event, http_data);

  while (true) {
    std::vector<std::shared_ptr<http::HttpData>> events = Epoll::poll(serverSocket, 1024, -1);
    for (auto req : events) {
      thread_pool.enqueue(std::bind(&HttpServer::handle_request, this, std::placeholders::_1), req);
    }
  }
}

void net::HttpServer::handle_request(std::shared_ptr<http::HttpData> http_data) {
  util::Buffer buffer;

  http::HttpRequestParser::PARSE_STATE  parse_state = http::HttpRequestParser::PARSE_REQUESTLINE;

  while (true) {
    int time = 0;
  again:
    int error_no;
    ssize_t recv_data = buffer.readFd(http_data->client_socket->fd, &error_no);
    if (recv_data == -1) {
      // std::cout << error_no << std::endl;
      if ((error_no == EAGAIN) || (error_no == EWOULDBLOCK)) {
        ++time;
        if (time == 3) return;
        goto again;
      }
      std::cout << "reading failed" << std::endl;
      return;
    }
    if (recv_data == 0) {
        std::cout << "connection closed by peer" << std::endl;
        break;
    }

    // http_data->request = std::make_shared<http::HttpRequest>(new http::HttpRequest());
    http::HttpRequestParser::HTTP_CODE  retcode = http::HttpRequestParser::parse_content(
            buffer, parse_state, *(http_data->request));

    if (retcode == http::HttpRequestParser::NO_REQUEST) {
        continue;
    }

    // http_data->response = std::make_shared<http::HttpResponse>(new http::HttpResponse());
    if (retcode == http::HttpRequestParser::GET_REQUEST) {
      // std::cout << "get a request" << std::endl;
      header(http_data);
      getMime(http_data);
      static_file(http_data, STATIC_PATH);
      send(http_data);
    } else {
      std::cout << "Bad Request" << std::endl;
    }
  }
}

void net::HttpServer::header(std::shared_ptr<http::HttpData> http_data) {
  if (http_data->request->version == http::HttpRequest::HTTP_11) {
    http_data->request->version = http::HttpRequest::HTTP_11;
  } else {
    http_data->request->version = http::HttpRequest::HTTP_10;
  }
  http_data->response->headers.insert(std::make_pair("Server", "Tiny Web Server"));
}

void net::HttpServer::getMime(std::shared_ptr<http::HttpData> http_data) {
  std::string filepath = http_data->request->uri;
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
    http_data->response->mime = it->second;
  } else {
    http_data->response->mime = http::mime_map.find("default")->second;
  }

  http_data->response->filepath = filepath;
}

void net::HttpServer::static_file(std::shared_ptr<http::HttpData> http_data, const std::string& basepath) {
  struct stat file_stat;
  std::string filepath = basepath + http_data->response->filepath;
  if (stat(filepath.c_str(), &file_stat) < 0) {
    http_data->response->state_code = http::HttpResponse::k404NotFound;
    http_data->response->short_msg = "Not Found";
    http_data->response->filepath = basepath + "/404.html";
    return;
  }

  if(!S_ISREG(file_stat.st_mode)){
    http_data->response->state_code = http::HttpResponse::k403forbiden;
    http_data->response->short_msg = "ForBidden";
    http_data->response->filepath = basepath + "/403.html";
    return;
  }

  http_data->response->state_code = http::HttpResponse::k200Ok;
  http_data->response->short_msg = "OK";
  http_data->response->filepath = filepath;
  return;
}

void net::HttpServer::send(std::shared_ptr<http::HttpData> http_data) {
  // char header[BUFFERSIZE];
  // bzero(header, '\0');
  std::string header;
  http_data->response->appendBuffer(header);
  std::string internal_error = "Internal Error";

  std::string filepath = http_data->response->filepath;

  struct stat file_stat;
  if (stat(filepath.c_str(), &file_stat) < 0) {
    header += "Content-length: " + std::to_string(internal_error.size()) + "\r\n\r\n";
    header += internal_error;
    ::send(http_data->client_socket->fd, header.c_str(), header.size(), 0);
  }


  int filefd = ::open(filepath.c_str(), O_RDONLY, 0);
  if (filefd < 0) {
    header += "Content-length: " + std::to_string(internal_error.size()) + "\r\n\r\n";
    header += internal_error;
    ::send(http_data->client_socket->fd, header.c_str(), header.size(), 0);
  }

  header += "Content-length: " + std::to_string(file_stat.st_size) + "\r\n\r\n";

  ::send(http_data->client_socket->fd, header.c_str(), header.size(), 0);
  void *mapbuf = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, filefd, 0);
  ::send(http_data->client_socket->fd, mapbuf, file_stat.st_size, 0);
  munmap(mapbuf, file_stat.st_size);
  close(filefd);
  return;
}