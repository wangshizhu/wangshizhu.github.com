---
layout: second_template
title: select、poll、epoll
category: network
tagline: "Supporting tagline"
tags : [network]
permalink: select-poll-epoll
---

[addr]:https://github.com/wangshizhu/network

I/O多路复用模型常用的有select、poll、epoll，实际生产中Windows开发环境使用的select，Linux环境用的是epoll

### select
--------------------------------------------------

select函数签名：
	
	int select(int maxfd, fd_set *readset, fd_set *writeset, fd_set *exceptset, const struct timeval *timeout);
	
* 参数maxfd
	
	当前需要监控的描述符基数，传递需要监控的最大描述符作为参数，但是应该+1，例如当前最大描述符是6，那么传给此函数时应该为7
	
	**表示描述符集合通常用数组的形式，描述符为数组索引，所以描述符基数应该是最大描述符+1**
	
* 参数readset
	
	需要监控的读描述符集合
	
* 参数writeset
	
	需要监控的写描述符集合
	
* 参数exceptset
	
	需要监控的异常描述符集合
	
* 参数timeout
	
	需要传递timeval结构体指针，如下：
		
		struct timeval 
		{ 
			long tv_sec; /* seconds */ 
			
			long tv_usec; /* microseconds */
		};
		
	如果传递为NULL，表示如果没有 I/O 事件发生，则 select 一直等待下去
	
	如果传递为一个非零的值，这个表示等待固定的一段时间后从 select 阻塞调用中返回
	
	如果传递的timeval结构体tv_sec 和 tv_usec 都设置成 0，表示根本不等待，检测完毕立即返回
	
* 返回值
	
	返回值>0：就绪描述符数量
	
	返回值=0：超时
	
	返回值=-1：出错
	
**使用select时如果有事件发生，会重新设置监控的描述符集合**，所以必须按照如下方法使用：

	int network::SelectPoller::ProcessEvent()
	{
	
		fd_set	read_fds;
		fd_set	write_fds;
	
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
	
		read_fds = fd_read_set_;
		write_fds = fd_write_set_;
	
		struct timeval tv;
		// TODO: select 超时时间有待修改
		tv.tv_sec = 1;
		tv.tv_usec = 500;
	
		int num = select(fd_largest_ + 1, &read_fds,fd_write_count_ ? &write_fds : nullptr, nullptr, &tv);
	
		...
	
		return num;
	}
	
套接字描述符就绪条件：

* 可读事件
	
	1. 套接字接收缓冲区有数据可以读
	2. 收到FIN包，对应读取后直接返回0
	3. 监听套接字而言，有连接建立完成
	4. 套接字有错误要处理，对应读取后返回-1，需调用系统错误函数查看、处理

* 可写事件
	
	1. 套接字发送缓冲区足够大
	2. 连接的写半边已经关闭
	3. 套接字有错误要处理，对应写入后返回-1，需调用系统错误函数查看、处理

当有就绪套接字但是应用程序没有处理、或者没有处理完，例如只处理了接收缓冲区里一部分数据，那么到下一个tick时，这个套接字依然是就绪的、有事件要处理的

**select 有个缺点，所支持的文件描述符的个数是有限的。在 Linux 系统中，select 的默认最大值为 1024**

### poll
--------------------------------------------------

**poll I/O多路复用技术避免了select的缺点，突破了描述符数量的限制**，poll函数签名如下：

	int poll(struct pollfd *fds, unsigned long nfds, int timeout);
	
* 参数fds
	
	pollfd结构数组，pollfd结构如下:
		
		struct pollfd 
		{ 
			int fd; /* file descriptor */ 
			
			short events; /* events to look for */ 
			
			short revents; /* events returned */ 
		};
		
	结构中fd：待监控的描述符
	
	结构中events：期望监控的事件
	
		#define POLLIN 0x0001 /* any readable data available */
		#define POLLPRI 0x0002 /* OOB/Urgent readable data */
		#define POLLOUT 0x0004 /* file descriptor is writeable */
		
	POLLIN 和 POLLOUT 可以表示读和写事件
	
	结构中revents：和 select 非常不同的地方在于，poll 每次检测之后的结果不会修改原来的传入值，而是将结果保留在 revents 字段中，这样就不需要每次检测完都得重置待检测的描述字和感兴趣的事件
	
* 参数nfds
	
	数组 fds 的大小
	
* 参数timeout
	
	<0：在有事件发生之前永远等待
	
	=0：不阻塞线程，立即返回
	
	大于0：等待指定的毫秒数后返回
	
* 返回值

	返回值>0：检测到事件的描述符个数
	
	返回值=0：超时
	
	返回值=-1：出错
	
上面只是列出几个事件，下面列出了所有事件及对应解释：

![Alt text][poll_event]

[poll_event]: assets/themes/my_blog/img/poll_event.jpg

具体poll使用可以参考[github][addr]上的网络库代码，下载时选择master分支

### epoll
--------------------------------------------------

epoll 和 poll很相似，和poll不同的是epoll提供了触发机制，一种是条件触发level-triggered，一种是边缘触发edge-triggered，先看一下epoll如何使用，epoll提供了3个函数：

* epoll_create(int size)
	
	函数签名：
		
		int epoll_create(int size);
	
	
	从 Linux 2.6.8 开始，epoll_create函数的参数 size 被自动忽略，
	最初的epoll_create实现中，size参数告诉内核添加到epoll实例中的文件描述符数量。内核使用该信息作为初始分配内部数据结构空间大小，
	如果调用者的使用超过了size，则内核会在必要的情况下分配更多的空间。目前，这个参数已经不再需要了，内核动态的改变数据结构的大小，但是为了保证向后兼容性
	但是该值仍需要一个大于 0 的整数
	
	若成功创建返回一个大于0的值，表示epoll实例；若返回-1表示出错
	
	**如果这个 epoll 实例不再需要，例如服务器正常关机，需要调用 close() 方法释放 epoll 实例，这样系统内核可以回收 epoll 实例所分配使用的内核资源**
	
* epoll_ctl
	
	通过调用 epoll_ctl 向epoll实例增加、修改、删除监控的事件
	
	函数签名：
	
		int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
		
	- 参数epfd
		
		使用epoll_create创建的epoll实例
		
	- 参数op
		
		有三个选项EPOLL_CTL_ADD、EPOLL_CTL_DEL、EPOLL_CTL_MOD
		
	- 参数fd
	
		要监控的套接字
		
	- 参数event
		
		表示注册的事件类型，并且可以在这个结构体里设置用户需要的数据，其中最为常见的是使用联合结构里的 fd 字段，表示事件所对应的文件描述符：
		
			typedef union epoll_data 
			{ 
				void *ptr; 
				int fd; 
				uint32_t u32; 
				uint64_t u64; 
			} epoll_data_t;
			 
			struct epoll_event 
			{ 
				uint32_t events; /* Epoll events */ 
				
				epoll_data_t data;
			};
			
		需要检测的事件类型有：
		
		+ EPOLLIN：表示对应的文件描述字可以读
		+ EPOLLOUT：表示对应的文件描述字可以写
		+ EPOLLRDHUP：表示套接字的一端已经关闭，或者半关闭
		+ EPOLLHUP：表示对应的文件描述字被挂起
		+ EPOLLERR：表示对应的文件描述字有错误
		+ EPOLLET：设置为 edge-triggered
		+ EPOLLLT：设置为 level-triggered，默认为 level-triggered
		
	- 返回值
		
		若成功返回0；若返回-1表示出错，可以调用错误函数查看错误码

* epoll_wait
			
	事件分发函数，函数签名：
		
		int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
		
	- 参数epfd
		
		使用epoll_create创建的epoll实例
		
	- 参数events
		
		返回给用户空间需要处理的 I/O 事件，这是一个数组，数组的大小由 epoll_wait 的返回值决定，这个数组的每个元素都是一个需要待处理的 I/O 事件，
		其中 events 表示具体的事件类型，事件类型取值和 epoll_ctl 可设置的值一样，这个 epoll_event 结构体里的 data 值就是在 epoll_ctl 那里设置的 data，
		也就是用户空间和内核空间调用时需要的数据
		
	- 参数maxevents
		
		表示 epoll_wait 可以返回的最大事件值
		
	- 参数timeout
		
		大于0表示阻塞调用超时时间
		
		-1表示不超时
		
		0表示立即返回，即使没有任何 I/O 事件发生
		
这个3个函数的用法可以参考[github][addr]上的网络库代码，下载时选择master分支

上面提到边缘触发和条件触发，下面看一下它们的含义：

* 边缘触发：表示事件就绪时，假设对事件没做处理，内核不会反复通知事件就绪
	
	接收缓存区没有可读数据，则epoll_wait不会返回EPOLLIN，如果此时缓冲区有可读数据，则epoll_wait会返回**一次EPOLLIN**，如果这次抛出的事件，
	并没有读完缓冲区的数据，此时的缓冲区数据会保留，直到对端再次发送数据进入接收缓存区，内核才会继续通知可读事件
	
	发送缓冲区如果有空余，这种模式只会抛出一次EPOLLOUT事件

* 条件触发：表示事件就绪时，假设对事件没做处理，内核会反复通知事件就绪
	
	缓存区没有可读数据，则epoll_wait不会返回EPOLLIN，如果此时缓冲区有可读数据，则epoll_wait会**持续返回EPOLLIN**
	
	**select和poll属于条件触发，例如缓冲区有10字节可读数据时，每个tick只读一个字节，那么下一个tick依然抛出可读事件**
	
	发送缓冲区如果有空余，这种模式会持续抛出EPOLLOUT事件
	
**poll 和 epoll 之间还有一个重要的区别，涉及到效率问题，主要出现在 epoll返回的是有事件发生的数组,而poll返回的是准备好的个数,
每次poll函数返回都要遍历注册的描述符数组 尤其是数量越大遍历次数就越多**