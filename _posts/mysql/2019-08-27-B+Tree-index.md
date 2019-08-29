---
layout: second_template
title: B+Tree索引的使用
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: B+Tree-index
---

[B+Tree]:/B+Tree
[charset-collation]:/charset-collation-one

***
* ### B+Tree 索引的使用 ###

	索引的使用更接近实战，先创建一张表：

		CREATE TABLE `t_person` (
		`id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '记录ID',
		`name` VARCHAR(100) NOT NULL DEFAULT '' COMMENT '名字',
		`age` SMALLINT NOT NULL DEFAULT '0' COMMENT '年龄',
		`tel` VARCHAR(24) NOT NULL DEFAULT '' COMMENT '电话',
		`addr` VARCHAR(128) NOT NULL DEFAULT '' COMMENT '地址',
		`country` VARCHAR(16) NOT NULL DEFAULT '' COMMENT '国籍',
		PRIMARY KEY (id),
		KEY idx_name_age_tel (name, age, tel)
		)ENGINE=INNODB DEFAULT CHARSET=utf8mb4;

	上篇[文章][B+Tree]介绍了聚簇索引、联合索引，聚簇索引是以主键值大小进行排序的B+Tree；而联合索引（例如刚刚创建的表）是以name、age、tel的大小进行排序而创建的B+Tree，当name值相同时按照age大小排序，当age值相同时按照tel大小排序。上表的字段name、tel的数据类型是varchar，对于这样的数据类型比较大小依赖于字符集及比较规则，关于字符集和比较规则可以参考这篇[文章][charset-collation]，一个字符串的比较过程可以是这样的：

	1. 先按照字符串的第一个字符进行排序

	2. 如果第一个字符相同再按照第二个字符进行排序

	3. 如果第二个字符相同再按照第三个字符进行排序，依此类推

	介绍这些，使用索引时要按照这些规则使用：

	- 全值匹配 
	
		全值匹配指的是和索引中所有列进行匹配，即匹配name、age、tel，而条件语句的顺序无关，MySQL有查询优化器，会分析这些搜索条件并且按照可以使用的索引中列的顺序来决定先使用哪个搜索条件，后使用哪个搜索条件

			SELECT country FROM t_person WHERE name='zhangsan' AND age=10 AND tel='18888888888';

		或者：

			SELECT country FROM t_person WHERE age=10 AND tel='18888888888' AND name='zhangsan';

	- 匹配最左前缀

		以上表为例我们可以查找name为zhangsan的人，即索引的第一列name

			SELECT country FROM t_person WHERE name='zhangsan';

		也可匹配左侧多列，例如：

			SELECT country FROM t_person WHERE name='zhangsan' AND age=10;

		<font color="#dd0000">如果想使用联合索引中尽可能多的列，搜索条件中的各个列必须是联合索引中从最左边连续的列。 不能跳过联合索引的第一列</font>例如：

			SELECT country FROM t_person WHERE age=10; // 错误的使用
			SELECT country FROM t_person WHERE tel='18888888888'; // 错误的使用

		而下面的使用只能使用name列索引

			SELECT country FROM t_person WHERE name='zhangsan' AND tel='18888888888';

	- 匹配列前缀

		也可以只匹配某一列值的<font color="#dd0000">开头</font>部分，例如：

			SELECT country FROM t_person WHERE name LIKE 'zhang%';

		或者：

			SELECT country FROM t_person WHERE name='zhangsan' AND age=10 AND tel LIKE '188%';

		注意是开头部分，下面的使用是错误的:

			SELECT country FROM t_person WHERE name LIKE '%zhang%'; // 错误的使用

	- 匹配范围值

		以上表为例，查找zhangsan和wangwu之间的人，只使用索引的第一列:

			SELECT country FROM t_person WHERE name > 'zhangsan' AND name < 'wangwu';

		不过在使用联合索引进行范围查找的时候需要注意，如果对多个列同时进行范围查找时，只有对索引最左边的那个列进行范围查找的时候才能用到B+树索引：

			SELECT country FROM t_person WHERE name > 'zhangsan' AND name < 'wangwu' AND age > 10;

		这个查询可以分成两个部分：

		1. 通过条件name > 'zhangsan' AND name < 'wangwu'来对name进行范围，查找的结果可能有多条name值不同的记录

		2. 对这些name值不同的记录继续通过age > 10条件继续过

		对于联合索引idx_name_age_tel来说，只能用到name列的部分，而用不到age列的部分，因为只有name值相同的情况下才能用age列的值进行排序，而这个查询中通过name进行范围查找的记录中可能并不是按照age列进行排序的，所以在搜索条件中继续以age列进行查找时是用不到这个B+树索引的

	- 精确匹配某一列并范围匹配另一列

		以上表为例，即第一列name全匹配，第二、三列范围匹配

			SELECT country FROM t_person WHERE name='zhangsan' AND age > 10 AND tel > '18888888888';

		或者：

			SELECT country FROM t_person WHERE name='zhangsan' AND age = 10 AND tel > '18888888888';

***
* ### B+Tree索引在ORDER BY 和 GROUP BY 中的应用 ###

	ORDER BY在MySQL实际使用中经常遇见，<font color="#dd0000">对于联合索引ORDER BY的子句后边的列的顺序必须按照索引列的顺序给出</font>

		SELECT country,addr FROM t_person ORDER BY name,age,tel;

	上面的这个查询语句意思是：按照name升序排序，name相同按照age升序排序，age相同按照tel升序排序，这种排序方式正是以联合索引创建的B+Tree数据排列方式，所以直接取出主键值到聚簇索引中取出数据就可以了，这个过程称为回表

	ORDER BY name、ORDER BY name, age这种匹配索引左边的列的形式可以使用部分的B+树索引。当联合索引左边列的值为常量，也可以使用后边的列进行排序

		SELECT country,addr FROM t_person WHERE name='zhangsan' ORDER BY age,tel LIMIT 10;

	有时候可能查询的结果集太大以至于不能在内存中进行排序的话，还可能暂时借助磁盘的空间来存放中间结果，排序操作完成后再把排好序的结果集返回到客户端。在MySQL中，把这种在内存中或者磁盘上进行排序的方式统称为文件排序filesort。filesort会极大影响查询速度，所以当触发filesort时应该检查一下索引的建立是否合理，查询语句是否有可替代方案等等

	不能使用索引进行排序的几种情况：

	* ASC、DESC混用

		查询语句里既有ASC又有DESC

			SELECT country,addr FROM t_person ORDER BY name,age DESC LIMIT 10;

		这样的查询使算法很复杂，先从索引左侧找到最小值，在找到等于这个最小值的所有记录，再从这些记录的右侧往左找10条记录，如果这个值的记录数量不足10条，再找次小的记录......，这种情况不能使用索引进行排序

	* WHERE子句中出现非排序使用到的索引列

		这样的查询只能先把符合搜索条件country = 'USA'的记录提取出来后再进行排序

			SELECT * FROM t_person WHERE country='USA' ORDER BY name LIMIT 10;

	* 排序列包含非同一个索引的列

		用来排序的多个列不是一个索引里的，这种情况也不能使用索引进行排序

			SELECT * FROM t_person ORDER BY name,country LIMIT 10;

	* 排序列使用复杂表达式

		有调用函数或者计算表达式

			SELECT * FROM t_person ORDER BY UPPER(name) LIMIT 10;


		GROUP BY的使用和使用B+树索引进行排序是一个道理，分组列的顺序也需要和索引列的顺序一致，也可以只使用索引列中左边的列进行分组，例如：

			SELECT name,age,tel FROM t_person GROUP BY name,age,tel;

***
* ### 回表和覆盖索引 ###

	上面提到了回表，下面的查询会触发回表：

		SELECT * FROM t_person WHERE name > 'zhangsan' AND name < 'wangwu';

	这个查询过程是：先在以联合索引idx_name_age_tel创建的B+Tree中找到这个条件的记录，取得主键id，再根据主键id到聚簇索引中取得用户数据记录。由于索引idx_name_age_tel对应的B+树中的记录首先会按照name列的值进行排序，所以值在zhangsan～wangwu之间的记录在磁盘中的存储是相连的，集中分布在一个或几个数据页中，我们可以很快的把这些连着的记录从磁盘中读出来，这种读取方式称为顺序I/O，而聚簇索引中是以主键大小的顺序排列数据的，当我们用主键id到聚簇索引中取用户数据记录时，多个主键id有可能并不在一个数据页上，这样读取完整的用户记录可能要访问更多的数据页，这种读取方式称为随机I/O。一般情况下，顺序I/O比随机I/O的性能高很多

	使用二级索引或者联合索引查询所有列值时会触发回表、随机I/O。需要回表的记录越多，使用二级索引的性能就越低，name值在zhangsan～wangwu之间的用户记录数量占全部记录数量95%以上，那么如果使用idx_name_age_tel索引的话，有95%多的id值需要回表，什么时候采用全表扫描的方式，什么时候使用采用二级索引 + 回表的方式去执行查询，这个是查询优化器做的工作，查询优化器会事先对表中的记录计算一些统计数据，然后再利用这些统计数据根据查询的条件来计算一下需要回表的记录数，需要回表的记录数越多，就越倾向于使用全表扫描，反之倾向于使用二级索引 + 回表的方式。当然优化器做的分析工作不仅仅是这么简单，但是大致上是个这个过程。一般情况下，限制查询获取较少的记录数会让优化器更倾向于选择使用二级索引 + 回表的方式进行查询，因为回表的记录越少，性能提升就越高，上面查询语句可以改写成这样：

		SELECT * FROM t_person WHERE name > 'zhangsan' AND name < 'wangwu' LIMIT 10;

	添加了LIMIT 10的查询更容易让优化器采用二级索引 + 回表的方式进行查询，上面有个查询语句是这样的：

		SELECT country,addr FROM t_person ORDER BY name,age,tel;

	由于查询列表是country、addr，所以如果使用二级索引进行排序的话，需要把排序完的二级索引记录全部进行回表操作，这样操作的成本还不如直接遍历聚簇索引然后再进行文件排序（filesort）低，所以优化器会倾向于使用全表扫描的方式执行查询，增加limit后优化器就会倾向于使用二级索引 + 回表的方式执行查询

	上面的查询是当查询列不在索引列触发了回表，例如country、addr都不在索引列，如果查询列是索引列就可以避免回表，查询效率被提高，例如：

		SELECT id,name,age,tel FROM t_person ORDER BY name,age,tel LIMIT 10;

	<font color="#dd0000">把这种只需要用到索引的查询方式称为索引覆盖</font>

***
* ### 如何建立索引 ###

	建立正确的索引对于高效查询很重要

	- 只为用于搜索、排序或分组的列创建索引

	- 考虑列的基数

		列的基数指的是某一列中不重复数据的个数，也就是说在记录行数一定的情况下，列的基数越大，该列中的值越分散，列的基数越小，该列中的值越集中。可以按照这个公式统计：`count(distinct(索引列))/count(*)`，以上表为例统计name列的基数，式子是这样的：`count(distinct(name))/count(*)`，结果越接近1说明该列中的值越分散

		最好为那些列的基数大的列建立索引，为基数太小列的建立索引效果可能不好

	- 索引列的类型尽量小

		在定义表结构的时候要显式的指定列的类型，像整数类型TINYINT、MEDIUMINT、INT、BIGINT，对某个整数列建立索引的话，在表示的整数范围允许的情况下，尽量让索引列使用较小的类型

		1. 数据类型越小，在查询时进行的比较操作越快

		2. 数据类型越小，索引占用的存储空间就越少，在一个数据页内就可以放下更多的记录，从而减少磁盘I/O带来的性能损耗，也就意味着可以把更多的数据页缓存在内存中，从而加快读写效率

		这个建议对于表的主键来说更加适用，因为不仅是聚簇索引中会存储主键值，其他所有的二级索引的节点处都会存储一份记录的主键值，如果主键适用更小的数据类型，也就意味着节省更多的存储空间和更高效的I/O

	- 索引字符串值的前缀

		只索引字符串值的前缀的效率比较高，尤其是在字符串类型能存储的字符比较多的时候。如果以全字符串做索引，需要面临这些问题：

		1. B+树索引中的记录需要把该列的完整字符串存储起来，而且字符串越长，在索引中占用的存储空间越大

		2. 如果B+树索引中索引列存储的字符串很长，那在做字符串比较时会占用更多的时间

		那么以字符串前缀做索引也就是说在二级索引的记录中只保留字符串前几个字符。这样在查找记录时虽然不能精确的定位到记录的位置，但是能定位到相应前缀所在的位置，然后根据前缀相同的记录的主键值回表查询完整的字符串值，这样只在B+树中存储字符串的前几个字符的编码，既节约空间，又减少了字符串的比较时间，以上表为例，创建表语句可以是这样：

			CREATE TABLE `t_person` (
			`id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '记录ID',
			`name` VARCHAR(100) NOT NULL DEFAULT '' COMMENT '名字',
			`age` SMALLINT NOT NULL DEFAULT '0' COMMENT '年龄',
			`tel` VARCHAR(24) NOT NULL DEFAULT '' COMMENT '电话',
			`addr` VARCHAR(128) NOT NULL DEFAULT '' COMMENT '地址',
			`country` VARCHAR(16) NOT NULL DEFAULT '' COMMENT '国籍',
			PRIMARY KEY (id),
			KEY idx_name_age_tel (name(10), age, tel)
			)ENGINE=INNODB DEFAULT CHARSET=utf8mb4;

		name(10)表示在建立的B+树索引中只保留记录的前10个字符的编码，正因为只保留记录的前10个字符，所以下面的查询并不能使用索引：

			SELECT * FROM t_person ORDER BY name LIMIT 10;

	- 索引列在比较表达式中单独出现

		以一个整数列做索引为例，这样的查询会导致全表扫描`SELECT * FROM t_person WHERE gid + 10 > 1000;`，索引列并没有以单独列的形式出现，是以gid + 10这样的表达式出现的，存储引擎会依次遍历所有的记录，计算这个表达式的值是否大于1000，所以这种情况下是用不到为gid列建立的B+树索引的，可以做出这样调整`SELECT * FROM t_person WHERE gid > 990;`

		总结下来就是：<font color="#dd0000">如果索引列在比较表达式中不是以单独列的形式出现，而是以某个表达式，或者函数调用形式出现的话，是用不到索引的</font>

	- 主键插入顺序

		尽量避免主键忽大忽小的插入数据，从上篇文章知道，聚簇索引的用户记录是按照记录主键值从小到大的顺序进行排序，所以如果我们插入的记录的主键值是依次增大的话，那每插满一个数据页就换到下一个数据页继续插，而如果插入的主键值忽大忽小会导致页分裂，也就意味着性能损耗。所以最好让插入的记录的主键值依次递增

	- 重复索引	

		已经存在的索引或者可以使用部分列做索引就没必要再创建索引

