---
layout: second_template
title: 创建高性能索引
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: high-performance-mysql-5-chapter
---

* 索引

	索引优化应该是对查询性能优化最有效的手段，效果立竿见影。索引是在存储引擎层而不是服务器层

	创建一个包含多列的索引，那么列的顺序也很重要，因为MySQL只能高效的使用索引的最左前缀。
	同时创建一个包含两列的索引和创建两个只包含一列的索引大不相同。

	存储引擎以不同的方式使用B-Tree索引，B-Tree通常意味所有值都是按顺序存储的，从索引的根节点进行搜索，根节点的存放着指向子节点的指针，
	通过比较节点页的值和要查找的值可以找到合适的指针进入下层子节点。

	**B-Tree索引适用于全键值、键值范围、键前缀查找，键前缀只适用于最左前缀**
