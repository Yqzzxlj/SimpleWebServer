#include "util.h"

#include <unistd.h>
#include <stdio.h> // for fprintf
#include <stdlib.h> // for exit
#include <arpa/inet.h> // for struct sockaddr_in
#include <sys/stat.h> // stat
#include <fcntl.h> // open
#include <sys/mman.h> // mmap, munmap
#include <sys/wait.h> // wait

#include <string.h>

#include <string>

// doit函数处理一个HTTP事务。
void doit(int fd);

// 读取请求头，read request headers
void read_requesthdrs(int fd);

// 将uri解析为一个文件名和一个可选的CGI参数字符串
int parse_uri(char* uri, char* filename, char* cgiargs);

// 处理静态请求
void serve_static(int fd, char* filename, int filesize);

void get_filetype(char* filename, char* filetype);

// 派生一个子进程并在子进程的上下文中运行一个CGI程序。
void serve_dynamic(int fd, char* filename, char* cgiargs);

// 检查一些明显的错误，并返回给客户端。
void clienterror(int fd, char* cause, char* errnum, char* shortmsg, char* longmsg);

int main(int argc, char** argv) {
  int listenfd;
  int connfd;
  int port;

  // check command
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  port = atoi(argv[1]);

  listenfd = open_listenfd(port);
  while (true) {
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    if ((connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen)) < 0) {
      return -1;
    }

    doit(connfd);

    if(close(connfd) < 0) {
      return -1;
    }
  }

  return 0;
}

// doit - handle one HTTP request/response transaction
void doit(int fd) {
  char buf[MAXLINE];

  char method[MAXLINE];
  char uri[MAXLINE];
  char version[MAXLINE];


  readline(fd, buf, MAXLINE);
  sscanf(buf, "%s %s %s", method, uri, version);
  if (strcasecmp(method, "GET")) {
    clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
    return;
  }

  read_requesthdrs(fd);
  char filename[MAXLINE];
  char cgiargs[MAXLINE];
  bool is_static = parse_uri(uri, filename, cgiargs);

  struct stat sbuf;
  if (stat(filename, &sbuf) < 0) {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  if (is_static) { // serve static content
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny coundn't read the file");
      return;
    }
    serve_static(fd, filename,sbuf.st_size);
  } else { // Serve dynamic content
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny coundn't run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs);
  }
}

void read_requesthdrs(int fd) {
  char buf[MAXLINE];
  readline(fd, buf, MAXLINE);
  while (strcmp(buf, "\r\n")) {
    // printf("I'm there!");
    readline(fd, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

int parse_uri(char* uri, char* filename, char* cgiargs) {
  if (!strstr(uri, "cgi-bin")) { // static content
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if(uri[strlen(uri) - 1] == '/') {
      strcat(filename, "home.html");
    }
    return 1;
  } 
  // dynamic content
  char* ptr = index(uri, '?');
  if (ptr) {
    strcpy(cgiargs, ptr + 1);
    *ptr = '\0';
  } else {
    strcpy(cgiargs, "");
  }
  strcpy(filename, ".");
  strcat(filename, uri);
  return 0; 
}

void get_filetype(char* filename, char* filetype) {
  if (strstr(filename, ".html")) {
    strcpy(filetype, "text/html");
  } else if (strstr(filename, ".gif")) {
    strcpy(filetype, "image/gif");
  } else if (strstr(filename, ".jpg")) {
    strcpy(filetype, "image/jpeg");
  } else {
    strcpy(filetype, "text/plain");
  }
}

void clienterror(int fd, char* cause, char* errnum, char* shortmsg, char* longmsg) {
  char buf[MAXLINE];
  char body[MAXLINE];

  // Build hte HTTP reaponse body
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  // print the HTTP response
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  writen(fd, buf, strlen(buf));
  writen(fd, body, strlen(body));
}

void serve_static(int fd, char* filename, int filesize) {
  char filetype[MAXLINE];
  char buf[MAXBUF];

  // send response headers to client
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  writen(fd, buf, strlen(buf));

  // send response body to client
  int srcfd = open(filename, O_RDONLY, 0);
  char* srcp = (char*)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  close(srcfd);
  writen(fd, srcp, filesize);
  munmap(srcp, filesize);
}

void serve_dynamic(int fd, char* filename, char* cgiargs) {
  char buf[MAXLINE];
  char* emptyList[] = {NULL};

  // Return first part of HTTP response
  sprintf(buf, "HTTP/1.0 200 OK\n");
  writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\n");
  writen(fd, buf, strlen(buf));

  if (fork() == 0) {
    setenv("QUERY_STRING", cgiargs, 1);
    dup2(fd, STDOUT_FILENO);
    execve(filename, emptyList, environ);
  }
  wait(NULL);
}