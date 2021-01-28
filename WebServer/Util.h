#pragma once
#include <sys/types.h>

ssize_t readn(int fd, void* buff, int n);
ssize_t writen(int fd, void* buff, int n);