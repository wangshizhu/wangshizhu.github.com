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
	|Infimum + Supremum(最小记录和最大记录)|26字节|两条系统插入的行记录|
	|User Records(用户真实记录)|不确定|实际存储的行记录数据|
	|Free Space(空闲空间)|不确定|数据页中未使用的空间|
	|Page Directory(页面目录)|不确定|数据页中某些记录的相对位置|
	|File Trailer(文件尾部)|8字节|校验页是否完整|


	我们的数据就存储在User Records这部分空间，User Record在页的建立起初并不存在，User Records这部分空间是从空闲空间Free Space逐渐划分而来，当Free Space部分的空间全部被User Records部分替代掉之后，也就意味着这个页使用完了，如果还有新的记录插入的话，就需要去申请新的页。我们先创建一张表，并插入一些数据去解释各个部分的作用。如下：

		USE test;
		CREATE TABLE `t_test_page` (
			c1 INT NOT NULL DEFAULT '0',
			c2 CHAR NOT NULL DEFAULT '',
			PRIMARY KEY(c1)
		)ENGINE INNODB CHARSET=ASCII ROW_FORMAT=COMPACT;

		INSERT INTO t_test_page(c1,c2) VALUES (0,"a"),(1,"b"),(2,"c");

	**把c1列设置为主键是因为在compact行格式中就没必要为我们去创建那个所谓的 row_id 隐藏列了**