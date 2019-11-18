---
layout: second_template
title: InnoDB单表访问
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: innodb-table-access
---

[B_Tree_index]:/B+Tree-index

MySQL把执行查询语句的方式称之为访问方法

访问方法从使用索引角度分两种：

* 全表扫描
* 使用索引
	
	使用索引有细分几种：
	
	- 针对主键或唯一二级索引的等值查询
	- 针对普通二级索引的等值查询
	- 针对索引列的范围查询
	- 直接扫描整个索引

介绍这些访问方法之前先创建一张表：

	CREATE TABLE single_table (
		id INT NOT NULL AUTO_INCREMENT,
		key1 VARCHAR(100),
		key2 INT,
		key3 VARCHAR(100),
		key_part1 VARCHAR(100),
		key_part2 VARCHAR(100),
		key_part3 VARCHAR(100),
		common_field VARCHAR(100),
		PRIMARY KEY (id),
		KEY idx_key1 (key1),
		UNIQUE KEY idx_key2 (key2),
		KEY idx_key3 (key3),
		KEY idx_key_part(key_part1, key_part2, key_part3)
	) Engine=InnoDB CHARSET=utf8mb4;
		
这张表有5个索引，1个聚簇索引，4个二级索引（包含1个唯一索引、一个联合索引），对应着5颗B+树
	
--------------------------------------------------
### const访问方法

通过主键或者**唯一**二级索引列与常数的**等值**比较来定位**一条**记录的访问方法定义为：const

补充说明：这种const访问方法只能在主键列或者唯一二级索引列和一个常数进行等值比较时才有效，如果主键或者唯一二级索引是由多个列构成的话，索引中的每一个列都需要与常数进行等值比较，这个const访问方法才有效

对于唯一二级索引来说，查询该列为NULL值的情况比较特殊，
因为唯一二级索引列并不限制 NULL 值的数量，所以上述语句可能访问到多条记录，所以不可以使用const访问方法来执行

const访问方法的几个正确、错误使用例子：
		
	yes:
	SELECT * FROM single_table WHERE id = 1;
	SELECT * FROM single_table WHERE key2 = 2;
	
	no:
	SELECT * FROM single_table WHERE key2 IS NULL;
	SELECT * FROM single_table WHERE id >= 1;
	SELECT * FROM single_table WHERE key2 >= 1;
		
对于`SELECT * FROM single_table WHERE key2 = 2;`这样的语句：使用二级索引并且返回了二级索引之外的数据会触发**回表**，关于回表可以参考这篇[文章][B_Tree_index]

--------------------------------------------------
### ref访问方法和ref_or_null访问方法

搜索条件为普通二级索引列（不包括唯一二级索引）与常数等值比较，采用二级索引来执行查询的访问方法称为：ref

与上面的const访问方法的区别是当使用普通二级索引进行常数等值比较时有可能查找到多条记录，而const访问方法是定位**一条**记录

下面列出了可能使用ref访问方法的几种情况：

* 二级索引列值为NULL
	
	不论是普通的二级索引，还是唯一二级索引，它们的索引列对包含NULL值的数量并不限制，所以采用`key IS NULL`这种形式的搜索条件最多只能使用ref的访问方法
	
* 包含多列的联合索引，只要是最左边的连续索引列是与常数的等值比较

	多列的联合索引左边的连续索引列与常数的等值比较就**可能**采用ref的访问方法，之所以是可能用到ref访问方法是因为在二级索引等值比较时匹配的记录数较少时触发的回表成本低于全表扫描，
	如果索引列的基数接近于1，说明重复值很少，如果基数非常小说明重复值很多，那么可能还没有全表扫描的效率高
	
	同时如果最左边的连续索引列并不全部是等值比较时，它的访问方法就不能称为ref
	
下面列出了ref访问方法几个正确、错误使用例子：

	yes:
	SELECT * FROM single_table WHERE key1 = 'MySQL';
	
	SELECT * FROM single_table WHERE key_part1 = 'god like';
	
	SELECT * FROM single_table WHERE key_part1 = 'god like' AND key_part2 = 'legendary';
	
	SELECT * FROM single_table WHERE key_part1 = 'god like' AND key_part2 = 'legendary' AND key_part3 = 'penta kill';
	
	no:
	SELECT * FROM single_table WHERE key_part1 = 'god like' AND key_part2 > 'legendary';
	
ref_or_null访问方法从字面上就可以理解，即：对二级索引列的常数等值比较，同时查找该列值为NULL的记录，例如这样`SELECT * FROM single_table WHERE key1 = 'abc' OR key1 IS NULL;`
	
--------------------------------------------------
### range访问方法

上面的查询都是基于常数等值比较，有时对索引列进行范围查找，MySQL把这种利用索引进行范围匹配的访问方法称之为：range

进行范围匹配中的**索引**可以是聚簇索引，也可以是二级索引

这些都属于range访问方法：

	SELECT * FROM single_table WHERE key2 >= 1 AND key2 <= 10;
	
	SELECT * FROM single_table WHERE key2 IN (1,3,5) OR (key2 >= 100 AND key2 <= 200);
	
--------------------------------------------------
### index访问方法

MySQL把采用遍历二级索引记录的执行方式称之为：index。像这样`SELECT key_part1, key_part2, key_part3 FROM single_table WHERE key_part2 = 'abc';`

从这条查询语句的where条件看出key_part2并不是联合索引idx_key_part最左索引列，所以无法使用ref或者range访问方法来执行这个语句。但是这个查询符合下边这两个条件：

* 查询列表只有3个列：key_part1, key_part2, key_part3，而索引idx_key_part又包含这三个列
* 搜索条件中只有key_part2列。这个列也包含在索引idx_key_part中

对于一个二级索引的B+树中存储着索引列对应的值，所以像这样的查询可以直接遍历这个二级索引的B+树叶子结点，而且并不会触发回表

--------------------------------------------------
### all访问方法

使用全表扫描的执行方式称之为：all。即：直接扫描聚簇索引
