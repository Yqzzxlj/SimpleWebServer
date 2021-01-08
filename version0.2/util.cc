#include "util.h"

#include <unistd.h>     // read(), write(),
#include <errno.h>      // errno, EINTR

#include <sys/socket.h> // socket, sockaddr
#include <netdb.h>      // hostent
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>     // bzero(), bcopy()
ssize_t readn(int fd, void* usrbuf, size_t n) {
  size_t nleft = n;
  size_t nread = 0;
  char* bufp =  (char*)usrbuf;

  while (nleft > 0) {
    if ((nread = read(fd, usrbuf, nleft)) < 0) {
      if (errno == EINTR) { // interrupted by sig handler return
        nread = 0;          // call read() again
      } else {
        return -1;          // errno set by read()
      }
    } else if (nread == 0) {
      break;                // EOF
    }
    nleft -= nread;
    bufp += nread;
  }
  return (n - nleft);       // return >= 0
}

ssize_t writen(int fd, void* usrbuf, size_t n) {
  size_t nleft = n;
  size_t nwritten = 0;
  char* bufp = (char*)usrbuf;

  while (nleft > 0) {
    if ((nwritten = write(fd, usrbuf, nleft)) <= 0) {
      if (errno == EINTR) { // interrupted by sig handler return
        nwritten = 0;       // call write() again
      } else {
        return -1;          // errno set by write()
      }
    }
    nleft -= nwritten;
    bufp += nwritten;
  }
  return n;
}

ssize_t readline(int fd, void* usrbuf, size_t maxLen) {
  ssize_t n, rc;
  char c;
  char* bufp = (char*)usrbuf;

  for (n = 1; n < maxLen; ++n) {
  again:
    if ((rc = read(fd, &c, 1)) == 1) {
      *bufp++ = c;
      if (c == '\n') {
        break;
      }
    } else if (rc == 0) {
      *bufp = 0;
      return n - 1;       // EOF
    } else {
      if (errno == EINTR) {
        goto again;
      }
      return -1;
    }
  }
  *bufp = 0;
  return n;
}

// open_clientfd - open connection to server at <hostname, port>
//   and return a socket descriptor read for reading and writing.
//   Returns -1 and sets errno on Unix error
//   Returns -2 and sets h_error on DNS(gethostbyname) error.
int open_clientfd(char* hostname, int port) {
  int clientfd;
  struct hostent* hp;
  struct sockaddr_in serveraddr;

  // Create a socket descriptor
  if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return -1;
  }

  // Fill in the server's IP address and port
  if ((hp = gethostbyname(hostname)) == NULL) {
    return -2;
  }
  bzero((char*)&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char*)hp->h_addr_list[0], (char*)&serveraddr.sin_addr.s_addr, hp->h_length);
  serveraddr.sin_port = htons(port);

  // Establish a connection with the server
  if(connect(clientfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
    return -1;
  }

  return clientfd;
}

// open_listenfd - open and return a listening socket on port
//   Returns -1 and sets errno on Unix error
int open_listenfd(int port) {
  int listenfd;
  int optval = 1;
  struct sockaddr_in serveraddr;

  // Create a socket descriptor
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return -1;
  }

  // Eliminates "Address already in use" error form bind
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) < 0) {
    return -1;
  }

  // Listenfd will be an endpoint for all requests to port on any IP address for this host
  bzero((char*)&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(port);
  
  if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
    return -1;
  }

  // Make it a listening socket ready to accept connection requests
  if (listen(listenfd, LISTENQ) < 0) {
    return -1;
  }

  return listenfd;
}