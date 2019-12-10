---
layout: second_template
title: InnoDB数据统计
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: innodb-statistics
---

[cost]:/cost
[separate_innodb_table_space]:/separate-innodb-table-space

在[查询成本文章][cost]提到了统计数据，从存储这些统计数据的方式上分为两种：

1. 永久性的统计数据
	
	统计数据存储在磁盘
	
2. 非永久性的统计数据
	
	统计数据存储在内存中
	
同时提供了变量来决定用户选择哪种方式存储，可以通过下面查询查看这个系统变量：

	show variables like '%innodb_stats_persistent%';
	+--------------------------------------+-------+
	| Variable_name                        | Value |
	+--------------------------------------+-------+
	| innodb_stats_persistent              | ON    |
	| innodb_stats_persistent_sample_pages | 20    |
	+--------------------------------------+-------+
	
在MySQL 5.6.6之前，innodb_stats_persistent的值默认是OFF，也就是说InnoDB的统计数据默认是存储到内存的，之后的版本中innodb_stats_persistent的值默认是ON，也就是统计数据默认被存储到磁盘中

InnoDB默认是以表为单位来收集和存储统计数据的，在创建和修改表的时候通过指定STATS_PERSISTENT属性来指明该表的统计数据存储方式：

	CREATE TABLE table_name (...) Engine=InnoDB, STATS_PERSISTENT = (1|0);
	
	ALTER TABLE table_name Engine=InnoDB, STATS_PERSISTENT = (1|0);
	
当STATS_PERSISTENT=1时，表明该表的统计数据永久的存储到磁盘上，当STATS_PERSISTENT=0时，表明该表的统计数据临时的存储到内存中。如果我们在创建表时未指定STATS_PERSISTENT属性，那默认采用系统变量innodb_stats_persistent的值作为该属性的值

### 永久性的统计数据
--------------------------------------------------

当选择在磁盘上保存统计数据时，这些统计数据其实是以通用的方式——以表的方式，存储在mysql数据库下，可以通过下面查询查看：

	show tables from mysql like 'innodb%';
	+---------------------------+
	| Tables_in_mysql (innodb%) |
	+---------------------------+
	| innodb_index_stats        |
	| innodb_table_stats        |
	+---------------------------+
	
1. innodb_table_stats存储了关于表的统计数据，每一条记录对应着一个表的统计数据。 
2. innodb_index_stats存储了关于索引的统计数据，每一条记录对应着一个索引的一个统计项的统计数据

我们先看一下表innodb_table_stats都有哪些列：

	SHOW COLUMNS FROM innodb_table_stats;
	+--------------------------+---------------------+------+-----+-------------------+-----------------------------+
	| Field                    | Type                | Null | Key | Default           | Extra                       |
	+--------------------------+---------------------+------+-----+-------------------+-----------------------------+
	| database_name            | varchar(64)         | NO   | PRI | NULL              |                             |
	| table_name               | varchar(64)         | NO   | PRI | NULL              |                             |
	| last_update              | timestamp           | NO   |     | CURRENT_TIMESTAMP | on update CURRENT_TIMESTAMP |
	| n_rows                   | bigint(20) unsigned | NO   |     | NULL              |                             |
	| clustered_index_size     | bigint(20) unsigned | NO   |     | NULL              |                             |
	| sum_of_other_index_sizes | bigint(20) unsigned | NO   |     | NULL              |                             |
	+--------------------------+---------------------+------+-----+-------------------+-----------------------------+

|							|
|字段名						|描述						|
|database_name				|数据库名					|
|table_name					|表名						|
|last_update				|本条记录最后更新时间		|
|n_rows						|表中记录的条数				|
|clustered_index_size		|表的聚簇索引占用的页面数量	|
|sum_of_other_index_sizes	|表的其他索引占用的页面数量	|

在[查询成本文章][cost]文章提到了n_rows，它的值并不精确，计算这个值的算法大致是这样的：

1. 按照一定算法选取几个叶子节点页面，计算每个页面中主键值记录数量
2. 计算平均一个页面中主键值的记录数量乘以全部叶子节点的数量就算是该表的n_rows值

这个值的精确与否取决于统计时采样的页面数量，MySQL提供系统变量innodb_stats_persistent_sample_pages来控制使用永久性的统计数据时，
计算统计数据时采样的页面数量。该值设置的越大，统计出的n_rows值越精确，但是统计耗时也就最久；该值设置的越小，统计出的n_rows值越不精确，但是统计耗时特别少。
所以在实际使用是需要权衡利弊，从上面的查询`'%innodb_stats_persistent%'`看出系统变量的默认值是20，同样我们可以在创建表或修改表时指定属于这个表的采样页面数量：

	CREATE TABLE table_name (...) Engine=InnoDB, STATS_SAMPLE_PAGES = 具体的采样页面数量;
	
	ALTER TABLE table_name Engine=InnoDB, STATS_SAMPLE_PAGES = 具体的采样页面数量;

在创建表的语句中如果没有指定STATS_SAMPLE_PAGES属性的话，将默认使用系统变量innodb_stats_persistent_sample_pages的值作为该属性的值

clustered_index_size和sum_of_other_index_sizes这两个字段的数据统计依赖表空间的知识，关于表空间可以参考这篇[文章][separate_innodb_table_space]，这个统计过程分为以下步骤：

1. 从数据字典里找到表的各个索引对应的根页面位置
	
	系统表SYS_INDEXES里存储了各个索引对应的根页面信息
	
2. 从根页面的Page Header里找到叶子节点段和非叶子节点段对应的Segment Header

	在每个索引的根页面的Page Header部分都有两个字段：
	
	- PAGE_BTR_SEG_LEAF：表示B+树叶子段的Segment Header信息

	- PAGE_BTR_SEG_TOP：表示B+树非叶子段的Segment Header信息

3. 从叶子节点段和非叶子节点段的Segment Header中找到这两个段对应的INODE Entry结构

4. 从INODE ENTRY 结构找到零散页面及FREE、NOT_FULL、FULL链表的基节点

5. 统计零散页面及3个链表的Length，Length表示占用区的大小，每个区对应64个页，最终就能统计出这个段占用了多少个页

6. 聚簇索引的叶子节点段和非叶子节点段占用的页面数量之和便是clustered_index_size的值，同样的算法可以统计出sum_of_other_index_sizes的值

未全部使用完的区也会统计，所以说聚簇索引和其他的索引实际占用的页面数可能比这两个值要小一些

通过如下命令查看innodb_index_stats表中的各个列：

	show columns from mysql.innodb_index_stats;
	+------------------+---------------------+------+-----+-------------------+-----------------------------+
	| Field            | Type                | Null | Key | Default           | Extra                       |
	+------------------+---------------------+------+-----+-------------------+-----------------------------+
	| database_name    | varchar(64)         | NO   | PRI | NULL              |                             |
	| table_name       | varchar(64)         | NO   | PRI | NULL              |                             |
	| index_name       | varchar(64)         | NO   | PRI | NULL              |                             |
	| last_update      | timestamp           | NO   |     | CURRENT_TIMESTAMP | on update CURRENT_TIMESTAMP |
	| stat_name        | varchar(64)         | NO   | PRI | NULL              |                             |
	| stat_value       | bigint(20) unsigned | NO   |     | NULL              |                             |
	| sample_size      | bigint(20) unsigned | YES  |     | NULL              |                             |
	| stat_description | varchar(1024)       | NO   |     | NULL              |                             |
	+------------------+---------------------+------+-----+-------------------+-----------------------------+
	
|字段名|描述|
|-|-|
|database_name|数据库名|
|table_name|表名|
|index_name|索引名|
|last_update|本条记录最后更新时间|
|stat_name|统计项的名称|
|stat_value|对应的统计项的值|
|sample_size|为生成统计数据而采样的页面数量|
|stat_description|对应的统计项的描述|

这个表的主键是(database_name,table_name,index_name,stat_name)，其中的stat_name是指统计项的名称，也就是说innodb_index_stats表的每条记录代表着一个索引的一个统计项，具体统计项如下：

* n_leaf_pages：表示该索引的叶子节点占用多少页面

* size：表示该索引共占用多少页面

* n_diff_pfxNN：表示对应的索引列不重复的值有多少

	NN可以被替换为01、02、03... 这样的数字。例如一个联合索引idx_key_part来说：
	
	- n_diff_pfx01表示的是统计key_part1这单单一个列不重复的值有多少

	- n_diff_pfx02表示的是统计key_part1、key_part2这两个列组合起来不重复的值有多少

	- n_diff_pfx03表示的是统计key_part1、key_part2、key_part3这三个列组合起来不重复的值有多少

	- n_diff_pfx04表示的是统计key_part1、key_part2、key_part3、id这四个列组合起来不重复的值有多少

对于普通的二级索引，并不能保证它的索引列值是唯一的，此时只有在索引列上加上主键值才可以区分两条索引列值都一样的二级索引记录。
对于主键和唯一二级索引则没有这个问题，它们本身就可以保证索引列值的不重复，所以也不需要再统计一遍在索引列后加上主键值的不重复值有多少

从对这两个表的数据更新方式上可以分为两种：

* 自动
	
	开启innodb_stats_auto_recalc，这个系统变量默认是开启的：
	
		show variables like '%innodb_stats_auto_recalc%';
		+--------------------------+-------+
		| Variable_name            | Value |
		+--------------------------+-------+
		| innodb_stats_auto_recalc | ON    |
		+--------------------------+-------+
		
	这个系统变量决定着服务器是否自动重新计算统计数据，每个表都维护了一个变量，该变量记录着对该表进行增删改的记录条数，如果发生变动的记录数量超过了表大小的10%，
	并且自动重新计算统计数据的功能是开启的，那么服务器会重新进行一次统计数据的计算，并且更新innodb_table_stats和innodb_index_stats表。
	不过自动重新计算统计数据的过程是**异步**发生的，也就是即使表中变动的记录数超过了10%，自动重新计算统计数据也不会立即发生
	
	同样的我们也可以单独为某个表设置是否自动重新计算统计数的属性，设置方式就是在创建或修改表的时候通过指定STATS_AUTO_RECALC属性来指明该表的统计数据存储方式：
	
		CREATE TABLE table_name (...) Engine=InnoDB, STATS_AUTO_RECALC = (1|0);
	
		ALTER TABLE table_name Engine=InnoDB, STATS_AUTO_RECALC = (1|0);
	
	当STATS_AUTO_RECALC=1时，表明该表自动重新计算统计数据，当STATS_AUTO_RECALC=0时，表明不想让该表自动重新计算统计数据。如果在创建表时未指定STATS_AUTO_RECALC属性，
	那默认采用系统变量innodb_stats_auto_recalc的值作为该属性的值
	
* 手动

	如果innodb_stats_auto_recalc系统变量的值为OFF，可以手动调用ANALYZE TABLE语句来更新统计信息：
	
		ANALYZE TABLE table_name;
		
	**ANALYZE TABLE语句会立即重新计算统计数据，也就是这个过程是同步的，在表中索引多或者采样页面特别多时这个过程可能会特别慢，所以要把握好手动调用的时机**
	
	innodb_table_stats和innodb_index_stats表就相当于一个普通的表一样，我们能对它们做增删改查操作。这也就意味着我们可以手动更新某个表或者索引的统计数据，修改了这两个表中的数据时，
	需要让MySQL查询优化器重新加载更改过的数据，可以通过如下命令：
	
		FLUSH TABLE table_name;
		
### 非永久性统计数据
--------------------------------------------------

当系统变量innodb_stats_persistent的值设置为OFF时，之后创建的表的统计数据默认就都是非永久性的了，或者直接在创建表或修改表时设置STATS_PERSISTENT属性的值为0，
那么该表的统计数据就是非永久性的

与永久性的统计数据不同，非永久性的统计数据采样的页面数量是由innodb_stats_transient_sample_pages控制的，这个系统变量的默认值是8，可以通过下面查询查看：

	show variables like '%innodb_stats_transient_sample_pages%';
	+-------------------------------------+-------+
	| Variable_name                       | Value |
	+-------------------------------------+-------+
	| innodb_stats_transient_sample_pages | 8     |
	+-------------------------------------+-------+

另外，由于非永久性的统计数据经常更新，所以导致MySQL查询优化器计算查询成本的时候依赖的是经常变化的统计数据，也就会生成经常变化的执行计划

在做<font color="#dd0000">索引列不重复的值的数量</font>统计时MySQL提供了系统变量innodb_stats_method，它主要是为处理索引列NULL值情况，可以通过如下命令查看：

	show variables like '%innodb_stats_method%';
	+---------------------+-------------+
	| Variable_name       | Value       |
	+---------------------+-------------+
	| innodb_stats_method | nulls_equal |
	+---------------------+-------------+
	
这个系统变量相当于在计算某个索引列不重复值的数量时如何对待NULL值，有三个可选值：

* nulls_equal：认为所有NULL值都是相等的。这个值也是innodb_stats_method的默认值

	如果某个索引列中NULL值特别多的话，这种统计方式会让优化器认为某个列中平均一个值重复次数特别多，所以倾向于不使用索引进行访问

* nulls_unequal：认为所有NULL值都是不相等的

	如果某个索引列中NULL值特别多的话，这种统计方式会让优化器认为某个列中平均一个值重复次数特别少，所以倾向于使用索引进行访问

* nulls_ignored：直接把NULL值忽略掉

索引列不重复的值的数量对于查询优化器采取什么样的决策起到至关重要的作用，因为通过它可以计算出在索引列中平均一个值重复多少行，它的应用场景主要有两个：

* 单表查询中单点区间太多
	
	例如这样的查询：
	
		SELECT * FROM tbl_name WHERE key IN ('xx1', 'xx2', ..., 'xxn');
		
	当IN里的参数数量过多时，采用index dive的方式直接访问B+树索引去统计每个单点区间对应的记录的数量就太耗费性能了，
	所以直接依赖统计数据中的平均一个值重复多少行来计算单点区间对应的记录数量
	
* 连接查询时，如果有涉及两个表的等值匹配连接条件，该连接条件对应的被驱动表中的列又拥有索引时，则可以使用ref访问方法来对被驱动表进行查询
	
	例如这样的查询：
	
		SELECT * FROM t1 JOIN t2 ON t1.column = t2.key WHERE ...;

	在真正执行对t2表的查询前，t1.comumn的值是不确定的，所以不能通过index dive的方式直接访问B+树索引去统计每个单点区间对应的记录的数量，
	所以也只能依赖统计数据中的平均一个值重复多少行来计算单点区间对应的记录数量
	
**NULL值带来的问题不止这些，所以在实际生产中尽量不要使用NULL值**

### 最后
--------------------------------------------------

* InnoDB以表为单位来收集统计数据，这些统计数据可以是基于磁盘的永久性统计数据，也可以是基于内存的非永久性统计数据

* innodb_stats_persistent控制着使用永久性统计数据还是非永久性统计数据

* innodb_stats_persistent_sample_pages控制着永久性统计数据的采样页面数量

* innodb_stats_transient_sample_pages控制着非永久性统计数据的采样页面数量

* innodb_stats_auto_recalc控制着是否自动重新计算统计数据

* 在创建和修改表时通过指定STATS_PERSISTENT、STATS_AUTO_RECALC、STATS_SAMPLE_PAGES的值来控制相关统计数据属性

* innodb_stats_method决定着在统计某个索引列不重复值的数量时如何对待NULL值