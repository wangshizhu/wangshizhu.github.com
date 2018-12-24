---
layout: second_template
title: 创建高性能索引之高性能索引策略
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: high-performance-mysql-5-chapter-strategy
---

* 索引的优点
	
	- 索引大大减少了服务器需要扫描的数据量

	- 索引可以帮助服务器避免排序和临时表

	- 索引可以将随机IO变为顺序IO

* 高性能索引策略