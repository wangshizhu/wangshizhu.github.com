---
layout: second_template
title: InnoDB Buffer Pool
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: innoDB-buffer-pool
---

[Buffer_Pool]:assets/themes/my_blog/img/Buffer_Pool.jpg

InnoDB存储引擎在处理客户端的请求时，当需要访问某个页的数据时，就会把完整的页的数据全部加载到内存中，也就是说即使只需要访问一个页的一条记录，
那也需要先把整个页的数据加载到内存中。将整个页加载到内存中后就可以进行读写访问了，在进行完读写访问之后并不立即把该页对应的内存空间释放掉，而是将其缓存起来，
这样将来有请求再次访问该页面时，就可以省去磁盘IO的开销了

为了缓存磁盘中的页，在MySQL服务器启动的时候就向操作系统申请了一片连续的内存，这块内存叫做Buffer Pool

Buffer Pool的大小可以在服务器的配置中字段innodb_buffer_pool_size指定：

	[server]
	innodb_buffer_pool_size = 268435456
	
innodb_buffer_pool_size以**字节**为单位，不能低于最小值5M，低于最小值时按照最小值5M设定

### Buffer Pool 简易结构
--------------------------------------------------

Buffer Pool的结构可以参考此图：

![Alt text][Buffer_Pool]

* 控制块

	图中缓存页对应着磁盘上的页，大小都为16K，每个缓存页对应一个控制块，控制块包含的信息可以在结构图中看到，每个缓存页对应的控制块占用的内存大小是相同的，
它们都被存放到 Buffer Pool 中，其中控制块被存放到 Buffer Pool 的前边，缓存页被存放到 Buffer Pool 后边，在MySQL5.7.21这个版本中，每个控制块占用的大小是808字节。
而设置的innodb_buffer_pool_size并不包含这部分控制块占用的内存空间大小，也就是说InnoDB在为Buffer Pool向操作系统申请连续的内存空间时，
这片连续的内存空间一般会比innodb_buffer_pool_size的值大5%左右

	从图中看到有碎片空间，所有的缓存页和控制块是一段连续的内存空间，碎片空间是这段连续内存空间分配完缓存页和控制块后所剩下的

* Buffer Pool Instance

	结构图中展示了整个Buffer Pool由两个Buffer Pool Instance组成，每个Buffer Pool Instance称为一个实例，**它们都是独立的，独立的去申请内存空间，独立的管理各种链表**，
所以在多线程并发访问时并不会相互影响，从而提高并发处理能力。实例个数可以通过配置设置：

	[server]
	innodb_buffer_pool_instances = 2
	
	同时MySQL规定当innodb_buffer_pool_size的值小于1G的时候设置多个实例是无效的，InnoDB会默认把innodb_buffer_pool_instances 的值修改为1
	
* chunk
	
	结构图中展示了一个实例包含了若干chunk,在MySQL 5.7.5之前，Buffer Pool的大小只能在服务器启动时通过配置innodb_buffer_pool_size启动参数来调整大小，
在服务器运行过程中是不允许调整该值的。不过MySQL5.7.5以及之后的版本中支持了在服务器运行过程中调整Buffer Pool大小的功能，
在服务器运行期间调整Buffer Pool的大小时就是以chunk为单位增加或者删除内存空间，也就是说一个Buffer Pool实例其实是由若干个chunk组成的，
一个chunk就代表一片连续的内存空间，包含了若干缓存页与其对应的控制块，

	chunk的大小是在配置文件通过innodb_buffer_pool_chunk_size启动参数指定的，它的默认值是134217728，也就是128M。
**innodb_buffer_pool_chunk_size的值只能在服务器启动时指定，在服务器运行过程中是不可以修改的**

### Free链表
--------------------------------------------------

从图中看到有Free链表，而链表的节点是控制块

当启动MySQL服务器的时候，需要完成对Buffer Pool的初始化过程，就是先向操作系统申请Buffer Pool的内存空间，然后把它划分成若干对控制块和缓存页。
但是此时并没有真实的磁盘页被缓存到Buffer Pool中，之后随着程序的运行，会不断的有磁盘上的页被缓存到Buffer Pool中

当从磁盘加载数据到缓存页时，需要找到未使用的缓存页，那么这个未使用的缓存页就是由Free链表管理的（严格的说它管理着缓存页对应的控制块），取出空闲的缓存页，
并且把该缓存页对应的控制块的信息填上（就是该页所在的表空间、页号之类的信息），然后把该缓存页对应的free链表节点从链表中移除

同时这个链表的基础信息由一个基节点管理着，这个基节点包含的一些基础数据可以从图中看出（链表的头节点、链表的尾节点、节点数量等）

在从磁盘加载一个页之前，需要判断这个页是否在缓存中，这个判断过程访问了以**表空间+页号**为key、缓存页为value的hash表，
如果有，直接使用该缓存页就好，如果没有，那就从free链表中选一个空闲的缓存页，然后把磁盘中对应的页加载到该缓存页的位置

### Flush链表和刷新
--------------------------------------------------

从结构图中看到有一个Flush链表，顾名思义，这个链表管理着修改过的缓存页对应的控制块（已经修改的缓存页也称为脏页dirty page），为以后将数据同步到磁盘准备的，
Flush链表的基础数据同样由一个基节点管理着

刷新脏页到磁盘是由存储引擎层**专门的线程**每隔一段时间触发，刷新有两种方案：

* 从LRU链表的冷数据中刷新一部分页面到磁盘

	线程会定时从LRU链表尾部开始扫描一些页面，扫描的页面数量可以通过系统变量innodb_lru_scan_depth来指定，如果发现脏页，会把它们刷新到磁盘。
	这种刷新页面的方式被称之为**BUF_FLUSH_LRU**
	
* 从flush链表中刷新一部分页面到磁盘

	线程也会定时从flush链表中刷新一部分页面到磁盘，刷新的速率取决于当时系统是不是很繁忙。这种刷新页面的方式被称之为**BUF_FLUSH_LIST**
	
有时候线程刷新脏页的进度比较慢，导致用户线程在准备加载一个磁盘页到Buffer Pool时没有可用的缓存页，
这时就会尝试看看LRU链表尾部有没有可以直接释放掉的未修改页面，
如果没有的话会不得不将LRU链表尾部的一个脏页同步刷新到磁盘（和磁盘交互是很慢的，这会降低处理用户请求的速度）。
这种刷新单个页面到磁盘中的刷新方式被称之为**BUF_FLUSH_SINGLE_PAGE**。

有时候系统特别繁忙时，也可能触发用户线程批量的从flush链表中刷新脏页的情况

### LRU链表
--------------------------------------------------

当Buffer Pool中不再有空闲的缓存页时，就需要淘汰掉部分最近很少使用的缓存页，按照这个语意正好符合LRU(least recently used)链表，一个简单的LRU链表可以按照这种逻辑设计：

* 在LRU链表中找不到节点，则创建节点加入LRU链表头部
* 在LRU链表中找到节点，则将该节点移到LRU链表头部

使用简单的LRU链表对于InnoDB是暴力的，LRU链表在MySQL中分成了两部分：

* 一部分存储使用频率非常高的缓存页，所以这一部分链表也叫做热数据，或者称young区域
* 另一部分存储使用频率不是很高的缓存页，所以这一部分链表也叫做冷数据，或者称old区域

划分点是通过系统变量innodb_old_blocks_pct的值来确定old区域的，下面展示了查看、设置这个变量例子：

	SHOW VARIABLES LIKE 'innodb_old_blocks_pct';
	SET GLOBAL innodb_old_blocks_pct = 40;
	
这个例子中old区域占LRU链表的比例就是40%。这个系统变量属于全局变量，一经修改，会对所有客户端生效。也可以通过配置文件设置这个变量：
	
	[server]
	innodb_old_blocks_pct = 40
	
做出这样的设计是为了支持两种机制：

* 预读read ahead
	
	预读就是InnoDB认为执行当前的请求可能之后会读取某些页面，就预先把它们加载到Buffer Pool中，
预读分两种：
	
	1. 线性预读
		
		线性预读是如果顺序访问了某个区extent的页面超过系统变量innodb_read_ahead_threshold的值，
		就会触发一次**异步**读取下一个区中全部的页面到Buffer Pool的请求，
		**异步**读取意味着从磁盘中加载这些被预读的页面并不会影响到当前工作线程的正常执行
		
		innodb_read_ahead_threshold系统变量的值默认是56，可以在服务器启动时通过启动参数或者服务器运行过程中直接调整该系统变量的值，同样它是一个全局变量
		
	2. 随机预读
		
		随机预读是如果Buffer Pool中已经缓存了某个区的13个连续的页面，不论这些页面是不是顺序读取的，
		都会触发一次**异步读取**本区中所有其的页面到Buffer Pool的请求。可以通过innodb_random_read_ahead系统变量来控制是否开启随机预读，它的默认值为OFF，可以通过下面命令设置开启：
		
			SET GLOBAL innodb_random_read_ahead = ON;
	
* 全表扫描
	
	全表扫描就是将表所有的页加载到内存，如果按照上面的简单的LRU链表设计很容易把当前访问很热的页移除掉，也很容易将Buff Pool中所有的页换掉，反而大大降低了缓存命中率
	
有了这样的LRU链表设计可能大大提高缓存命中率：

* 预读read ahead
	
	当磁盘上的某个页面在初次加载到Buffer Pool中的某个缓存页时，该缓存页对应的控制块会被放到old区域的头部。
这样针对预读到Buffer Pool却不进行后续访问的页面就会被逐渐从old区域逐出，而不会影响young区域中比较热的缓存页
	
* 全表扫描
	
	在进行全表扫描时，虽然首次被加载到Buffer Pool的页被放到了old区域的头部，但是后续会被马上访问到，
每次进行访问的时候又会把该页放到young区域的头部，这样仍然会把那些比较热的页面给顶下去，针对这样的问题有这样的设计：
**在对某个处在old区域的缓存页进行第一次访问时就在它对应的控制块中记录下来这个访问时间，如果后续的访问时间与第一次访问的时间在某个时间间隔内，
那么该页面就不会被从old区域移动到young区域的头部，否则将它移动到young区域的头部**，这个时间间隔可以通过系统变量innodb_old_blocks_time设置：

		SHOW VARIABLES LIKE 'innodb_old_blocks_time';
		SET GLOBAL innodb_old_blocks_time = 1000;
	
	这个系统变量的单位是毫秒

将LRU链表划分为young和old区域这两个部分，又添加了innodb_old_blocks_time这个系统变量，才使得预读机制和全表扫描造成的缓存命中率降低的问题得到了遏制，
因为用不到的预读页面以及全表扫描的页面都只会被放到old区域，而不影响young区域中的缓存页

如果只对LRU链表来讨论，有很多优化策略，对LRU链表的使用也有很多场景，例如Python的GC，node.js的GC等等，可以看看LRU在这些场景的应用

### 更多关于Buffer Pool
--------------------------------------------------

上面提到了多个关于Buffer Pool 的配置，关于Buffer Pool的配置有一些注意点：

1. innodb_buffer_pool_size必须是innodb_buffer_pool_chunk_size × innodb_buffer_pool_instances的倍数
	
	这一点主要是保证每一个Buffer Pool Instance中包含的chunk数量相同，默认的innodb_buffer_pool_chunk_size值是128M，指定的innodb_buffer_pool_instances的值是16，这两个值的乘积就是2G
所以innodb_buffer_pool_size的值必须是2G或者2G的整数倍，如果指定的innodb_buffer_pool_size大于2G并且不是2G的整数倍，那么服务器会自动的把innodb_buffer_pool_size的值调整为2G的整数倍

2. 如果在服务器启动时，innodb_buffer_pool_chunk_size × innodb_buffer_pool_instances的值已经大于innodb_buffer_pool_size的值，
那么innodb_buffer_pool_chunk_size的值会被服务器自动设置为innodb_buffer_pool_size/innodb_buffer_pool_instances的值

可以通过下面命令查看Buffer Pool相关信息：

	SHOW ENGINE INNODB STATUS\G;
	
这条命令会输出很多信息，这部分BUFFER POOL AND MEMORY信息是关于Buffer Pool的，着重看一下这部分数据代表的含义：

* Total memory allocated

	此字段代表Buffer Pool向操作系统申请的连续内存空间大小，包括全部控制块、缓存页、以及碎片的大小

* Dictionary memory allocated

	此字段代表为数据字典信息分配的内存空间大小，这个内存空间和Buffer Pool没什么关系，不包括在Total memory allocated中

* Buffer pool size

	此字段代表该Buffer Pool可以容纳多少缓存页，单位是页

* Free buffers

	此字段代表当前Buffer Pool还有多少空闲缓存页，也就是free链表中还有多少个节点

* Database pages
	
	此字段代表LRU链表中的页的数量，包含young和old两个区域的节点数量

* Old database pages
	
	此字段代表LRU链表old区域的节点数量。

* Modified db pages

	此字段代表脏页数量，也就是flush链表中节点的数量

* Pending reads
	
	此字段代表正在等待从磁盘上加载到Buffer Pool中的页面数量，当准备从磁盘中加载某个页面时，会先为这个页面在Buffer Pool中分配一个缓存页以及它对应的控制块，
然后把这个控制块添加到LRU的old区域的头部，但是这个时候真正的磁盘页并没有被加载进来，Pending reads的值会加1

* Pending writes LRU

	此字段代表即将从LRU链表中刷新到磁盘中的页面数量

* Pending writes flush list

	此字段代表即将从flush链表中刷新到磁盘中的页面数量

* Pending writes single page

	此字段代表即将以单个页面的形式刷新到磁盘中的页面数量。

* Pages made young

	此字段代表LRU链表中曾经从old区域移动到young区域头部的节点数量，一个节点每次只有从old区域移动到young区域头部时才会将Pages made young的值加1，
也就是说如果该节点本来就在young区域，由于它符合在young区域1/4后边的要求，下一次访问这个页面时也会将它移动到young区域头部，但这个过程并不会导致Pages made young的值加1

* Page made not young

	此字段代表在将innodb_old_blocks_time设置的值大于0时，首次访问或者后续访问某个处在old区域的节点时由于不符合时间间隔的限制而不能将其移动到young区域头部时，
	Page made not young的值会加1。对于处在young区域的节点，如果由于它在young区域的1/4处而导致它没有被移动到young区域头部，这样的访问并不会将Page made not young的值加1

* youngs/s

	此字段代表每秒从old区域被移动到young区域头部的节点数量

* non-youngs/s

	此字段代表每秒由于不满足时间限制而不能从old区域移动到young区域头部的节点数量

* Pages read、created、written

	此字段代表读取，创建，写入了多少页。后边跟着读取、创建、写入的速率

* Buffer pool hit rate

	此字段代表在过去某段时间，平均访问1000次页面，有多少次该页面已经被缓存到Buffer Pool了

* young-making rate

	此字段代表在过去某段时间，平均访问1000次页面，有多少次访问使页面移动到young区域的头部了，
	这里统计的将页面移动到young区域的头部次数不仅仅包含从old区域移动到young区域头部的次数，还包括从young区域移动到young区域头部的次数（访问某个young区域的节点，
	只要该节点在young区域的1/4处往后，就会把它移动到young区域的头部）

* not (young-making rate)
	
	此字段代表在过去某段时间，平均访问1000次页面，有多少次访问没有使页面移动到young区域的头部，
	这里统计的没有将页面移动到young区域的头部次数不仅仅包含因为设置了innodb_old_blocks_time系统变量而导致访问了old区域中的节点但没把它们移动到young区域的次数，
还包含因为该节点在young区域的前1/4处而没有被移动到young区域头部的次数

* LRU len

	此字段代表LRU链表中节点的数量

* unzip_LRU

	此字段代表unzip_LRU链表中节点的数量

* I/O sum

	此字段代表最近50s读取磁盘页的总数

* I/O cur

	此字段代表现在正在读取的磁盘页数量

* I/O unzip sum
	
	此字段代表最近50s解压的页面数量

* I/O unzip cur

	此字段代表正在解压的页面数量