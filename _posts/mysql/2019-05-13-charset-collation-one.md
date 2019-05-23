---
layout: second_template
title: 字符集和比较规则之一
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: charset-collation-one
---
***
* ### 测试环境 ###
	
	MySQL 5.6

***
* ### 几个常用的字符集 ###
	
	只简单介绍几个字符集

	1. ASCII字符集

		共收录128个字符，包括空格、标点符号、数字、大小写字母和一些不可见字符。由于总共才128个字符，所以可以使用1个字节来进行编码

	2. ISO 8859-1字符集

		共收录256个字符，是在ASCII字符集的基础上又扩充了128个西欧常用字符(包括德法两国的字母)，也可以使用1个字节来进行编码。这个字符集也有一个别名latin1

	3. GB2312字符集

		收录了汉字以及拉丁字母、希腊字母、日文平假名及片假名字母、俄语西里尔字母。其中收录汉字6763个，其他文字符号682个。同时这种字符集又兼容ASCII字符集，如果该字符在ASCII字符集中，则采用1字节编码，否则采用2字节编码

	4. GBK字符集

		GBK字符集只是在收录字符范围上对GB2312字符集作了扩充，编码方式上兼容GB2312

	5. utf8字符集

		收录地球上能想到的所有字符，而且还在不断扩充。这种字符集兼容ASCII字符集，采用变长编码方式，编码一个字符需要使用1～4个字节，严格的讲，utf8只是Unicode字符集的一种编码方案，Unicode字符集可以采用utf8、utf16、utf32这几种编码方案，utf8使用1～4个字节编码一个字符，utf16使用2个或4个字节编码一个字符，utf32使用4个字节编码一个字符

***
* ### MySQL支持的字符集 ###

	重点说一下MySQL中的utf8，上边说utf8字符集表示一个字符需要使用1～4个字节，但是我们常用的一些字符使用1～3个字节就可以表示了。而在MySQL中字符集表示一个字符所用最大字节长度在某些方面会影响系统的存储和性能，对于utf8有两个划分：

	1. utf8mb3：缩小版的utf8字符集，只使用1～3个字节表示字符
	2. utf8mb4：正宗的utf8字符集，使用1～4个字节表示字符

	**在MySQL中utf8是utf8mb3的别名，所以之后在MySQL中提到utf8就意味着使用1~3个字节来表示一个字符，要存储表情之类的特殊符号应该使用utf8mb4，这也是我们在创建表结构需要指出字符集是utf8mb4**

	查询MySQL支持的字符集：`SHOW CHARSET;`或者`show charset like '%utf8%';`

	我们运行一下这条语句`show charset like '%utf8%';`：运行结果如下：

		+---------+---------------+--------------------+--------+
		| Charset | Description   | Default collation  | Maxlen |
		+---------+---------------+--------------------+--------+
		| utf8    | UTF-8 Unicode | utf8_general_ci    |      3 |
		| utf8mb4 | UTF-8 Unicode | utf8mb4_general_ci |      4 |
		+---------+---------------+--------------------+--------+

	**Default collation列表示这种字符集中一种默认的比较规则。最后一列Maxlen它代表该种字符集表示一个字符最多需要几个字节**

***
* ### MySQL支持的排序规则 ###

	查看MySQL中支持的比较规则的命令：`SHOW COLLATION [LIKE 匹配的模式];`，当我们运行这条命令会输出：

		+--------------------------+---------+-----+---------+----------+---------+
		| Collation                | Charset | Id  | Default | Compiled | Sortlen |
		+--------------------------+---------+-----+---------+----------+---------+
		| utf8_general_ci          | utf8    |  33 | Yes     | Yes      |       1 |
		| utf8_bin                 | utf8    |  83 |         | Yes      |       1 |
		| utf8_unicode_ci          | utf8    | 192 |         | Yes      |       8 |
		| utf8_icelandic_ci        | utf8    | 193 |         | Yes      |       8 |
		| utf8_latvian_ci          | utf8    | 194 |         | Yes      |       8 |
		| utf8_romanian_ci         | utf8    | 195 |         | Yes      |       8 |
		| utf8_slovenian_ci        | utf8    | 196 |         | Yes      |       8 |
		| utf8_polish_ci           | utf8    | 197 |         | Yes      |       8 |
		| utf8_estonian_ci         | utf8    | 198 |         | Yes      |       8 |
		| utf8_spanish_ci          | utf8    | 199 |         | Yes      |       8 |
		| utf8_swedish_ci          | utf8    | 200 |         | Yes      |       8 |
		| utf8_turkish_ci          | utf8    | 201 |         | Yes      |       8 |
		| utf8_czech_ci            | utf8    | 202 |         | Yes      |       8 |
		| utf8_danish_ci           | utf8    | 203 |         | Yes      |       8 |
		| utf8_lithuanian_ci       | utf8    | 204 |         | Yes      |       8 |
		| utf8_slovak_ci           | utf8    | 205 |         | Yes      |       8 |
		| utf8_spanish2_ci         | utf8    | 206 |         | Yes      |       8 |
		| utf8_roman_ci            | utf8    | 207 |         | Yes      |       8 |
		| utf8_persian_ci          | utf8    | 208 |         | Yes      |       8 |
		| utf8_esperanto_ci        | utf8    | 209 |         | Yes      |       8 |
		| utf8_hungarian_ci        | utf8    | 210 |         | Yes      |       8 |
		| utf8_sinhala_ci          | utf8    | 211 |         | Yes      |       8 |
		| utf8_german2_ci          | utf8    | 212 |         | Yes      |       8 |
		| utf8_croatian_ci         | utf8    | 213 |         | Yes      |       8 |
		| utf8_unicode_520_ci      | utf8    | 214 |         | Yes      |       8 |
		| utf8_vietnamese_ci       | utf8    | 215 |         | Yes      |       8 |
		| utf8_general_mysql500_ci | utf8    | 223 |         | Yes      |       1 |
		+--------------------------+---------+-----+---------+----------+---------+
		27 rows in set (0.00 sec)

	排序规则的命名：所属字符集+比较规则主要作用于哪种语言+是否区分语言中的重音、大小，例如：utf8_spanish_ci表示以西班牙语的规则比较，utf8_turkish_ci表示以土耳其语的规则比较，utf8_general_ci是一种通用的比较规则，后缀的意义：

	1. `_ai` 不区分重音
	2. `_as` 区分重音
	3. `_ci` 不区分大小写
	4. `_cs` 区分大小写
	5. `_bin` 以二进制方式比较

***
* ### 字符集和比较规则的几个级别 ###

	MySQL有4个级别的字符集和比较规则，分别是：

	1. 服务器级别
	2. 数据库级别
	3. 表级别
	4. 列级别

***
* ### 服务器级别 ###

	服务器级别可通过系统变量character_set_server查看服务器级别的字符集，通过collation_server查看服务器级别的比较规则。例如我的MySQL环境：

		SHOW VARIABLES LIKE 'character_set_server';
		+----------------------+--------+
		| Variable_name        | Value  |
		+----------------------+--------+
		| character_set_server | latin1 |
		+----------------------+--------+
		1 row in set (0.00 sec)

		SHOW VARIABLES LIKE 'collation_server';
		+------------------+-------------------+
		| Variable_name    | Value             |
		+------------------+-------------------+
		| collation_server | latin1_swedish_ci |
		+------------------+-------------------+
		1 row in set (0.00 sec)

	我的环境默认的服务器级别的字符集是latin1，比较规则是latin1_swedish_ci，我们可以在启动选项或者在服务器程序运行过程中使用SET语句修改这两个变量的值：

		# 启动选项
		character_set_server=utf8mb4
		collation_server=utf8_general_ci

		# 运行过程中修改
		set character_set_server=utf8mb4;
		Query OK, 0 rows affected (0.00 sec)

	如果运行过程中修改这两个变量，当重启服务时还会恢复到启动选项的设置

***
* ### 数据库级别 ###

	数据库级别可在创建数据库时指定，例如：

		create database character_test character set utf8mb4 collate utf8mb4_general_ci;

	查看一下我们刚刚创建的数据库的字符集和比较规则：

		mysql> use character_test;
		Database changed

		mysql> show variables like 'character_set_database';
		+------------------------+---------+
		| Variable_name          | Value   |
		+------------------------+---------+
		| character_set_database | utf8mb4 |
		+------------------------+---------+
		1 row in set (0.00 sec)

		mysql> SHOW VARIABLES LIKE 'collation_database';
		+--------------------+--------------------+
		| Variable_name      | Value              |
		+--------------------+--------------------+
		| collation_database | utf8mb4_general_ci |
		+--------------------+--------------------+
		1 row in set (0.00 sec)

	**character_set_database 和 collation_database 这两个系统变量是只读的，我们不能通过修改这两个变量的值而改变当前数据库的字符集和比较规则**

	**创建表时如果未指定字符集和比较规则则使用服务器级别的字符集和比较规则，例如：`create database character_test;`**

***
* ### 表级别 ###

	表级别可在创建表时指定，例如：

		USE character_test;
		CREATE TABLE `t_character_test` (
		  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '记录ID',
		  `gid` INT NOT NULL DEFAULT '0' COMMENT '用户GID',
		  `name` VARCHAR(255) NOT NULL DEFAULT '' COMMENT '用户名称',
		  `level` INT NOT NULL DEFAULT '0' COMMENT '用户等级',
		  PRIMARY KEY (`Id`),
		  KEY `idx_user_gid` (`gid`)
		)ENGINE=INNODB DEFAULT CHARSET=utf8mb4 COLLATE utf8mb4_general_ci;

		// 查看表的字符集
		SHOW TABLE STATUS FROM character_test LIKE '%t_character_test%';

		// 修改表的字符集和比较规则
		ALTER TABLE t_character_test CHARACTER SET utf8 COLLATE utf8_general_ci;

	**如果只指定了字符集而未指定比较规则，则使用的是这个字符集默认的比较规则，同时，如果未指定表的字符集，则默认使用该表所在数据库的字符集和比较规则**

***
* ### 列级别 ###

	列级别的字符集和比较规则可以在创建表时指定，也可以单独修改，同一个表中的不同的列可以有不同的字符集和比较规则，例如：

		USE character_test;
		CREATE TABLE `t_character_test` (
		  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '记录ID',
		  `gid` INT NOT NULL DEFAULT '0' COMMENT '用户GID',
		  `name` VARCHAR(255) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '' COMMENT '用户名称',
		  `nickname` VARCHAR(255) CHARACTER SET gbk COLLATE gbk_chinese_ci NOT NULL DEFAULT '' COMMENT '用户昵称',
		  `level` INT NOT NULL DEFAULT '0' COMMENT '用户等级',
		  PRIMARY KEY (`Id`),
		  KEY `idx_user_gid` (`gid`)
		)ENGINE=INNODB DEFAULT CHARSET=utf8mb4 COLLATE utf8mb4_general_ci;

		// 修改列字符集和比较规则
		USE character_test;
		ALTER TABLE t_character_test MODIFY nickname VARCHAR(255) CHARACTER SET utf8 COLLATE utf8_general_ci;

	**对于某个列来说，如果在创建和修改的语句中没有指明字符集和比较规则，将使用该列所在表的字符集和比较规则作为该列的字符集和比较规则， 在转换列的字符集时需要注意，如果转换前列中存储的数据不能用转换后的字符集进行表示会发生错误。例如修改前的列使用的字符集是utf8，列中存储了一些汉字，现在把列的字符集转换为ascii的话就会出错，因为ascii字符集并不能表示汉字字符**

***
* ### 只修改字符集或只修改比较规则 ###

	每个比较规则对应着一个字符集，每个字符集有自己默认的比较规则，如果我们只修改了字符集，比较规则也会跟着变化，如果只修改了比较规则，字符集也会跟着变化，总结下来就是：

	- 只修改字符集，则比较规则将变为修改后的字符集默认的比较规则

	- 只修改比较规则，则字符集将变为修改后的比较规则对应的字符集

	**不论哪个级别的字符集和比较规则，这两条规则都适用**

***
* ### 总结 ###
	
	下面这几点总结上面提到了一些

	- 如果创建或修改列时没有显式的指定字符集和比较规则，则该列默认用表的字符集和比较规则
	- 如果创建或修改表时没有显式的指定字符集和比较规则，则该表默认用数据库的字符集和比较规则
	- 如果创建或修改数据库时没有显式的指定字符集和比较规则，则该数据库默认用服务器的字符集和比较规则

	知道了这些规则后，我们可以根据需求做出适当选择，同时我们可以通过命令查询列的字符集和比较规则，从而根据这个列的类型来确定存储数据时每个列的实际数据占用的存储空间大小

	