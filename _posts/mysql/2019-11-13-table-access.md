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

--------------------------------------------------
### 复杂查询

上面提到了一些访问方法，但有时查询条件有点复杂，例如使用了多个索引等等

* 可能只利用单个索引
	
	`SELECT * FROM single_table WHERE key1 = 'abc' AND key2 > 1000;`对于这样的查询通常优化器一般会根据single_table表的统计数据来判断到底使用哪个条件到对应的二级索引中查询扫描的行数会更少，
	选择那个扫描行数较少的条件到对应的二级索引中查询（关于如何比较的细节我们后边的章节中会唠叨）。
	然后将从该二级索引中查询到的结果经过回表得到完整的用户记录后再根据其余的WHERE条件过滤记录。一般来说，
	等值查找比范围查找需要扫描的行数更少，也就是ref的访问方法一般比range好，但这也不总是一定的，也可能采用ref访问方法的那个索引列的值为特定值的行数特别多，
	假设优化器决定使用idx_key1索引进行查询，整个查询过程可以分为两个步骤：

	1. 使用二级索引定位记录的阶段，也就是根据条件key1 = 'abc'从idx_key1索引代表的B+树中找到对应的二级索引记录。
	2. 回表阶段，也就是根据上一步骤中找到的记录的主键值进行回表操作，也就是到聚簇索引中找到对应的完整的用户记录，再根据条件key2 > 1000到完整的用户记录继续过滤。将最终符合过滤条件的记录返回给用户。

	因为二级索引的节点中的记录只包含索引列和主键，所以在步骤1中使用idx_key1索引进行查询时只会用到与key1列有关的搜索条件，其余条件，比如key2 > 1000这个条件在步骤1中是用不到的，
	只有在步骤2完成回表操作后才能继续针对完整的用户记录中继续过滤
	
	像这样的查询`SELECT * FROM single_table WHERE key2 > 100 AND common_field = 'abc';`只能利用索引idx_key2，二级索引不包含字段common_field，查询过程还是上面的步骤：
	
	1. 根据条件key2 > 100确定查询范围
	2. 回表，找到对应完整用户记录
	3. 根据条件common_field = 'abc'筛选符合条件的用户记录

* 条件范围简化
	
	有些条件在业务层完全可以做到简化，对于单个索引的范围查询包括AND或者OR,例如`SELECT * FROM single_table WHERE key2 > 100 AND key2 > 200;`这样的查询语句，
	我们在业务层完全可以简化成`SELECT * FROM single_table WHERE key2 > 200;`
	
	`SELECT * FROM single_table WHERE key2 > 100 OR key2 > 200;` 可以简化成`SELECT * FROM single_table WHERE key2 > 100;`
	
* 条件包含索引但无法使用
	
	上面提到的一个查询`SELECT * FROM single_table WHERE key2 > 100 AND common_field = 'abc';`当我们把AND换成OR，像这样`SELECT * FROM single_table WHERE key2 > 100 OR common_field = 'abc';`
	如果使用索引idx_key2，条件简化后`key2 > 100 OR TRUE`，因为使用的是OR，所以最终条件是`where TRUE；`
	
	这也就说如果强制使用idx_key2执行查询的话，对应的范围区间就是(-∞, +∞)，也就是需要将全部二级索引的记录进行回表，这个代价肯定比直接全表扫描都大了。
	也就是说**一个使用到索引的搜索条件和没有使用该索引的搜索条件使用OR连接起来后是无法使用该索引的**
	
--------------------------------------------------
### 索引合并

上面提到对于多列索引出现在条件语句中时可能使用单个索引，但可能在一个查询中使用到多个二级索引，对于这种使用到多个索引来完成一次查询的执行方法称之为：index merge

* Intersection交集合并
	
	下面列出了能够使用交集合并的**必要不充分条件**
	
	- 二级索引列是等值匹配的情况，对于联合索引来说，在联合索引中的每个列都必须等值匹配，不能出现只匹配部分列的情况
			
			yes:
			SELECT * FROM single_table WHERE key1 = 'a' AND key_part1 = 'a' AND key_part2 = 'b' AND key_part3 = 'c';
			
			no:
			SELECT * FROM single_table WHERE key1 > 'a' AND key_part1 = 'a' AND key_part2 = 'b' AND key_part3 = 'c';
			
			SELECT * FROM single_table WHERE key1 = 'a' AND key_part1 = 'a';
			
		对于InnoDB的二级索引来说，记录先是按照索引列进行排序，如果该二级索引是一个联合索引，那么会按照联合索引中的各个列依次排序。
		而二级索引的用户记录是由索引列 + 主键构成的，二级索引列的值相同的记录可能会有好多条，
		这些索引列的值相同的记录又是按照主键的值进行排序的。所以重点来了，之所以在二级索引列都是等值匹配的情况下才可能使用Intersection索引合并，
		是因为只有在这种情况下根据二级索引查询出的结果集是按照主键值排序的
		
		就第一条查询语句来讲：
		
		从idx_key1中获取到已经排好序的主键值：1、3、5
		从idx_key_part中获取到已经排好序的主键值：2、3、4

		那么求交集的算法是这样：逐个取出这两个结果集中最小的主键值，如果两个值相等，则加入最后的交集结果中，否则丢弃当前较小的主键值，再取该丢弃的主键值所在结果集的后一个主键值来比较，
		直到某个结果集中的主键值用完了，那么这个过程：

		1. 先取出这两个结果集中较小的主键值做比较，因为1 < 2，所以把idx_key1的结果集的主键值1丢弃，取出后边的3来比较

		2. 因为3 > 2，所以把idx_key2的结果集的主键值2丢弃，取出后边的3来比较

		3. 因为3 = 3，所以把3加入到最后的交集结果中，继续两个结果集后边的主键值来比较

		4. 后边的主键值也不相等，所以最后的交集结果中只包含主键值3

		这个取交集的算法时间复杂度是O(n)，这里可以设想一下如果并不是有序的两个集合取交集的算法及时间复杂度
		
	- 主键列可以是范围匹配
		
			SELECT * FROM single_table WHERE id > 100 AND key1 = 'a';
			
		之所以是主键列可以是范围匹配，这里和二级索引的存储数据方式有关，上面已经提到二级索引的用户记录是由索引列 + 主键构成，所以取出key1 = 'a'的记录再从这些记录中按照条件找出id > 100 的记录
		
	上边说的条件1和条件2只是发生Intersection索引合并的必要条件，不是充分条件。也就是说即使条件1、条件2成立，也不一定发生Intersection索引合并，
	**优化器只有在单独根据搜索条件从某个二级索引中获取的记录数太多，导致回表开销太大，而通过Intersection索引合并后需要回表的记录数大大减少时才会使用Intersection索引合并**
	
	所以就条件1中的查询可能只采用一个二级索引查询出符合条件的用户记录，再回表查询出这些记录的全部数据，再根据另外条件筛选，最后返回给用户
	
* Union并集合并

	下面列出了能够使用并集合并的**必要不充分条件**

	- 二级索引列是等值匹配的情况，对于联合索引来说，在联合索引中的每个列都必须等值匹配，不能出现只出现匹配部分列的情况
			
			yes:
			SELECT * FROM single_table WHERE key1 = 'a' OR ( key_part1 = 'a' AND key_part2 = 'b' AND key_part3 = 'c');
			
			no:
			SELECT * FROM single_table WHERE key1 > 'a' OR (key_part1 = 'a' AND key_part2 = 'b' AND key_part3 = 'c');
	
			SELECT * FROM single_table WHERE key1 = 'a' OR key_part1 = 'a';
	
	- 主键列可以是范围匹配
	
	- 使用Intersection索引合并的搜索条件
	
		搜索条件的某些部分使用Intersection索引合并的方式得到的主键集合和其他方式得到的主键集合取交集，例如：
	
			SELECT * FROM single_table WHERE key_part1 = 'a' AND key_part2 = 'b' AND key_part3 = 'c' OR (key1 = 'a' AND key3 = 'b');
	
		优化器可能采用这样的方式来执行这个查询：
	
		先按照搜索条件key1 = 'a' AND key3 = 'b'从索引idx_key1和idx_key3中使用Intersection索引合并的方式得到一个主键集合。
	
		再按照搜索条件key_part1 = 'a' AND key_part2 = 'b' AND key_part3 = 'c'从联合索引idx_key_part中得到另一个主键集合。
	
		采用Union索引合并的方式把上述两个主键集合取并集，然后进行回表操作，将结果返回给用户
	
	**查询条件符合了这些情况也不一定就会采用Union索引合并，优化器只有在单独根据搜索条件从某个二级索引中获取的记录数比较少，通过Union索引合并后进行访问的代价比全表扫描更小时才会使用Union索引合并**

* Sort-Union排序并集合并

	下边这个查询就无法使用到Union索引合并：
	
		SELECT * FROM single_table WHERE key1 < 'a' OR key3 > 'z'
			
	这是因为根据key1 < 'a'从idx_key1索引中获取的二级索引记录的主键值不是有序的，根据key3 > 'z'从idx_key3索引中获取的二级索引记录的主键值也不是有序的，所以可以这样：
	
	1. 根据key1 < 'a'条件从idx_key1二级索引中获取记录，并按照记录的主键值进行排序
	
	2. 再根据key3 > 'z'条件从idx_key3二级索引中获取记录，并按照记录的主键值进行排序
	
	3. 使用时间复杂度为O(n)的算法对两个集合求并集
	
	**上述这种先按照二级索引记录的主键值进行排序，之后按照Union索引合并方式执行的方式称之为Sort-Union索引合并**，这种Sort-Union索引合并比单纯的Union索引合并多了一步对二级索引记录的主键值排序的过程，
	Sort-Union的适用场景是单独根据搜索条件从某个二级索引中获取的记录数比较少，这样即使对这些二级索引记录按照主键值进行排序的成本也不会太高
	
* 为什么没有Sort-Intersection
	
	Sort-Union的适用场景是：单独根据搜索条件从某个二级索引中获取的记录数比较少，这样即使对这些二级索引记录按照主键值进行排序的成本也不会太高 
	
	Intersection索引合并的适用场景是：单独根据搜索条件从某个二级索引中获取的记录数太多，导致回表开销太大，合并后可以明显降低回表开销
	
	如果有Sort-Intersection后，就需要为大量的二级索引记录按照主键值进行排序，这个成本可能比回表查询都高了，所以也就没有Sort-Intersection，这也是对于**在联合索引中的每个列都必须等值匹配**这样要求的原因