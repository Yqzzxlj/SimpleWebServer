#include <sys/types.h>
#include <string>

const int MAXLINE = 8192; // max text line length
const int MAXBUF = 8192;  // max I/O buffer size
const int LISTENQ = 1024; // second argument to listen()

// io functions
ssize_t readn(int fd, void* usrbuf, size_t n);
ssize_t writen(int fd, void* usrbuf, size_t n);
ssize_t readline(int fd, void* usrbuf, size_t maxLen);

// client/server help functions
int open_clientfd(char* hostname, int portno);
int open_listenfd(int portno);

