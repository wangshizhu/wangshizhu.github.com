---
layout: second_template
title: 创建高性能索引之哈希索引
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: high-performance-mysql-5-chapter-hash-index
---

* 哈希索引

	哈希索引基于哈希表实现，只有精确匹配索引所有列的查询才有效。对每一行数据计算哈希值，哈希算法必须选择一个能够生成较小值得算法，哈希索引将索引的哈希值存储在索引中，同时在哈希表中保存指向数据行的指针。在MySQL中只有memory引擎显式支持哈希索引，并且是非唯一哈希索引，如果多个列的哈希值相同，索引会以链表的方式存放多个记录指针到同一个哈希条目中。memory引擎同时也支持B-Tree索引

* 哈希索引的限制

	- 因为哈希索引只包含哈希值和行指针，而不存储哈希字段值，所以不能使用索引中的值来避免读取行

	- 哈希索引数据并不是按照索引值排序存储的，所以也就无法用于排序

	- 因为哈希索引始终是使用索引列的全部内容来计算哈希值，所以哈希索引不支持部分索引列匹配查找，
	例如在数据列（A,B）上建立哈希索引，如果查询只有数据A，则无法使用索引

	- 哈希索引只支持等值比较查询，包括=、IN()、<=>。也不支持任何范围查询，例如`where price > 100`

	- 访问哈希索引的数据非常快，除非有很多哈希冲突。当出现哈希冲突时，存储引擎必须遍历链表中所有的行的指针，逐行比较

	- 如果哈希冲突很多的话，一些索引维护操作的代价也会很高。

* 创建自定义哈希索引

	基于innoDB创建哈希索引，在B-Tree基础上创建一个伪哈希索引，和上面提到的真正哈希索引不是一回事，实际还是通过B-Tree进行查找，它使用的是哈希值而不是键本身，但是我们要在where子句中指定哈希函数。例如一张通过url做索引的表，我们通过URL进行查找，URL本身很长，导致存储的内容很大，而且查询很慢，我们可以建立一个保存URL哈希值的列，令它为索引，同时where子句包含URL常量值，去掉哈希冲突的行。
	例如：
	
		select id from t_url where url_crc=CRC32("http://www.baidu.com") and url="http://www.baidu.com"

	这样我们就需要维护哈希值，可以手动维护，也可以通过触发器

	例如：

		DELIMITER //
		CREATE TRIGGER t_url_ins BEFORE INSERT ON t_url FOR EACH ROW 
		BEGIN 
		SET NEW.url_crc=crc32(NEW.url);
		END;
		//
		CREATE TRIGGER t_url_upd BEFORE UPDATE ON t_url FOR EACH ROW 
		BEGIN
		SET NEW.url_crc=crc32(NEW.url);
		END;
		//
		DELIMITER;

	如果采用这样的方式，就不要使用md5()、sha1()作为哈希函数；这两个函数计算出来的是一个很长的字符串，不但浪费大量空间，而且比较时也慢，虽然这两个函数引起哈希冲突的概率比crc32低，crc32返回的是32位整数，索引当进行比较时会很快，但是当索引有93000条记录时出现的概率是1%。可以引入FNV64函数作为哈希函数，哈希值64位，速度快，冲突比crc32要少





