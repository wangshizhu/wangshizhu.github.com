---
layout: second_template
title: 字符集和比较规则
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: charset&collation
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
* ### MySQL支持的字符集###

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
* ### 字符集和比较规则应用 ###

	MySQL有4个级别的字符集和比较规则，分别是：

	1. 服务器级别
	2. 数据库级别
	3. 表级别
	4. 列级别

	