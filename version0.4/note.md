线程池开启时,多次关闭fd. 解决方法：使用智能指针在堆上创建对象。

使用命名空间，防止覆盖。

监听socket listen_fd 上是不能注册EPOLLONESHOT事件的，否则应用程序只能处理一个客户连接。
后续的客户连接请求将不再触发listenfd上的EPOLLIN事件

非阻塞时，不出错也可能返回-1

出现bug，epoll时只能读取一次，原因是没有写好socket函数，fd默认初始值-1，关闭了一个不相关的fd，0。然后将0分配给clientsocket建立连接，如果服务器没有处理，客户端一直等待响应。

accept socket失败，errno=22， EINVAL	/* Invalid argument */。 猜测指针问题，初始化clientsocket。

socketfd 3，
epollfd 4
client_fd : 5， 6..