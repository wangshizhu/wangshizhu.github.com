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

### 原因
--------------------------------------------------