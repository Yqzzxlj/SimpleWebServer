#include <sys/types.h>
#include <string>

const int MAXLINE = 8192; // max text line length
const int MAXBUF = 8192;  // max I/O buffer size
const int LISTENQ = 1024; // second argument to listen()

// io functions
ssize_t readn(int fd, void* usrbuf, size_t n);
ssize_t readn(int fd, std::string& str);

ssize_t writen(int fd, const void* usrbuf, size_t n);
ssize_t writen(int fd, const std::string& str);

ssize_t readline(int fd, void* usrbuf, size_t maxLen);
ssize_t readline(int fd, std::string& str);


// client/server help functions
int open_listenfd(int portno);

// handle HTTP request
void handle_request(int fd);

// read request headers
void read_requesthdrs(int fd);

// parse the request is static or not. 
int parse_uri(std::string& uri,
              std::string& filename,
              std::string& cgiargs);

// serve static requests
void serve_static(int fd,
                  const std::string& filename,
                  int filesize);

std::string get_filetype(const std::string& filename);

// serve dynamic requests
void serve_dynamic(int fd,
                   const std::string& filename,
                   const std::string& cgiargs);

// return the error response to client.
void clienterror(int fd,
                 const std::string& cause,
                 const std::string& errnum,
                 const std::string& shortmsg,
                 const std::string& longmsg);

