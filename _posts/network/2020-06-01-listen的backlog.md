---
layout: second_template
title: listen的backlog
category: network
tagline: "Supporting tagline"
tags : [network]
permalink: listen-backlog
---

[listen_backlog]:http://veithen.io/2014/01/01/how-tcp-backlog-works-in-linux.html

服务器程序的监听函数listen的第二个参数backlog，对这个参数的解读只知道这个参数越大，并发数目理论上也会越大，这个参数究竟代表什么意思，网上文章众说纷纭，有几种说法：

* Kernel会为LISTEN状态的socket维护一个队列，其中存放SYN RECEIVED和ESTABLISHED状态的套接字，backlog就是这个队列的大小

* Kernel会为LISTEN状态的socket维护两个队列，一个是SYN RECEIVED状态，另一个是ESTABLISHED状态，而backlog就是这两个队列的大小之和

* Kernel会为LISTEN状态的socket维护两个队列，一个是SYN RECEIVED状态，另一个是ESTABLISHED状态，backlog是队列ESTABLISHED的长度

有一篇比较详细的[文章][listen_backlog]，这篇文章给出的解释是：

当一个应用使用listen系统调用让socket进入LISTEN状态时，它需要为该套接字指定一个backlog。backlog通常被描述为连接队列的限制，由于TCP使用的3次握手，
连接在到达ESTABLISHED状态之前经历中间状态SYN RECEIVED，并且可以由accept系统调用返回到应用程序。这意味着TCP / IP协议栈有两个选择来为LISTEN状态的套接字实现backlog队列：

* 一种就是两种状态在一个队列
	
	这个方案的实现，队列大小由backlog参数确定。 当收到SYN数据包时，它发送回SYN/ACK数据包，并将连接添加到队列。 当接收到相应的ACK时，连接将其状态改变为已建立。 
	这意味着队列可以包含两种不同状态的连接：SYN RECEIVED和ESTABLISHED。 只有处于后一状态的连接才能通过accept syscall返回给应用程序

* 一种是**分别**在一个队列
	
	一个是未完成连接的队列即SYN RECEIVED状态队列，一个是ESTABLISHED状态队列，当接收到3次握手中的ACK分组时，将它们移动到ESTABLISHED状态队列。 
	显而易见，accept系统调用只是简单地从ESTABLISHED状态队列中取出连接，在这个队列的设计里，backlog决定着ESTABLISHED状态队列长度
	
历史上，BSD 派生系统实现的TCP使用第一种方法。 第一种方法意味着当达到最大backlog时，系统将不再响应于SYN分组发送回SYN/ACK分组。 通常，TCP的实现将简单地丢弃SYN分组，使得客户端重试，

在更新版本的Linux上，是和上面不同的。在listen的 man page中所提到的：
**在Linux内核2.2之后，socket backlog参数的形为改变了，现在它指等待accept的完全建立的套接字的队列（ESTABLISHED状态队列）长度，而不是不完全连接请求的数量。 
不完全连接的长度可以使用/proc/sys/net/ipv4/tcp_max_syn_backlog设置**

这就意味中在更新版本的Linux系统上，**有两个队列，一个是由系统设置长度的不完全连接队列（SYN队列），
一个是由参数backlog指定长度的完全连接队列（ESTABLISHED状态队列或者称为ACCEPT队列）**

### 验证结论
--------------------------------------------------

Linux版本：3.10.0-514.26.2.el7.x86_64

验证方案：

* 创建TCP服务
* 设置backlog参数为一个比较小的数，如：3
* TCP服务不调用ACCEPT函数，为了保证ACCEPT队列为满载状态
* 创建10个TCP客户端

服务端程序：

	./test0306 --listen_backlog=3 --no_accept

客户端程序：

	./bin/client -p 5701 --dest_host="IP" --dest_port=5700  >/dev/null 2>&1 &
	./bin/client -p 5702 --dest_host="IP" --dest_port=5700  >/dev/null 2>&1 &
	./bin/client -p 5703 --dest_host="IP" --dest_port=5700  >/dev/null 2>&1 &
	./bin/client -p 5704 --dest_host="IP" --dest_port=5700  >/dev/null 2>&1 &
	./bin/client -p 5705 --dest_host="IP" --dest_port=5700  >/dev/null 2>&1 &
	./bin/client -p 5706 --dest_host="IP" --dest_port=5700  >/dev/null 2>&1 &
	./bin/client -p 5707 --dest_host="IP" --dest_port=5700  >/dev/null 2>&1 &
	./bin/client -p 5708 --dest_host="IP" --dest_port=5700  >/dev/null 2>&1 &
	./bin/client -p 5709 --dest_host="IP" --dest_port=5700  >/dev/null 2>&1 &
	./bin/client -p 5710 --dest_host="IP" --dest_port=5700  >/dev/null 2>&1 &
	
查看服务端连接状态：

	netstat -alepn|grep 5700
	
	tcp        4      0 0.0.0.0:5700            0.0.0.0:*               LISTEN      17353/./test0306    
	tcp        0      0 *.*.*.*:5700       *.*.*.*:29750    ESTABLISHED -                   
	tcp        0      0 *.*.*.*:5700       *.*.*.*:29754    ESTABLISHED -                   
	tcp        0      0 *.*.*.*:5700       *.*.*.*:29749    ESTABLISHED -                   
	tcp        0      0 *.*.*.*:5700       *.*.*.*:29751    ESTABLISHED -
	
ESTABLISHED状态的连接有4个，可是backlog参数是`--listen_backlog=3`，这所以多一个，是因为内核的源码对ACCEPT队列长度和backlog的判断是类似这样的：

	if(ACCEPT队列长度 > backlog)
	{
		goto ...
	}
	
	// 创建子套接字
	...
	
所以造成ESTABLISHED状态的连接有4个

