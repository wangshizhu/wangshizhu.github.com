---
layout: second_template
title: 字符集和比较规则之二
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: charset-collation-two
---

[one]:/charset-collation-one

***
* ### 测试环境 ###
	
	MySQL 5.6

	win10 64位

***
* ### 字符集在客户端和服务器通信中的应用 ###

	在上一篇[文章][one]中介绍了几个字符集、字符集和比较规则在MySQL中各级别的应用。我们日常编码中都遇见过乱码的情况，例如编码时使用的是utf8字符集，编码一个字符‘我’，而解码时使用的是gbk字符集，这时就出现了乱码。简单解释一下这个过程：

	1. 字符‘我’按照utf8字符集编码的十六进制是0xE68891，按照gbk字符集解码时，第一个字节0xE6，它的值大于0x7F（十进制：127），说明是两字节编码，继续读一字节后是0xE688，然后从gbk编码表中查找字节为0xE688对应的字符，发现是字符'鎴'

	2. 继续读一个字节0x91，它的值也大于0x7F，说明是两字节编码，但是后面没有了，所以这是半个字符

	3. 最后字符‘我’按照gbk字符集解码被解析成一个字符'鎴'和半个字符

	MySQL客户端在和MySQL服务器进行通信时，要经历如下步骤：

	1. MySQL客户端按照所在操作系统的字符集进行编码，向服务器发送对应二进制数据

	2. MySQL服务器收到消息后，按照MySQL系统变量character_set_client所指的字符集进行解码

	3. MySQL服务器处理请求时把字符串（character_set_client所指的字符集解码的结果）向character_set_connection所指的字符集转换，如果某个列使用的字符集和character_set_connection代表的字符集不一致的话，还需要进行一次字符集转换

	4. 处理完请求按照character_set_results所指的字符集进行转换，同时把二进制数据返回给客户端

	5. MySQL客户端按照所在操作系统的字符集进行解码，打印到屏幕上——就是我们看到

	先解释一下上述步骤涉及到三个MySQL系统变量：

	1. character_set_client		服务器解码请求时使用的字符集

	2. character_set_connection 	服务器处理请求时会把请求字符串从character_set_client转为character_set_connection

	3. character_set_results	服务器向客户端返回数据时使用的字符集

	再看一下我的环境这三个系统变量所指定的字符集：

		mysql> show variables like 'character_set_client';
		+----------------------+-------+
		| Variable_name        | Value |
		+----------------------+-------+
		| character_set_client | gbk   |
		+----------------------+-------+
		1 row in set (0.00 sec)

		mysql> show variables like 'character_set_connection';
		+--------------------------+-------+
		| Variable_name            | Value |
		+--------------------------+-------+
		| character_set_connection | gbk   |
		+--------------------------+-------+
		1 row in set (0.00 sec)

		mysql> show variables like 'character_set_results';
		+-----------------------+-------+
		| Variable_name         | Value |
		+-----------------------+-------+
		| character_set_results | gbk   |
		+-----------------------+-------+
		1 row in set (0.00 sec)

	我的操作系统所使用的字符集gbk，通常客户端所使用的字符集和当前操作系统一致，不同操作系统使用的字符集可能不一样，如下：

	- 类Unix系统使用的是utf8
	- Windows使用的是gbk

	从上面分析的从MySQL客户端发送请求到收到MySQL服务器返回的结果需要经历多次的字符集转换(这里假设了每个系统变量的字符集都不一样)：

	- 服务器认为客户端发送过来的请求是用character_set_client编码的

		假设你的客户端采用的字符集和 character_set_client 不一样的话，这就会出现意想不到的情况

	- character_set_connection只是服务器在将请求的字节串从character_set_client转换为character_set_connection时使用，该字符集包含的字符范围一定涵盖请求中的字符，不然会导致有的字符无法使用character_set_connection代表的字符集进行编码。例如把character_set_client设置为utf8，把character_set_connection设置成ascii，那么此时如果从客户端发送一个汉字到服务器，那么服务器无法使用ascii字符集来编码这个汉字，就会向用户发出一个警告

	- 处理请求时向查询列的字符集转换

	- 服务器将把得到的结果集使用character_set_results编码后发送给客户端

	- 客户端按照自己的字符集转换从MySQL服务器返回的结果

		假设客户端采用的字符集和 character_set_results 不一样的话，这就可能会出现客户端无法解码结果集的情况，结果就是在屏幕上出现乱码

	**字符集的来回转换可能各种问题，在实际中尽量都把 character_set_client 、character_set_connection、character_set_results 这三个系统变量设置成和客户端使用的字符集一致的情况，这样减少了很多无谓的字符集转换**

***
* ### 设置字符集 ###

	设置上述所提的三个MySQL系统变量，可以通过如下命令：

		SET NAMES 字符集名称;

	与上面设置语句等价的是：

		SET character_set_client = 字符集名;
		SET character_set_connection = 字符集名;
		SET character_set_results = 字符集名;

	同时也可以在启动客户端的时候就把character_set_client、character_set_connection、character_set_results这三个系统变量的值设置成一样的，那我们可以在启动客户端的时候指定一个叫default-character-set的启动选项：

		[client]
		default-character-set=utf8

	它起到的效果和执行一遍`SET NAMES utf8`是一样的，都会将三个系统变量的值设置成utf8



