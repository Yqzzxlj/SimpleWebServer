# A tiny web server on linux

## 简介

使用 C++11 编写的 Web 服务器，支持 GET、HEAD 方法处理静态资源


## Envoirment
+ OS : Ubuntu 20.04
+ Complier: g++ 9.3.0

## 技术要点

+ 使用 Reactor 编程模型，非阻塞 I/O + epoll + 线程池处理高并发请求 
+ 使用状态机解析 HTTP 请求，支持 GET、HEAD 方法。 
+ 使用最小堆结构实现定时器，以支持 HTTP 长连接，定时处理过期请求 
+ 使用双缓冲区技术实现异步日志系统，记录服务器运行状态 
+ 使用 RAII 机制管理指针、互斥器、文件描述符等资源，避免泄漏

## 压测
使用Webbench测试短链接，1000客户端进程，60s，虚拟机4核，4G

QPS ：HEAD方法36621， GET方法12619
