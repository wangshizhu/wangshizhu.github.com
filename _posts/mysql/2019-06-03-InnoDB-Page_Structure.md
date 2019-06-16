---
layout: second_template
title: InnoDB数据页结构
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: innoDB-page-structure
---

[row-format]:/innodb-row-format-compact

***
* ### 测试环境 ###
	
	MySQL 5.6

***
* ### 数据页 ###
	
	在[行格式文章][row-format]中提到了一些页的概念,它是InnoDB管理存储空间的基本单位，一个页的大小一般是16KB。InnoDB为了不同的目的而设计了许多种不同类型的页，例如：存放表空间头部信息的页，存放Insert Buffer信息的页，存放INODE信息的页，存放undo日志信息的页等。而我们的数据就存放在官方所称的**索引页**中，通常使用过程中更多称为**数据页**

***
* ### 数据页结构 ###

	|名称|占用空间大小|描述|
	|-|-|-|
	|File Head(文件头部)|38字节|页的一些基本信息|
	|Page Head(页头部)|56字节|数据页的一些基本信息|