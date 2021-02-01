#include<unistd.h> // for read(), write(), close()
#include<sys/socket.h> // socket
#include<arpa/inet.h>
#include<fcntl.h> // open()

#include<string.h>
#include<stdio.h> 

int initServer() {
  // 创建socket
  int fd = socket(PF_INET, SOCK_STREAM, 0); // PF_INET: IPv4, SOCK_STREAM: 流服务（TCP）， 0:选择具体协议，（通常唯一，默认为0）

  if(-1 == fd) {
    perror("创建socket失败\n");
    return -1;
  }
  printf("创建socket成功！\n");

  int optval = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

  // 指定地址
  sockaddr_in addr = {0}; // IPv4专用socket地址结构体
  addr.sin_family = PF_INET;
  addr.sin_port = htons(80);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // 命名
  int ret = bind(fd, (sockaddr*)&addr, sizeof(addr));
  if (-1 == ret) {
    perror("命名socket失败\n");
    close(fd);
    return -1;
  }
  printf("命名socket成功！\n");

  // 监听
  ret = listen(fd, 10);
  if (-1 == ret) {
    perror("监听socket失败\n");
    close(fd);
    return -1;
  }
  printf("监听端口成功！\n");

  return fd;
}

void handle(int fd) {
  char buff[1024*1024] = {0};
  int ret = read(fd, buff, sizeof(buff));
  if (ret > 0) {
    printf("接收到请求: %s\n", buff);
  }
  char fileName[20] = "./";
  sscanf(buff, "GET /%s", fileName + 2);
  printf("解析出来的文件名: %s\n",fileName);

  const char* mime = NULL;
  if (strstr(fileName, ".html")) {
    mime = "text/html";
  } else if (strstr(fileName, ".jpg")) {
    mime = "image/jpg";
  }

  char response[1024*1024] = {0};
  sprintf(response, "HTTP/1.1 200 ok\nContent-Type: %s\n\n", mime);
  int responseLen = strlen(response);
  int fileFd = open(fileName, O_RDONLY);
  int fileLen = read(fileFd, response + responseLen, sizeof(response) - responseLen);
  write(fd, response, responseLen + fileLen);
  close(fileFd);
  sleep(1);
}

int main() {
  int serverfd = initServer();
  
  sockaddr_in client;
  socklen_t client_addrlength = sizeof(client);
  while (true) {
    int clientfd = accept(serverfd, (sockaddr*)&client, &client_addrlength);
    if (-1 == clientfd) {
      perror("服务器崩溃....");
      break;
    }
    handle(clientfd);
    close(clientfd);
  }
}