---
layout: second_template
title: InnoDB记录结构之Compact
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: innodb-row-format-compact
---
***
* ### 测试环境 ###
	
	MySQL 5.6

***
* ### InnoDB页简介 ###

	InnoDB是一个将表中的数据存储到磁盘上的存储引擎

	InnoDB将数据划分为若干个页，以页作为磁盘和内存之间交互的基本单位，InnoDB中页的大小一般为 16 KB。也就是在一般情况下，一次最少从磁盘中读取16KB的内容到内存中，一次最少把内存中的16KB内容刷新到磁盘中

***
* ### InnoDB行格式 ###

	以记录为单位来向表中插入数据的，这些记录在磁盘上的存放方式被称为行格式或者记录格式

	InnoDB存储引擎到现在为止设计了4种不同类型的行格式，分别是Compact、Redundant(5.0之前)、Dynamic和Compressed行格式

	创建或修改表的语句中指定行格式:

		CREATE TABLE 表名 (列的信息) ROW_FORMAT=行格式名称

		ALTER TABLE 表名 ROW_FORMAT=行格式名称

	以我自己的MySQL环境为例，可以通过`show table status like 'table_name' \G`查看默认行格式，默认的行格式`Row_format: Compact`

***
* ### Compact行格式 ###

	![Alt text][Compact]

	[Compact]: assets/themes/my_blog/img/compact_row_format.jpg

	创建一个测试表：

		CREATE TABLE t_test_format (
			c1 VARCHAR(10),
			c2 VARCHAR(10) NOT NULL,
			c3 CHAR(10),
			c4 VARCHAR(10)
			) CHARSET=ascii ROW_FORMAT=COMPACT;

		INSERT INTO t_test_format(c1, c2, c3, c4) VALUES('aaaa', 'bbb', 'cc', 'd'), ('eeee', 'fff', NULL, NULL);

	从上图中可以看出来，一条完整的记录其实可以被分为**记录的额外信息**和**记录的真实数据**两大部分


***
* ### 变长字段长度列表 ###

	MySQL支持一些变长的数据类型，比如VARCHAR(M)、VARBINARY(M)、各种TEXT类型，各种BLOB类型

	变长字段中存储多少字节的数据是不固定的，所以我们在存储真实数据的时候需要把这些数据占用的字节数也存起来，所以这些变长字段占用的存储空间分为两部分：

	1. 真正的数据内容
	2. 占用的字节数

	在Compact行格式中，把所有变长字段的真实数据占用的字节长度都存放在记录的开头部位，从而形成一个变长字段长度列表，各变长字段数据占用的字节数按照列的顺序**逆序存放**

	以上表和数据为例，我们指定的字符集是ascii字符集，**ascii字符集每个字符占用1字节**。c1、c2、c4是变长列，以第一条记录为例，变长列表是这样的(以十六进制表示)`01 03 04`，注意是逆序

	从上面看变长列表记录了真实内容实际占用字节长度，那么记录真实内容实际占用字节长度的字段(我们暂且称为字段)长度怎么选择呢？以上为例的话1字节足够，那么如果真实内容实际占用字节长度比较大可能需要2字节，InnoDB有它的一套规则，我们首先声明一下W、M和L的意思：

	1. 假设某个字符集中表示一个字符最多需要使用的字节数为W，也就是使用SHOW CHARSET语句的结果中的Maxlen列，比方说utf8字符集中的W就是3，gbk字符集中的W就是2，ascii字符集中的W就是1

	2. 对于变长类型VARCHAR(M)来说，这种类型表示能存储最多M个字符（**注意是字符不是字节**），所以这个类型能表示的字符串最多占用的字节数就是M×W

	3. 假设它实际存储的字符串占用的字节数是L

	规则：

		- 如果M×W <= 255，那么使用1个字节来表示真正字符串占用的字节数，也就是说InnoDB在读记录的变长字段长度列表时先查看表结构，如果某个变长字段允许存储的最大字节数不大于255时，可以认为只使用1个字节来表示真正字符串占用的字节数

		- 如果M×W > 255，则分为两种情况：
			1. 如果L <= 127，则用1个字节来表示真正字符串占用的字节数
			2. 如果L > 127，则用2个字节来表示真正字符串占用的字节数

	**InnoDB在读记录的变长字段长度列表时先查看表结构，如果某个变长字段允许存储的最大字节数大于255时，该怎么区分它正在读的某个字节是一个单独的字段长度还是半个字段长度呢？设计InnoDB时使用该字节的第一个二进制位作为标志位：如果该字节的第一个位为0，那该字节就是一个单独的字段长度（使用一个字节表示不大于127的二进制的第一个位都为0），如果该字节的第一个位为1，那该字节就是半个字段长度。 对于一些占用字节数非常多的字段，比方说某个字段长度大于了16KB，那么如果该记录在单个页面中无法存储时，InnoDB会把一部分数据存放到所谓的溢出页中，在变长字段长度列表处只存储留在本页面中的长度，所以使用两个字节也可以存放下来**

	可以总结为：**如果该可变字段允许存储的最大字节数（M×W）超过255字节并且真实存储的字节数（L）超过127字节，则使用2个字节，否则使用1个字节**

	**变长字段长度列表中只存储值为 非NULL 的列内容占用的长度，值为 NULL 的列的长度是不储存的，同时并不是所有记录都有这个 变长字段长度列表 部分，比方说表中所有的列都不是变长的数据类型的话，这一部分就不需要有**。也就是说对于第二条记录来说，因为c4列的值为NULL，所以第二条记录的变长字段长度列表只需要存储c1和c2列的长度即可。其中c1列存储的值为'eeee'，占用的字节数为4，c2列存储的值为'fff'，占用的字节数为3。数字4可以用1个字节表示，3也可以用1个字节表示，所以整个变长字段长度列表共需2个字节。填充完变长字段长度列表的两条记录的对比图如下：

	![Alt text][Compact1]

	[Compact1]: assets/themes/my_blog/img/compact_row_format_1.jpg

***
* ### NULL值列表 ###

	表中的某些列可能存储NULL值，如果把这些NULL值都放到记录的真实数据中存储会很占地方，所以Compact行格式把这些值为NULL的列统一管理起来，存储到NULL值列表中，它的处理过程是这样的：

	1. 统计表中允许存储NULL的列有哪些，以上表为例t_test_format的3个列c1、c3、c4都是允许存储NULL值的，而c2列是被NOT NULL修饰，不允许存储NULL值

	2. 如果表中没有允许存储 NULL 的列，则 NULL值列表也不存在了，否则将每个允许存储NULL的列对应一个二进制位，二进制位按照列的顺序同样按照**逆序排列**，二进制位表示的意义如下：

		1. 二进制位的值为1时，代表该列的值为NULL
		2. 二进制位的值为0时，代表该列的值不为NULL

	3. MySQL规定NULL值列表必须用整数个字节的位表示，如果使用的二进制位个数不是整数个字节，则在字节的高位补0，以上表的第二条记录为例对应的二进制位是这样的:
	![Alt text][Compact2]

	[Compact2]: assets/themes/my_blog/img/compact_row_format_2.jpg

***
* ### 记录头信息 ###
		
	除了变长字段长度列表、NULL值列表之外，还有一个用于描述记录的记录头信息，它是由固定的5个字节组成。5个字节也就是40个二进制位，不同的位代表不同的意思，如图：
	![Alt text][Compact_Head]

	[Compact_Head]: assets/themes/my_blog/img/compact_row_format_head.jpg

	1. 预留位1		占用1bit		没有使用

	2. 预留位2		占用1bit		没有使用

	3. delete_mask		占用1bit		标记该记录是否被删除

	4. min_rec_mask		占用1bit		B+树的每层非叶子节点中的最小记录都会添加该标记

	5. n_owned		占用4bit		表示当前记录拥有的记录数

	6. heap_no		占用13bit		表示当前记录在记录堆的位置信息

	7. record_type		占用3bit			表示当前记录的类型，0表示普通记录，1表示B+树非叶子节点记录，2表示最小记录，3表示最大记录

	8. next_record		占用16bit		表示下一条记录的相对位置

***
* ### 记录的真实数据 ###

	一上表为例t_test_format，记录的真实数据除了c1、c2、c3、c4这几个我们自己定义的列的数据以外，MySQL会为每个记录默认的添加一些列（也称为隐藏列），具体的列如下：

	1. row_id	不是必须		占用6字节	行ID，唯一标识一条记录
	2. transaction_id	必须		占用6字节	事务ID
	3. roll_pointer	必须 	占用7字节	回滚指针

	实际上这几个列的真正名称其实是：DB_ROW_ID、DB_TRX_ID、DB_ROLL_PTR

	**InnoDB表对主键的生成策略：优先使用用户自定义主键作为主键，如果用户没有定义主键，则选取一个Unique键作为主键，如果表中连Unique键都没有定义的话，则InnoDB会为表默认添加一个名为row_id的隐藏列作为主键。所以我们从上表中可以看出：InnoDB存储引擎会为每条记录都添加 transaction_id 和 roll_pointer 这两个列，但是 row_id 是可选的（在没有自定义主键以及Unique键的情况下才会添加该列）**

	![Alt text][Compact3]

	[Compact3]: assets/themes/my_blog/img/compact_row_format_3.jpg

	1. 表t_test_format使用的是ascii字符集，所以0x61616161就表示字符串'aaaa'，0x626262就表示字符串'bbb'，以此类

	2. 注意第1条记录中c3列的值，它是CHAR(10)类型的，它实际存储的字符串是：'cc'，而ascii字符集中的字节表示是'0x6363'，虽然表示这个字符串只占用了2个字节，但整个c3列仍然占用了10个字节的空间，除真实数据以外的8个字节的统统都用空格字符填充，空格字符在ascii字符集的表示就是0x20

	3. 注意第2条记录中c3和c4列的值都为NULL，它们被存储在了前边的NULL值列表处，在记录的真实数据处就不再冗余存储，从而节省存储空间

***
* ### CHAR(M)列的存储格式 ###

	我们上面提到在Compact行格式下只会把变长类型的列的长度逆序存到变长字段长度列表中，上面我们的字符集ascii是个定长的字符集，也就是说表示一个字符采用固定的一个字节，如果采用变长的字符集（也就是表示一个字符需要的字节数不确定，比如gbk表示一个字符要1~2个字节、utf8表示一个字符要1~3个字节等）的话，c3列的长度也会被存储到变长字段长度列表中，我们尝试修改c3列的字符集`ALTER TABLE t_test_format MODIFY COLUMN c3 CHAR(10) CHARACTER SET utf8;`

	![Alt text][Compact4]

	[Compact4]: assets/themes/my_blog/img/compact_row_format_4.jpg

	**对于 CHAR(M) 类型的列来说，当列采用的是定长字符集时，该列占用的字节数不会被加到变长字段长度列表，而如果采用变长字符集时，该列占用的字节数也会被加到变长字段长度列表，变长字符集的CHAR(M)类型的列要求至少占用M个字节，而VARCHAR(M)却没有这个要求。比方说对于使用utf8字符集的CHAR(10)的列来说，该列存储的数据字节长度的范围是10～30个字节。即使我们向该列中存储一个空字符串也会占用10个字节，这是怕将来更新该列的值的字节长度大于原有值的字节长度而小于10个字节时，可以在该记录处直接更新，而不是在存储空间中重新分配一个新的记录空间，导致原有的记录空间成为所谓的碎片**

***
* ### 行溢出数据 ###
	
	以最敏感的VARCHAR(M)为例，我们通过上面的分析可以知道VARCHAR(M)类型的列最多可以占用65535个字节，其中M代表最多存储的字符数量，如果我们使用ascii字符集的话，一个字符就代表一个字节，那么当我们创建如下表时：

		CREATE TABLE row_format_test(
		c VARCHAR(65535)
		)ENGINE=INNODB CHARSET=ASCII ROW_FORMAT=COMPACT;

	抛出如下错误：

		Row size too large. The maximum row size for the used table type, not counting BLOBs, is 65535. This includes storage overhead, check the manual. You have to change some columns to TEXT or BLOBs

	从报错信息里可以看出，MySQL对一条记录占用的最大存储空间是有限制的，除了BLOB或者TEXT类型的列之外，其他所有的列（不包括隐藏列和记录头信息）占用的字节长度加起来不能超过65535个字节。所以MySQL服务器建议我们把存储类型改为TEXT或者BLOB的类型。这个65535个字节除了列本身的数据之外，还包括一些其他的数据（storage overhead），比如说我们为了存储一个VARCHAR(M)类型的列，其实需要占用3部分存储空间：

	1. 真实数据
	2. 真实数据占用字节的长度
	3. NULL值标识，如果该列有NOT NULL属性则可以没有这部分存储空间

	如果该VARCHAR类型的列有NULL属性，那最多只能存储65532个字节的数据，因为真实数据的长度可能占用2个字节，NULL值标识需要占用1个字节，所以我们再次尝试创建表：

		CREATE TABLE row_format_test(
		c VARCHAR(65532)
		)ENGINE=INNODB CHARSET=ASCII ROW_FORMAT=COMPACT;

	创建成功，如果VARCHAR类型的列有NOT NULL属性，那最多只能存储65533个字节的数据，因为真实数据的长度可能占用2个字节，不需要NULL值标识：

		CREATE TABLE row_format_test(
		c VARCHAR(65533) NOT NULL DEFAULT ""
		)ENGINE=INNODB CHARSET=ASCII ROW_FORMAT=COMPACT;

	同样创建成功，上述创建都是基于ascii字符集，那么换成gbk或者utf8这个长度怎么计算呢？**如果VARCHAR(M)类型的列使用的不是ascii字符集，那M的最大取值取决于该字符集表示一个字符最多需要的字节数。在列的值允许为NULL的情况下，gbk字符集表示一个字符最多需要2个字节，那在该字符集下，M的最大取值就是32766（也就是：65532/2），也就是说最多能存储32766个字符；utf8字符集表示一个字符最多需要3个字节，那在该字符集下，M的最大取值就是21844，就是说最多能存储21844（也就是：65532/3）个字符**

	**还有一个事实：上述例子在列的值允许为NULL的情况下，gbk字符集下M的最大取值就是32766，utf8字符集下M的最大取值就是21844，这都是在表中只有一个字段的情况下说的，一定要记住一个行中的所有列（不包括隐藏列和记录头信息）占用的字节长度加起来不能超过65535个字节**

***
* ### 行溢出 ###

	文章开头说过，MySQL中磁盘和内存交互的基本单位是页，也就是说MySQL是以页为基本单位来管理存储空间的，我们的记录都会被分配到某个页中存储。而一个页的大小一般是16KB，也就是16384字节，以这个表为例:

		CREATE TABLE row_format_test(
		c VARCHAR(65532)
		)ENGINE=INNODB CHARSET=ASCII ROW_FORMAT=COMPACT;

	c列最多可以存储65532个字节，这就导致一个数据页放不下的情况，**在Compact和Reduntant(5.0之前)行格式中，对于占用存储空间非常大的列，在记录的真实数据处只会存储该列的一部分数据，把剩余的数据分散存储在几个其他的页中，然后记录的真实数据处用20个字节存储指向这些页的地址（当然这20个字节中还包括这些分散在其他页面中的数据的占用的字节数），从而可以找到剩余数据所在的页**

	**不只是 VARCHAR(M) 类型的列，其他的 TEXT、BLOB 类型的列在存储数据非常多的时候也会发生行溢出**

	那么列存储多少字节的数据时就会发生行溢出？**MySQL中规定一个页中至少存放两行记录**，以上表为例，上表只有一列，那么当插入两条记录时，每条记录至少多少字节才会溢出？我们先看一下数据页的存储信息：

	1. 每个页除了存放我们的记录以外，也需要存储一些额外的信息，加起来需要136个字节的空间，其他的空间都可以被用来存储记录
	2. 每个记录需要的额外信息是27字节
		- 2个字节用于存储真实数据的长度
		- 1个字节用于存储列是否是NULL值
		- 5个字节大小的头信息
		- 6个字节的row_id列
		- 6个字节的transaction_id列
		- 7个字节的roll_pointer列

	那么就可以得出一个公式：`136 + 2×(27 + n) > 16384`，求得`n > 8098`。也就是说如果一个列中存储的数据不大于8098个字节，那就不会发生行溢出，否则就会发生行溢出。不过这个8098个字节的结论只是针对只有一个列的row_format_test表来说的，如果表中有多个列，那上边的式子和结论都需要改一改了，所以重点就是：**你不用关注这个临界点是什么，只要知道如果我们想一个行中存储了很大的数据时，可能发生行溢出的现象**

***
* ### Dynamic、Compressed、Redundant行格式 ###

	Redundant行格式是MySQL5.0之前版本，不做讨论，Dynamic和Compressed行格式，这两种行格式和Compact行格式挺像，只不过在处理行溢出数据时不一样，它们不会在记录的真实数据处存储字段真实数据的前768个字节，而是把所有的字节都存储到其他页面中，只在记录的真实数据处存储其他页面的地址，Compressed行格式和Dynamic不同的一点是，Compressed行格式会采用压缩算法对页面进行压缩，以节省空间