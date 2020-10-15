---
layout: second_template
title: 创建高性能索引之高性能索引策略
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: high-performance-mysql-5-chapter-strategy
---

[hash_strategy]:/high-performance-mysql-5-chapter-hash-index

### 索引的优点
--------------------------------------------------
	
- 索引大大减少了服务器需要扫描的数据量

- 索引可以帮助服务器避免排序和临时表

- 索引可以将随机IO变为顺序IO

### 独立的列
--------------------------------------------------

独立的列是指索引不能是表达式的一部分，也不能是函数的参数。

例如有以user_id列的索引，错误的用法：

	select user_id from t_users where user_id + 1 = 10

	select user_id from t_users where function(user_id) = 10

### 前缀索引及索引选择性
--------------------------------------------------

有时我们需要很长的字符列做索引，这会让索引变的很大而且比较时很慢，这时可以采用介绍哈希索引时的[创建自定义哈希索引][hash_strategy],也可采用下面的方案；首先说明一个概念**选择性**

索引的选择性是指**不重复的索引值数量/数据表的记录总数**的比值。索引的选择性越高则查询效率越高。唯一索引的选择性是1，这时最好的索引选择性，性能也最好

对于以字符列做索引，可以让字符的开始部分作为索引，这样可以大大节约索引空间，从而提高索引效率，但这样也会降低索引的选择性。这里的诀窍就是要选择足够长的前缀，同时又不能太长。
我们可以通过测试查询，选择最优的长度。例如：

- 统计相同列值数量

		select count(name) as cnt,name from t_users group by name ORDER BY cnt DESC LIMIT 10;

- 统计相同列值前缀

		select count(name) as cnt,LEFT(name,3) as pref from t_users group by pref ORDER BY cnt DESC LIMIT 10;

计算合适前缀长度的另一个办法：

1. 首先计算完整列的选择性

		select COUNT(DISTINCT name)/COUNT(*) from t_users;

2. 最后计算不同前缀长度的选择性，选出最接近完整列的选择性的前缀长度

		select COUNT(DISTINCT LEFT(name,3))/COUNT(*) AS sel1,
		COUNT(DISTINCT LEFT(name,4))/COUNT(*) AS sel2
		COUNT(DISTINCT LEFT(name,5))/COUNT(*) AS sel3
		COUNT(DISTINCT LEFT(name,6))/COUNT(*) AS sel4
		COUNT(DISTINCT LEFT(name,7))/COUNT(*) AS sel5 from t_users;

3. 创建字符列做索引

		ALTER TABLE t_users ADD KEY (name(4));

**前缀索引是一种能使索引更小、更快的有效办法，但MySQL无法使用前缀索引做ORDER BY 和 GROUP BY，也无法使用前缀索引做覆盖扫描**

### 多列索引
--------------------------------------------------

非常错误的观点：为每个列创建独立的索引

在多个列上创建独立的索引大部分情况下并不能提高查询性能。MySQL5.0和更新的版本引入了“索引合并“的策略，更早的版本只能使用其中某个单列索引，
在更早的版本中像这样的查询`select film_id,actor_id from t_films where actor_id=1 or film_id=1`会导致全表扫描，除非使用UNION的方式，
像这样：

	select film_id,actor_id from t_films where actor_id=1 UNION ALL 
	select film_id,actor_id from t_films where film_id=1 and actor_id <> 1;

在5.0以上版本查询能够使用这两个单列索引，并进行结果合并，但是这样也会很糟糕，对于这种查询有以下几点说明:

- 当出现服务器对多个索引做相交操作（多个and）时，通常意味着需要一个包含所有相关列的多列索引，而不是多个单列索引

- 当服务器需要对多个索引做联合操作时（多个or）时，通常需要耗费大量CPU和内存资源在算法的缓存、排序和合并的操作上。
	特别是当索引的选择度不高，需要合并扫描返回大量数据的时候

- 更重要的是优化器不会把这些计算到“查询成本”中，优化器只关心随机页面读取，这会使得对查询成本被低估，还可能影响查询的并发性

如果在EXPLAIN中看到有索引合并，这时应该警觉性的看看查询语句和表结构，也可以通过optimizer_switch来关闭索引合并功能，也可以使用IGNORE INDEX提示让优化器忽略到某些索引

