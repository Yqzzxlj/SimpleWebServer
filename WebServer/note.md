+ one loop per thread 每个IO线程有一个事件循环。
+ EventLoop的主要作用是运行事件循环。每个线程只能有一个EventLoop。
+ Channel对象，属于EventLoop对象，负责一个文件描述符的IO事件分发。

+ EPoll IO复用。 获得活跃的Channel对象，即在fd上某事件就绪。

+ TimerNode 为每个HttpData加上时间点。可以设置过期时间，并根据当前时间判断该Node是否生效。
+ TimerManager 管理整个TimerNode队列，负责添加节点和将队列中的过时Node删除。当一个节点被删除时，不是直接从堆中删除，而是将deleted_状态设为true，在其过期时间点再pop出堆。

+ 使用epoll_create1的优点是它允许你指定标志,close-on-exec(在执行另一个进程时文件描述符会自动关闭).