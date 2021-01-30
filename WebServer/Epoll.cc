#include "Epoll.h"
#include "http/HttpData.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "Timer.h"
#include "Socket.h"
#include "Util.h"
#include "log/Logging.h"

#include <unistd.h>


const int Epoll::MAX_EVENTS = 10240;

// 可读 ｜ ET模式 ｜ 保证一个socket连接在任一时刻只被一个线程处理
const __uint32_t Epoll::DEFAULT_EVENTS = (EPOLLIN | EPOLLET | EPOLLONESHOT);
struct epoll_event* Epoll::events_ = NULL;
std::unordered_map<int, std::shared_ptr<HttpData>> Epoll::fd2httpData_;
TimerManager Epoll::timer_manager_;

int Epoll::init(int max_event = MAX_EVENTS) {
  int epoll_fd = ::epoll_create(max_event);
  if (epoll_fd == -1) { 
    LOG_ERROR << "epoll create error";
    exit(-1);
  }
  events_ = new struct epoll_event[max_event];
  return epoll_fd;
}

int Epoll::addfd(int epoll_fd, int fd, __uint32_t events,
                 std::shared_ptr<HttpData> http_data) {
  struct epoll_event event;
  event.events = events;
  event.data.fd = fd;

  fd2httpData_[fd] = http_data;
  int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
  if (ret < 0) {
    LOG_ERROR << "epoll add error";
    fd2httpData_[fd].reset();
    return -1;
  }
  LOG_TRACE << "add a fd: " << fd;
  return 0;
}

int Epoll::modfd(int epoll_fd, int fd, __uint32_t events,
                 std::shared_ptr<HttpData> http_data) {
  struct epoll_event event;
  event.events = events;
  event.data.fd = fd;

  fd2httpData_[fd] = http_data;
  int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
  if (ret < 0) {
    LOG_ERROR << "epoll mod error";
    fd2httpData_[fd].reset();
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
    LOG_ERROR << "epoll del error";
    return -1;
  }
  auto it = fd2httpData_.find(fd);
  if (it != fd2httpData_.end()) {
    fd2httpData_.erase(it);
  }
  return 0;
}

std::vector<std::shared_ptr<HttpData>> Epoll::poll(
  const ServerSocket& server_socket, int max_event, int timeout) {
  
  int event_num = ::epoll_wait(server_socket.epoll_fd_, events_, max_event, timeout);
  if (event_num < 0) {
    LOG_ERROR << "epoll_wait error";
    exit(1);
  }

  std::vector<std::shared_ptr<HttpData>> httpDatas;

  for (int i = 0; i < event_num; ++i) {
    int fd = events_[i].data.fd;
    if (fd == server_socket.listen_fd_) {
      handleConnection(server_socket);
    } else {
      // 出错的描述符，关闭文件描述符
      if ((events_[i].events & EPOLLERR)           // 错误
          || (events_[i].events & EPOLLRDHUP)      // TCP连接被对方关闭，或者对方关闭了写操作
          || (events_[i].events & EPOLLHUP)        // 挂起
         ) {
        LOG_ERROR << "event error";
        auto it = fd2httpData_.find(fd);
        if (it != fd2httpData_.end()) {
          it->second->close_timer();
          // fd2httpData_.erase(it);
        }
        continue;
      }

      auto it = fd2httpData_.find(fd);
      if (it != fd2httpData_.end()) {
        if ((events_[i].events & EPOLLIN) || (events_[i].events & EPOLLPRI)) {
          httpDatas.push_back(it->second);

          // LOG_DEBUG << "http data pointer count: " << it->second.use_count();
          it->second->close_timer();
          // LOG_DEBUG << "http data pointer count: " << it->second.use_count();
          fd2httpData_.erase(it);
          // LOG_DEBUG << "http data pointer count: " << it->second.use_count();
        }
      } else {
        LOG_ERROR << "长连接第二次连接未找到";
        ::close(fd);
        continue;
      }
    }
  }
  return httpDatas;
}

void Epoll::handleConnection(const ServerSocket& server_socket) {
  std::shared_ptr<ClientSocket> temp_client(new ClientSocket());

  while (server_socket.accept(*temp_client) >= 0) {
    setNonblocking(temp_client->fd_);
    
    std::shared_ptr<HttpData> http_data(new HttpData());
    http_data->request_ = std::shared_ptr<HttpRequest>(new HttpRequest());
    http_data->response_ = std::shared_ptr<HttpResponse>(new HttpResponse());

    std::shared_ptr<ClientSocket> client_socket(new ClientSocket());
    client_socket.swap(temp_client);
    http_data->client_socket_ = client_socket;
    http_data->epoll_fd_ = server_socket.epoll_fd_;
    // std::cout << "ready to add fd " << client_socket->fd << std::endl;
    addfd(server_socket.epoll_fd_, client_socket->fd_, DEFAULT_EVENTS, http_data);
    timer_manager_.addTimer(http_data, TimerManager::DEFAULT_TIME_OUT);
  }
  // std::cout << "handle over" << std::endl;
}