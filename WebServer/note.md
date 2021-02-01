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

不允许指针指向不完整的类类型  只有声明没有定义 前向声明，在命名空间里声明

timeout为-1时，epoll_wait 会被阻塞，导致无法即时处理超时事件。

share_ptr 导致资源不能httpdata正常释放, 使用share_ptr. use_count()查看

webbench测试完就会宕机。
连接建立，若某一端关闭连接，而另一端仍然向它写数据，第一次写数据后会收到RST响应，此后再写数据，内核将向进程发出SIGPIPE信号，通知进程此连接已经断开。而SIGPIPE信号的默认处理是终止程序，导致上述问题的发生。应该忽略SIGPIPE信号。

由于Nagle算法的存在，在某些时候导致小的及时传输的数据包不能被迅速的传递到对方，从而造成应用程序的性能问题，但如果将Nagle的算法Disable了，那么有些时候会出现1个有效字节40个包头（IP头+TCP头）的这样低效的包，增加网络负担从而影响网络性能

提高并发度，减少文件读写。因为是静态文件，存在char[]里，可大幅提高请求处理时间。