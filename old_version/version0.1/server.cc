#include "util.h"

#include <unistd.h> // close
#include <stdlib.h> // for exit
#include <arpa/inet.h> // for struct sockaddr_in

#include <string>


int main(int argc, char** argv) {
  // check command
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  int port = atoi(argv[1]);
  int listenfd = open_listenfd(port);

  while (true) {
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    int connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
    if (connfd < 0) {
      perror("accept error!");
      return -1;
    }

    handle_request(connfd);

    if(close(connfd) < 0) {
      perror("close error!");
      return -1;
    }
  }

  return 0;
}

