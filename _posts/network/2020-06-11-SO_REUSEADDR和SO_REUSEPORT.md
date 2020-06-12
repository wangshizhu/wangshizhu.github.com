---
layout: second_template
title: SO_REUSEADDR和SO_REUSEPORT
category: network
tagline: "Supporting tagline"
tags : [network]
permalink: SO_REUSEADDR&SO_REUSEPORT
---

[stack_overflow]:https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ/14388707#14388707
[socket_in_a_bind]:https://blog.heroku.com/sockets-in-a-bind
[bind_before_connect]:https://idea.popcount.org/2014-04-03-bind-before-connect/

在前几章做测试时发现如果CTRL+C杀掉服务器端时，对于ESTABLISHED的连接状态会切换为TIME_WAIT，如果这时重启服务器端程序会发现启动失败，错误发生在调用bind()失败，如下：

	tcp        0      0 *.*.*.*:5700       *.*.*.*:43296    TIME_WAIT   0          0          -

错误log如下：

	log:error,function: CreateTcpServer, line_num: 81, msg: bind faild errno:98
	
错误码98对应的描述为EADDRINUSE——Address already in use

### bind函数延伸
--------------------------------------------------

在解释出现上述问题原因前，先了解一下bind函数

**bind()通常用于即将监听的socket，因此内核需要确保【源地址、源端口】不与任何人共享。当以这种形式使用此技术时，总共不可能建立超过64k（即最大临时端口范围）的传出连接。
之后，尝试调用bind()将失败并显示 EADDRINUSE错误-所有源端口都将繁忙**

【源地址、源端口】通过该bind()函数设置。通过connect()函数设置【目标地址、目的端口】

UDP是无连接协议，因此无需连接即可使用UDP socket。但允许将它们连接起来，在某些情况下对于您的代码和常规应用程序设计非常有利。
在无连接模式下，首次通过其发送数据时**未显式绑定**的UDP socket通常由系统**自动绑定**，因为未绑定的UDP socket无法接收任何对端回复的数据。

TCP是面向连接的协议，对于**未绑定**的TCP socket也是如此，它会在连接之前**自动绑定**

显示的对socket实施bind时，可以将源端口设置为0，当设置为0时，就意味着端口的选择交给内核，在临时端口范围内找到一个可用的端口，对于源地址也可以不指定，即：0.0.0.0，
与端口不同的是，socket实际上可以bind到“任何地址”，这意味着“所有本地接口的所有源IP地址”。如果稍后再连接套接字，则**系统必须选择特定的源IP地址，并且同时绑定到任何本地IP地址**。
系统将选择一个适当的源地址，并将any绑定**替换为对所选源IP地址的绑定**。

假设一台机器有2个接口（192.168.0.1、10.0.0.1）并且端口选择为21，那么当使用any绑定源地址时，系统将这两个IP 进行bind，即：

	bind 192.168.0.1:21
	bind 10.0.0.1:21
	

默认情况下，没有两个socket可以bind到源地址和源端口的**相同组合**。只要源端口不同，源地址实际上就无关紧要

综上，下面列出了选择21为端口、2个IP（192.168.0.1、10.0.0.1）所有bind可行方案（默认情况下）：

* 先让socketA bind 0.0.0.0:21
	
	socketB **不可以**bind 192.168.0.1:21
	
	socketC **不可以**bind 10.0.0.1:21
	
* 先让socketA bind 192.168.0.1:21
	
	socketB **不可以**bind 0.0.0.0:21
	
	socketC **可以**bind 10.0.0.1:21
	
* 先让socketA bind 10.0.0.1:21
	
	socketB **不可以**bind 0.0.0.0:21
	
	socketC **可以**bind 192.168.0.1:21
	
有了上面这些知识，那么上一节提到的问题是就有答案了，原因是由于bind在5700端口、any ip地址的socket依然没有被释放，有个远端还与之建立着连接，这个连接的状态为TIME_WAIT，在这状态持续60秒后
，socket被系统回收，再次启动服务端程序就可以了

有问题就有解决方案，使用如下方法

### SO_REUSEADDR
--------------------------------------------------

如果SO_REUSEADDR在bind套接字之前，在该套接字上启用此选项，则该套接字可以成功bind，除非与另一个bind到【源地址、源端口】的**完全相同**的套接字冲突

**SO_REUSEADDR主要改变搜索冲突时处理通配符地址0.0.0.0的方式**