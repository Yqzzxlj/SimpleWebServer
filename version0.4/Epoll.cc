#include "Epoll.h"
#include "util.h"

#include <unistd.h>
#include <iostream>


const int Epoll::MAX_EVENTS = 10240;

// 可读 ｜ ET模式 ｜ 保证一个socket连接在任一时刻只被一个线程处理
const __uint32_t Epoll::DEFAULT_EVENTS = (EPOLLIN | EPOLLET | EPOLLONESHOT);
struct epoll_event* Epoll::events = NULL;
std::unordered_map<int, std::shared_ptr<http::HttpData>> Epoll::http_data_map;

int Epoll::init(int max_event = MAX_EVENTS) {
  int epoll_fd = ::epoll_create(max_event);
  if (epoll_fd == -1) {
    std::cout << "epoll create error" << std::endl;
    exit(-1);
  }
  events = new struct epoll_event[max_event];
  return epoll_fd;
}

int Epoll::addfd(int epoll_fd, int fd, __uint32_t events,
                 std::shared_ptr<http::HttpData> http_data) {
  struct epoll_event event;
  event.events = events;
  event.data.fd = fd;

  http_data_map[fd] = http_data;
  int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
  if (ret < 0) {
    std::cout << "epoll add error" << std::endl;
    http_data_map[fd].reset();
    return -1;
  }
  // std::cout << "add a fd!" << fd << std::endl;
  return 0;
}

int Epoll::modfd(int epoll_fd, int fd, __uint32_t events,
                 std::shared_ptr<http::HttpData> http_data) {
  struct epoll_event event;
  event.events = events;
  event.data.fd = fd;

  http_data_map[fd] = http_data;
  int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
  if (ret < 0) {
    std::cout << "epoll mod error" << std::endl;
    http_data_map[fd].reset();
    return -1;
  }
  return 0;
}

int Epoll::delfd(int epoll_fd, int fd, __uint32_t events) {
  struct epoll_event event;
  event.events = events;
  event.data.fd = fd;

  int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
  if(ret < 0) {
    std::cout << "epoll del error" << std::endl;
    return -1;
  }
  auto it = http_data_map.find(fd);
  if (it != http_data_map.end()) {
    http_data_map.erase(it);
  }
  return 0;
}

std::vector<std::shared_ptr<http::HttpData>> Epoll::poll(
  const net::ServerSocket& server_socket, int max_event, int timeout) {
  
  int event_num = ::epoll_wait(server_socket.epoll_fd, events, max_event, timeout);
  if (event_num < 0) {
    std::cout << "epoll_wait error" << std::endl;
    exit(1);
  }

  std::vector<std::shared_ptr<http::HttpData>> httpDatas;

  for (int i = 0; i < event_num; ++i) {
    int fd = events[i].data.fd;
    // std::cout << "event.id: " << fd << std::endl;
    if (fd == server_socket.listen_fd) {
      handleConnection(server_socket);
    } else {
      // 出错的描述符，关闭文件描述符
      if ((events[i].events & EPOLLERR)           // 错误
          || (events[i].events & EPOLLRDHUP)      // TCP连接被对方关闭，或者对方关闭了写操作
          || (events[i].events & EPOLLHUP)        // 挂起
         ) {
        std::cout << "event error" << std::endl;
        auto it = http_data_map.find(fd);
        if (it != http_data_map.end()) {
          http_data_map.erase(it);
        }
        continue;
      }

      auto it = http_data_map.find(fd);
      if (it != http_data_map.end()) {
        if ((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI)) {
          httpDatas.push_back(it->second);

          http_data_map.erase(it);
        }
      } else {
        std::cout << "长连接第二次连接未找到" << std::endl;
        ::close(fd);
        continue;
      }
    }
  }
  return httpDatas;
}

void Epoll::handleConnection(const net::ServerSocket& server_socket) {
  std::shared_ptr<net::ClientSocket> temp_client(new net::ClientSocket());

  while (server_socket.accept(*temp_client) >= 0) {
    util::setnonblocking(temp_client->fd);
    
    std::shared_ptr<http::HttpData> http_data(new http::HttpData());
    http_data->request = std::shared_ptr<http::HttpRequest>(new http::HttpRequest());
    http_data->response = std::shared_ptr<http::HttpResponse>(new http::HttpResponse());

    std::shared_ptr<net::ClientSocket> client_socket(new net::ClientSocket());
    client_socket.swap(temp_client);
    http_data->client_socket = client_socket;
    http_data->epoll_fd = server_socket.epoll_fd;

    addfd(server_socket.epoll_fd, client_socket->fd, DEFAULT_EVENTS, http_data);
  }
  // std::cout << "handle over" << std::endl;
}