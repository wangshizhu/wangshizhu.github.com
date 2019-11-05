---
layout: second_template
title: InnoDB独立表空间
category: mysql
tagline: "Supporting tagline"
tags : [mysql]
permalink: separate-innodb-table-space
---

[data_directory]:/data-directory
[leaf_type]:/innoDB-page-structure
[separate_table_space_struct]:assets/themes/my_blog/img/table_space.jpg

--------------------------------------------------
### 宏观结构

在数据目录这篇[文章][data_directory]提到**在MySQL5.6.6以及之后的版本中，InnoDB并不会默认的把各个表的数据存储到系统表空间中，而是为每一个表建立一个独立表空间，也就是说我们创建了多少个表，就有多少个独立表空间**

独立表空间结构如图![Alt text][separate_table_space_struct]

#### 区extent：

**我们知道InnoDB页是存储数据的基本单位，**对于页的管理是通过区extent，一个区包含64个**物理位置连续的页**，也就是1M

**每个区对应一个结构XDES Entry**

我们知道B+树节点（即页）之间是通过双向链表建立关系，页对应着磁盘上的物理位置，所以应该尽量让链表中相邻的页的物理位置也相邻，这样进行范围查询的时候才可以使用所谓的顺序I/O，
如果链表中相邻的两个页物理位置离得非常远，就是所谓的随机I/O。而磁盘的速度和内存的速度差好几个数量级，所以有了区extent的概念

在表中数据量大的时候，为某个索引分配空间的时候就不再按照页为单位分配了，而是按照区为单位分配，甚至在表中的数据十分非常多的时候，可以一次性分配多个连续的区

从结构图中可以看到XDES Entry的组成：

* Segment ID

	每一个段都有一个唯一的编号，Segment ID字段表示就是该区所在的段

* List Node

	这个部分可以将若干个XDES Entry结构串联成一个链表，列表的结构参照结构图

* State

	区大体上可以分为4种类型：

	+ 空闲的区：现在还没有用到这个区中的任何页面，记为FREE
	
	+ 有剩余空间的碎片区：表示碎片区中还有可用的页面，记为FREE_FRAG
	
	+ 没有剩余空间的碎片区：表示碎片区中的所有页面都被使用，没有空闲页面，记为FULL_FRAG

	+ 附属于某个段的区。每一个索引都可以分为叶子节点段和非叶子节点段，除此之外InnoDB还会另外定义一些特殊作用的段，在这些段中的数据量很大时将使用区来作为基本的分配单位，记为FSEG

	**处于FREE、FREE_FRAG以及FULL_FRAG这三种状态的区都是独立的，直属于表空间；而处于FSEG状态的区是附属于某个段的**

* Page State Bitmap

	这个部分共占用16个字节，也就是128个比特位。前面提到一个区默认有64个页，这128个比特位被划分为64个部分，每个部分2个比特位，
对应区中的一个页。比如Page State Bitmap部分的第1和第2个比特位对应着区中的第1个页面，第3和第4个比特位对应着区中的第2个页面。
这两个比特位的第一个位表示对应的页是否是空闲的，第二个比特位还没有用


#### 组：
	
256个区记为一组

整个表空间的第一组的第一个区（也就是图中extent 0）前三个页面是固定的，分别是FSP_HDR类型的页、IBUF_BITMAP类型的页、INODE类型的页，关于页类型可以参考这篇[文章][leaf_type]

整个表空间的其余组的第一个区（例如图中的extent 256、extent 512）前两个页面是固定的，分别是XDES类型的页、IBUF_BITMAP类型的页

#### 段segment：

**以索引为例，叶子结点为一个段，非叶子结点为一个段，所以一个索引有两个段，当然还有其他类型的段，例如对于一个包含聚簇索引、二级索引的表，那么它有4个段**

**每个段对应一个结构INODE Entry，**而段对应哪个INODE Entry结构是在INDEX类型的页有一个Page Header部分存储的————PAGE_BTR_SEG_LEAF、PAGE_BTR_SEG_TOP，关于INDEX类型页可以参考这篇[文章][leaf_type]
，PAGE_BTR_SEG_LEAF和PAGE_BTR_SEG_TOP都占用10个字节，它们都对应一个Segment Header的结构，下表是这个结构的组成：

|名称|占用字节|描述|
|-|-|-|
|Space ID of the INODE Entry|4|INODE Entry结构所在的表空间ID|
|Page Number of the INODE Entry|4|INODE Entry结构所在的页面页号|
|Byte Offset of the INODE Ent|2|INODE Entry结构在该页面中的偏移量|

PAGE_BTR_SEG_LEAF记录着叶子节点段对应的INODE Entry结构的地址是哪个表空间的哪个页面的哪个偏移量

PAGE_BTR_SEG_TOP记录着非叶子节点段对应的INODE Entry结构的地址是哪个表空间的哪个页面的哪个偏移量

**一个索引只对应两个段，所以只需要在索引的根页面中记录这两个结构**

有个应用场景是范围查询，范围查找最终是对B+树叶子节点中的记录进行顺序扫描，如果不区分叶子节点和非叶子节点，统统把节点代表的页面放到申请到的区中，进行范围扫描就有可能触发随机IO。
所以对B+树的叶子节点和非叶子节点进行了划分，也就是说叶子节点有自己独有的区，非叶子节点也有自己独有的区。**存放叶子节点的区的集合是一个段（segment），
存放非叶子节点的区的集合也是一个段，所以段是以区为单位申请存储空间的。** 

上面提到一个区默认占用1M存储空间，所以默认情况下有一个索引的表也需要2M的存储空间么？以后每次添加一个索引都要多申请2M的存储空间么？这对于存储记录比较少的表是很大的浪费。
这个问题的根本是申请的区extent都是专区专用的（也就是一个区被整个分配给某一个段，或者说区中的所有页面都是为了存储同一个段的数据而存在的，
即使段的数据填不满区中所有的页面，那余下的页面也不能挪作他用），现在为了考虑以完整的区为单位分配给某个段对于数据量较小的表太浪费存储空间的这种情况，
所以又有了碎片fragment区的概念，也就是在一个碎片区中，并不是所有的页都是为了存储同一个段的数据而存在的，而是碎片区中的页可以用于不同的目的，
例如有些页用于段A，有些页用于段B，有些页甚至哪个段都不属于。**碎片区直属于表空间，并不属于任何一个段**。所以此后为某个段分配存储空间的策略是这样的：

* 在刚开始向表中插入数据的时候，段是从某个碎片区以单个页面为单位来分配存储空间的

* 当某个段已经占用了32个碎片区页面之后，就会以完整的区为单位来分配存储空间

所以现在段不能仅定义为是某些区的集合，更准确定义是某些零散的页面和一些完整的区的集合。除了索引的叶子节点段和非叶子节点段之外，
InnoDB中还有为存储一些特殊的数据而定义的段，例如回滚段

从结构图中可以看到INODE ENTRY的组成：

* Segment ID

	INODE Entry结构对应的段的编号

* NOT_FULL_N_USED

	在NOT_FULL链表中已经使用了多少个页面。下次从NOT_FULL链表分配空闲页面时可以直接根据这个字段的值定位到。而不用从链表中的第一个页面开始遍历着寻找空闲页面
	
* List Base Node For Free List

	同一个段中，所有页面都是空闲的区对应的XDES Entry结构会被加入到这个链表。此处的FREE链表是附属于某个段，
	这样查找某个段的FREE链表的头节点和尾节点的时候，就可以直接到这个部分找到对应链表的List Base Node

* List Base Node For Not_Full List

	同一个段中，仍有空闲空间的区对应的XDES Entry结构会被加入到这个链表，同样附属于某个段
	
* List Base Node For Full List
	
	同一个段中，已经没有空闲空间的区对应的XDES Entry结构会被加入到这个链表，同样附属于某个段
	
* Magic Number

	标记这个INODE Entry是否已经被初始化了（初始化的意思就是把各个字段的值都填进去了）。如果这个数字是值的97937874，表明该INODE Entry已经初始化，否则没有被初始化
	
* Fragment Array Entry

	上面提到段是由零散的页面和一些完整区的集合，每个Fragment Array Entry结构都对应着一个零散的页面，这个结构一共4个字节，表示一个零散页面的页号
	
向段中插入数据的过程：

* 当段中数据较少的时

	首先会查看表空间中是否有状态为FREE_FRAG的区，也就是找还有空闲空间的碎片区，如果找到了，那么从该区中取一些零碎的页把数据插进去；
	否则到表空间下申请一个状态为FREE的区，也就是空闲的区，把该区的状态变为FREE_FRAG，然后从该新申请的区中取一些零碎的页把数据插进去。
	之后不同的段使用零碎页的时候都会从该区中取，直到该区中没有空闲空间，然后该区的状态就变成了FULL_FRAG
	
	一个表空间有很多区，而这个查找的过程目标很明确，例如先查找状态为FREE_FRAG的区。所以MySQL根据区不同的状态创建了对应的链表List Base Node for Free List、
	List Base Node for Free_Frag List、List Base Node for Full_Frag List这三个链表，这三个链表只属于整个表空间，所以在磁盘上的空间固定————第一个区的第一个页的File Space Header部分，下面会提到
	
* 当段中数据已经占满了32个零散的页后
	
	当段中数据已经占满了32个零散的页后，直接申请完整的区来插入数据。申请的区必须属于是这个段的区，同样**属于这个段的区的集合**也分为三种状态，就是这个小节提到的三个链表————List Base Node For Free List、
	List Base Node For Not_Full List、List Base Node For Full List
	
	段在数据量比较大时插入数据，会先获取NOT_FULL链表的头节点，直接把数据插入这个头节点对应的区中即可，如果该区的空间已经被用完，就把该节点移到FULL链表中

--------------------------------------------------

### FSP_HDR类型的页

一个表空间只有一个FSP_HDR这样类型的页，从结构图中可以看出它的组成，**它在整个表空间的位置固定**

#### File Header和File Trailer：

页面的头尾是通用的结构

#### File Space Header：

这个部分是用来存储表空间的一些整体属性

|名称|描述|
|-|-|
|Space ID|表空间的ID|
|Not Used|未被使用 忽略|
|Size|当前表空间占有的页面数|
|FREE Limit|尚未被初始化的最小页号，大于或等于这个页号的区对应的XDES Entry结构都没有被加入FREE链表|
|Space Flags|表空间的一些占用存储空间比较小的属性|
|FRAG_N_USED|FREE_FRAG链表中已使用的页面数量|
|List Base Node for FREE List|FREE链表的基节点|
|List Base Node for FREE_FRAG List|FREE_FRAG链表的基节点|
|List Base Node for FULL_FRAG List|FULL_FRAG链表的基节点|
|Next Unused Segment ID|当前表空间中下一个未使用的 Segment ID|
|List Base Node for SEG_INODES_FULL List|SEG_INODES_FULL链表的基节点|
|List Base Node for SEG_INODES_FREE List|SEG_INODES_FREE链表的基节点|

* List Base Node for FREE List、List Base Node for FREE_FRAG List、List Base Node for FULL_FRAG List。

	这三个链表就是上面提到的维护不同区状态的链表，分别是直属于表空间的FREE链表的基节点、FREE_FRAG链表的基节点、FULL_FRAG链表的基节点
	
* FRAG_N_USED

	在FREE_FRAG链表中已经使用的页面数量
	
* FREE Limit

	该字段表示的页号之前的区都被初始化了，之后的区尚未被初始化
	
* Next Unused Segment ID

	上面提到表中每个索引都对应2个段，每个段都有一个唯一的ID，那当我们为某个表新创建一个索引的时候，就意味着要创建两个新的段。
	那怎么为这个新创建的段找一个唯一的ID呢？最笨的办法就是去遍历现在表空间中所有的段，而该字段表明当前表空间中最大的段ID的下一个ID，
	所以直接使用这个字段的值就好了，同时对这个字段自增1。这种方法在开发过程中也很实用
	
* Space Flags

	这个字段主要是为表空间一些属性只占几位而设立的，32位的大小，如下表：
	
	|名称|占用位|描述|
	|-|-|-|
	|POST_ANTELOPE|1|文件格式是否大于ANTELOPE|
	|ZIP_SSIZE|4|压缩页面的大小|
	|ATOMIC_BLOBS|1|是否自动把值非常长的字段放到BLOB页里|
	|PAGE_SSIZE|4|页面大小|
	|DATA_DIR|1|表空间是否是从默认的数据目录中获取的|
	|SHARED|1|是否为共享表空间|
	|TEMPORARY|1|是否为临时表空间|
	|ENCRYPTION|1|表空间是否加密|
	|UNUSED|1|没有使用到的比特位|

* List Base Node for SEG_INODES_FULL List和List Base Node for SEG_INODES_FREE List

	上面提到每个段对应的INODE Entry结构，从结构图中看到有一个类型为INODE的页中会集中存放这些结构，如果表空间中的段特别多，则会有多个INODE Entry结构，可能一个页放不下，这些INODE类型的页会组成两种列表：
	
	+ SEG_INODES_FULL链表，该链表中的INODE类型的页面都已经被INODE Entry结构填充满了，没空闲空间存放额外的INODE Entry结构
	
	+ SEG_INODES_FREE链表，该链表中的INODE类型的页面仍有空闲空间来存放INODE Entry结构

#### XDES Entry 0...255：

有256个这样的结构，而上面提到每个这样的结构代表一个区，所以当前组所有区的结构都存储在这块连续的空间

--------------------------------------------------

### XDES类型的页

从结构图看出除了第一组外余下的每组的第一个区的首页是XDES类型的页，也看到它和FSP_HDR类型的页的差别就是少了File Space Header部分，也就是除了少了记录表空间整体属性的部分之外，其余的部分是一样的

--------------------------------------------------

### INODE类型的页

INODE类型的页就是为了存储INODE Entry结构而存在的，从结构图看出INODE类型的页组成，不同的部分就是List Node For Inode Page List、85个Inode Entry结构，上面提到Inode Entry结构对应一个段，这样的页面可以存储85个段

#### List Node For Inode Page List

一个表空间中可能存在超过85个段，所以可能一个INODE类型的页面不足以存储所有的段对应的INODE Entry结构，所以就需要额外的INODE类型的页面来存储这些结构。为了方便管理这些INODE类型的页面，
同时为了查询效率将这些页面分为两种类型的链表来管理，FSP_HDR类型页File Space Header部分内存储着这两种类型链表的基结点List Base Node for SEG_INODES_FULL List、List Base Node for SEG_INODES_FREE List，
这两个链表的基节点的位置是固定的，所以我们可以很轻松的访问到这两个链表。以后每当我们新创建一个段（创建索引时就会创建段）时，都会创建一个INODE Entry结构与之对应，存储INODE Entry的大致过程就是这样的：

* 先看看SEG_INODES_FREE链表是否为空，如果不为空，直接从该链表中获取一个节点，也就相当于获取到一个仍有空闲空间的INODE类型的页面，然后把该INODE Entry结构放到该页面中。当该页面中无剩余空间时，
就把该页放到SEG_INODES_FULL链表中

* 如果SEG_INODES_FREE链表为空，则需要从表空间的FREE_FRAG链表中申请一个页面，修改该页面的类型为INODE，把该页面放到SEG_INODES_FREE链表中，与此同时把该INODE Entry结构放入该页面


