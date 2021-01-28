#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HtteRequestParser.h"
#include "HttpData.h"
#include "../Buffer.h"
#include "../config.h"
#include "../ThreadPool.h"
#include "../Epoll.h"
#include "../Timer.h"

#include <sys/stat.h> // stat
#include <sys/mman.h> // mmap, munmap
#include <unistd.h> // close
#include <fcntl.h> // open
#include <iostream>
#include <memory>

void HttpServer::run() {
  ThreadPool thread_pool(5);
  int epoll_fd = Epoll::init(1024);
  
  std::shared_ptr<HttpData> http_data(new HttpData());
  http_data->epoll_fd_ = epoll_fd;
  serverSocket_.epoll_fd_ = epoll_fd;

  __uint32_t event = (EPOLLIN | EPOLLET);
  Epoll::addfd(epoll_fd, serverSocket_.listen_fd_, event, http_data);

  while (true) {
    std::vector<std::shared_ptr<HttpData>> events = Epoll::poll(serverSocket_, 1024, 1000);
    for (auto req : events) {
      thread_pool.enqueue(std::bind(&HttpServer::handle_request, this, std::placeholders::_1), req);
    }
    // std::cout << "ready to handle expired event" << std::endl;
    Epoll::timer_manager_.handleExpiredEvent();
  }
}

void HttpServer::handle_request(std::shared_ptr<HttpData> http_data) {
  Buffer buffer;

  HttpRequestParser::PARSE_STATE  parse_state = HttpRequestParser::PARSE_REQUESTLINE;

  while (true) {
    int time = 0;
  again:
    int error_no;
    ssize_t recv_data = buffer.readFd(http_data->client_socket_->fd_, &error_no);
    if (recv_data == -1) {
      // std::cout << error_no << std::endl;
      if ((error_no == EAGAIN) || (error_no == EWOULDBLOCK)) {
        ++time;
        if (time == 3) return;
        goto again;
      }
      LOG_ERROR << "reading failed";
      return;
    }
    if (recv_data == 0) {
        LOG_INFO << "connection closed by peer" << http_data->client_socket_->fd_;
        break;
    }

    // http_data->request = std::make_shared<http::HttpRequest>(new http::HttpRequest());
    HttpRequestParser::HTTP_CODE  retcode = HttpRequestParser::parse_content(
            buffer, parse_state, *(http_data->request_));

    if (retcode == HttpRequestParser::NO_REQUEST) {
        continue;
    }

    // http_data->response = std::make_shared<http::HttpResponse>(new http::HttpResponse());
    if (retcode == HttpRequestParser::GET_REQUEST) {
      // std::cout << "get a request" << std::endl;
      auto it = http_data->request_->headers.find(HttpRequest::Connection);
      if (it != http_data->request_->headers.end()) {
        if (it->second == "keep-alive") {
          http_data->response_->close_connection = false;
          http_data->response_->headers.insert({"Keep-Alive", std::string("timeout=5")});
        } else {
          http_data->response_->close_connection = true;
        }
      }
      header(http_data);
      getMime(http_data);
      static_file(http_data, STATIC_PATH);
      send(http_data);
      if (!http_data->response_->close_connection) {
        Epoll::modfd(http_data->epoll_fd_, http_data->client_socket_->fd_, Epoll::DEFAULT_EVENTS, http_data);
        Epoll::timer_manager_.addTimer(http_data, TimerManager::DEFAULT_TIME_OUT);
      }
    } else {
      LOG_ERROR << "Bad Request";
    }
  }
}

void HttpServer::header(std::shared_ptr<HttpData> http_data) {
  if (http_data->request_->version == HttpRequest::HTTP_11) {
    http_data->request_->version = HttpRequest::HTTP_11;
  } else {
    http_data->request_->version = HttpRequest::HTTP_10;
  }
  http_data->response_->headers.insert(std::make_pair("Server", "Tiny Web Server"));
}

void HttpServer::getMime(std::shared_ptr<HttpData> http_data) {
  std::string filepath = http_data->request_->uri;
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

  auto it = mime_map.find(mime);
  if (it != mime_map.end()) {
    http_data->response_->mime = it->second;
  } else {
    http_data->response_->mime = mime_map.find("default")->second;
  }

  http_data->response_->filepath = filepath;
}

void HttpServer::static_file(std::shared_ptr<HttpData> http_data, const std::string& basepath) {
  struct stat file_stat;
  std::string filepath = basepath + http_data->response_->filepath;
  if (stat(filepath.c_str(), &file_stat) < 0) {
    http_data->response_->state_code = HttpResponse::k404NotFound;
    http_data->response_->short_msg = "Not Found";
    http_data->response_->filepath = basepath + "/404.html";
    return;
  }

  if(!S_ISREG(file_stat.st_mode)){
    http_data->response_->state_code = HttpResponse::k403forbiden;
    http_data->response_->short_msg = "ForBidden";
    http_data->response_->filepath = basepath + "/403.html";
    return;
  }

  http_data->response_->state_code = HttpResponse::k200Ok;
  http_data->response_->short_msg = "OK";
  http_data->response_->filepath = filepath;
  return;
}

// TODO 用writen代替send
void HttpServer::send(std::shared_ptr<HttpData> http_data) {
  // char header[BUFFERSIZE];
  // bzero(header, '\0');
  std::string header;
  http_data->response_->appendBuffer(header);
  std::string internal_error = "Internal Error";

  std::string filepath = http_data->response_->filepath;

  struct stat file_stat;
  if (stat(filepath.c_str(), &file_stat) < 0) {
    header += "Content-length: " + std::to_string(internal_error.size()) + "\r\n\r\n";
    header += internal_error;
    ::send(http_data->client_socket_->fd_, header.c_str(), header.size(), 0);
  }


  int filefd = ::open(filepath.c_str(), O_RDONLY, 0);
  if (filefd < 0) {
    header += "Content-length: " + std::to_string(internal_error.size()) + "\r\n\r\n";
    header += internal_error;
    ::send(http_data->client_socket_->fd_, header.c_str(), header.size(), 0);
  }

  header += "Content-length: " + std::to_string(file_stat.st_size) + "\r\n\r\n";

  ::send(http_data->client_socket_->fd_, header.c_str(), header.size(), 0);
  void *mapbuf = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, filefd, 0);
  ::send(http_data->client_socket_->fd_, mapbuf, file_stat.st_size, 0);
  munmap(mapbuf, file_stat.st_size);
  close(filefd);
  LOG_INFO << "send a file to " << http_data->client_socket_->fd_;
  return;
}