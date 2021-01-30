#pragma once

#include <sys/types.h>
#include <string>

// std::string trim functions
std::string& ltrim(std::string& str);

std::string& rtrim(std::string& str);

std::string& trim(std::string& str);


int setNonblocking(int fd);
void setReuse(int fd);
void handle_for_sigpipe();

ssize_t readn(int fd, void* buff, int n);
ssize_t writen(int fd, void* buff, int n);
