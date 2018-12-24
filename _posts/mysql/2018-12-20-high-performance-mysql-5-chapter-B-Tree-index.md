---
layout: second_template
title: 创建高性能索引之B-Tree索引
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: high-performance-mysql-5-chapter-B-Tree-index
---

* B-Tree索引

	索引优化应该是对查询性能优化最有效的手段，效果立竿见影。索引是在存储引擎层而不是服务器层

	创建一个包含多列的索引，那么列的顺序也很重要，因为MySQL只能高效的使用索引的最左前缀。
	同时创建一个包含两列的索引和创建两个只包含一列的索引大不相同。

	存储引擎以不同的方式使用B-Tree索引，B-Tree通常意味所有值都是按顺序存储的，从索引的根节点进行搜索，根节点的存放着指向子节点的指针，
	通过比较节点页的值和要查找的值可以找到合适的指针进入下层子节点。

	**B-Tree索引适用于全键值、键值范围、键前缀查找，键前缀只适用于最左前缀**

	**索引对多个值进行排序的依据是CREATE TABLE语句中定义索引时列的顺序**

		CREATE TABLE IF NOT EXISTS `t_people` (
		  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
		  last_name VARCHAR(50) NOT NULL DEFAULT '',
		  first_name VARCHAR(50) NOT NULL DEFAULT '',
		  dob DATE NOT NULL,
		  gender ENUM('m','f') NOT NULL,
		  PRIMARY KEY (`id`),
		  KEY `key1` (`last_name`,`first_name`,dob)
		) ENGINE=INNODB CHARSET=utf8;	

		ALTER TABLE `th_content` DROP INDEX `idx_audit_status`;
		ALTER TABLE `th_content` ADD INDEX `idx_status_audit` (`status`, `audit_time`);

	以上面表格为例：

	- 全值匹配

	全值匹配指的是和索引中所有列进行匹配，即匹配last_name、first_name、dob

	- 匹配最左前缀

	以上表为例我们可以查找last_name为zhang的人，即索引的第一列last_name

	- 匹配列前缀

	也可以只匹配某一列的值得开头部分，以上表为例，查找以z开头的姓的人，只使用索引的第一列

	- 匹配范围值

	以上表为例，查找zhang和wang之间的人，只使用索引的第一列

	- 精确匹配某一列并范围匹配另一列

	以上表为例，即第一列last_name全匹配，第二列first_name范围匹配

* B-Tree索引的限制

	- 如果不是按照索引的最左列开始查找，则无法使用索引。以上表为例，`KEY 'key1' ('last_name','first_name',dob) `无法用于查找**名字**(即用first_name索引)为wuji的人，也无法用于查找特定生日的人(即用dob索引)，这两列索引均不是最左索引，也无法用于**姓**以某个字母结尾的人

	- 不能跳索引。以上表为例，`KEY 'key1' ('last_name','first_name',dob) `无法使用**姓zhang并且某个特定出生日期的人**，不使用first_name,则Mysql只能使用索引第一列

	- 如果查询中有某一列使用了范围查询，则其右边的所有列都无法使用索引优化查找。以上表为例，例如:`where last_name="zhang" AND first_name LIKE 'w%' AND dob='1976-12-23'`这个查询只能使用索引的前两列即last_name、first_name。使用过程中如果这个范围固定我们完全可以使用多个等于条件替代范围查找





