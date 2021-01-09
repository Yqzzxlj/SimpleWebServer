#include "util.h"

#include <unistd.h>     // read(), write(), close()
#include <errno.h>      // errno, EINTR

#include <sys/socket.h> // socket, sockaddr
#include <netdb.h>      // hostent
#include <netinet/in.h>
#include <arpa/inet.h> // for struct sockaddr_in

#include <stdio.h> // for fprintf


#include <fcntl.h> // open
#include <sys/stat.h> // stat
#include <sys/mman.h> // mmap, munmap
#include <sys/wait.h> // wait

#include <sstream>    // stringstream
#include <algorithm>

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

ssize_t readn(int fd, std::string& str) {
  size_t nread = 0;
  while (true) {
    char buff[MAXBUF];
    if ((nread = read(fd, buff, MAXBUF)) < 0) {
      if (errno == EINTR) {
        nread = 0;
      } else {
        perror("read error!");
        return -1;
      }
    } else if (nread == 0) {
      break;
    }
    str += std::string(buff, buff + nread);
  }
  return str.size();
}

ssize_t writen(int fd, const void* usrbuf, size_t n) {
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

ssize_t writen(int fd, const std::string& str) {
  size_t nleft = str.size();
  size_t nwritten = 0;
  const char* ptr = str.c_str();
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) <= 0) {
      if (errno == EINTR) {
        nwritten = 0;
      } else {
        perror("write error!");
        return -1;
      }
    }
    nleft -= nwritten;
    ptr += nwritten;
  }

  return str.size();
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

std::string readline(int fd) {
  ssize_t n, rc;
  char c;
  char buff[MAXBUF];
  char* ptr = buff;
  for (n = 1; n < MAXLINE; ++n) {
  again:
    if ((rc = read(fd, &c, 1)) == 1) {
      *ptr++ = c;
      if (c == '\n') {
        break;
      }
    } else if (rc == 0) {
      *ptr = 0;
      return std::string(buff, buff + n - 1); // EOF
    } else {
      if (errno == EINTR) {
        goto again;
      }
      perror("readline error!");
      return "";
    }
  }
  *ptr = 0;
  return std::string(buff, buff + n);
}

// open_listenfd - open and return a listening socket on port
//   Returns -1 and sets errno on Unix error
int open_listenfd(int port) {

  // Create a socket descriptor
  int listenfd = socket(AF_INET, SOCK_STREAM, 0); 
  if (listenfd < 0) {
    perror("error in create socket.\n");
    return -1;
  }

  // Eliminates "Address already in use" error form bind
  int optval = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) < 0) {
    perror("error in setsockopt.\n");
    return -1;
  }

  // Listenfd will be an endpoint for all requests to port on any IP address for this host
  struct sockaddr_in serveraddr = {0};
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(port);
  
  if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
    perror("error in bind.\n");
    return -1;
  }

  // Make it a listening socket ready to accept connection requests
  if (listen(listenfd, LISTENQ) < 0) {
    perror("error in listen.\n");
    return -1;
  }

  return listenfd;
}

// handle one HTTP request/response transaction
void handle_request(int fd) {
  printf("new request here!\n");
  std::string headline = readline(fd);
  printf("%s", headline.c_str());

  std::stringstream s(headline);
  std::string method;
  std::string uri;
  std::string version;
  s >> method >> uri >> version;
  std::transform(method.begin(), method.end(), method.begin(), ::toupper);

  if (method != "GET") {
    clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
    return;
  }

  read_requesthdrs(fd);

  std::string filename;
  std::string cgiargs;
  bool is_static = parse_uri(uri, filename, cgiargs);

  struct stat sbuf;
  if (stat(filename.c_str(), &sbuf) < 0) {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  if (is_static) { // serve static content
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny coundn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size);
  } else { // Serve dynamic content
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny coundn't run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs);
  }
  printf("hanlded over.\n");
}

void read_requesthdrs(int fd) {
  std::string headline = readline(fd);
  while (headline != "\r\n") {
    headline = readline(fd);
    printf("%s", headline.c_str());
  }
  return;
}

int parse_uri(std::string& uri,
              std::string& filename,
              std::string& cgiargs) {
  if (uri.find("cgi-bin") == uri.npos) { // static content
    cgiargs = "";
    filename = "." + uri;
    if(uri.back() == '/') {
      filename += "index.html";
    }
    return 1;
  } 
  // dynamic content
  size_t it = uri.find('?');

  if (it != uri.npos) {
    cgiargs = uri.substr(it + 1);
    uri = uri.substr(0, it);
  } else {
    cgiargs = "";
  }

  filename = "." + uri;
  return 0; 
}

std::string get_filetype(const std::string& filename) {
  if (filename.find(".html") != filename.npos) {
    return "text/html";
  } else if (filename.find(".gif") != filename.npos) {
    return "image/gif";
  } else if (filename.find(".jpg") != filename.npos) {
    return "image/jpeg";
  }
  return "text/plain";
}

void clienterror(int fd, 
                 const std::string& cause,
                 const std::string& errnum,
                 const std::string& shortmsg,
                 const std::string& longmsg
                ) {
  // Build hte HTTP reaponse body
  std::string body;
  body += "<html><title>Tiny Error</title>";
  body += "<body bgcolor=""ffffff"">\r\n";
  body += errnum + ": " + shortmsg + "\r\n";
  body += "<p>" + longmsg + ": " + cause + "\r\n";
  body += "<hr><em>The Tiny Web server</em>\r\n";

  // print the HTTP response
  writen(fd, "HTTP/1.0 " + errnum + " " + shortmsg + "\r\n");

  writen(fd, "Content-type: text/html\r\n");

  writen(fd, "Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
  writen(fd, body);
}

void serve_static(int fd, const std::string& filename, int filesize) {

  // send response headers to client
  std::string headers;
  headers += "HTTP/1.0 200 OK\r\n";
  headers += "Server: Tiny Web Server\r\n";
  headers += "Content-length: " + std::to_string(filesize) + "\r\n";
  headers += "%sContent-type: " + get_filetype(filename) + "\r\n\r\n";
  writen(fd, headers);

  // send response body to client
  int srcfd = open(filename.c_str(), O_RDONLY, 0);
  char* srcp = (char*)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  close(srcfd);
  writen(fd, srcp, filesize);
  munmap(srcp, filesize);
}

void serve_dynamic(int fd,
                   const std::string& filename,
                   const std::string& cgiargs
                  ) {
  char buf[MAXLINE];
  char* emptyList[] = {NULL};

  // Return first part of HTTP response
  writen(fd, "HTTP/1.0 200 OK\n");
  writen(fd, "Server: Tiny Web Server\n");

  if (fork() == 0) {
    setenv("QUERY_STRING", cgiargs.c_str(), 1);
    dup2(fd, STDOUT_FILENO);
    execve(filename.c_str(), emptyList, environ);
  }
  wait(NULL);
}