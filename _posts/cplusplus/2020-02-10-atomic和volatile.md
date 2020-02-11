---
layout: second_template
title: std::atomic和volatile
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: atomic_volatile
---

**在C++中volatile与并发程序设计毫无关系，可能在其他程序语言中如Java、C#中会对并发程序设计有用武之处**

**std::atomic和并发程序设计有很大关系，std::atomic模板实例提供的操作可以保证被其他线程视为原子的，一个st::atomic型别对象，针对它的操作就好像这些操作处于受互斥量保护的临界区域内一样，
实际上这些操作通常会使用特殊的机器指令来实现，这些指令比使用互斥量更高效**

### std::atomic
--------------------------------------------------

**std::atomic最好的特性之一是：一旦构造出std::atomic型别对象，在其上所有的成员函数（包括那些包含RMW操作的成员函数）都保证被其他线程视为原子的**，例如：
	
	std::atomic<int> ai(0);
	ai = 10; // 保证原子性的设置
	std::cout << ai << std::endl; // 保证原子性的读取
	++ai; // 保证原子性的自增
	--ai; // 保证原子性的自减
	
ai的自增、自减属于成员函数，其操作是读取-修改-写入（read-modify-write）即RMW操作，上面提到所有成员函数都保证被其他线程视为原子的

但是本例中语句`std::cout << ai << std::endl;`正如注释写到，**保证了原子性的读取，对于整条语句而言，并不能保证是原子性的**，因为在读取ai的值和调用operator<<之间另一个线程
有可能已经修改了ai的值，这对于整条语句的行为没有影响，因为整型的operator<<会使用按值传递的int型别的形参来输出，因此输出的值会是从ai读取的值

在使用std::atomic时避免出现下面的赋值行为：
	
	auto otherVar = ai; // 编译失败，复制操作被删除了
	
变量otherVar被推导为std::atomic<int>，为了使得从ai出发来构造otherVar的操作也是原子操作，编译器必须生成代码来在单一的原子操作中读取ai并写入otherVar，硬件通常无法完成这样的操作，所以std::atomic
不支持复制构造、复制赋值、移动构造、移动赋值运算符，通常使用使用如下方式：

	std::atomic<int> otherVar(ai.load());
	
或者：
	
	std::atomic<int> otherVar(0);
	....
	otherVar.store(ai.load());
	
### volatile
--------------------------------------------------

开头提到在C++中volatile与并发程序设计毫无关系，下面在多线程环境下的例子：
	
	volatile int volatileVar = 0;
	volatileVar = 10;
	std::cout << volatileVar << std::endl;
	++volatileVar;
	++volatileVar;
	
当一个线程做写操作，一个线程做读操作，那么读出来的值很可能产生未定义行为，没有使用std::atomic，也没有使用互斥量保护，这就是数据竞争风险。在例如：
	
	volatile int volatileVar = 0;
	/*线程1*/				/*线程1*/
	++volatileVar;			++volatileVar;

有可能发生如下情况：

* 线程1把读取的值0自增为1，并写入volatileVar
* 线程2把读取的值0自增为1，并写入volatileVar

最后volatileVar为1，显然这并不是我们期望的

### atomic和volatile
--------------------------------------------------

volatile到底能在什么场景下能使用，什么场景下不能使用

atomic到底能在什么场景下能使用，什么场景下不能使用

从上面两个小节中得知**在多线程中std::atomic更适合RMW操作，而volatile并不适合**

下面的例子也显示了在多线程环境中除了RMW操作外std::atomic比volatile更合适：
	
	std::atomic<bool> atomicVar(false);
	auto val = computeAValue();
	atomicVar = true;
	
这个例子在多线程环境中经常见到，一个线程写入这个变量值为true，而另一个线程依赖这个值做判断，从而做出相应逻辑处理，当阅读这段代码时，我们收到一个强烈的信息，就是语句`auto val = computeAValue();`
必须在这个语句`atomicVar = true;`之前，如果去掉atomic，像这样：

	bool atomicVar(false);
	auto val = computeAValue();
	atomicVar = true;
	
这两个例子的赋值语句都是不相关的赋值操作，那么编译器极有可能将其重新排序已达到运行高效，即使编译器不这样做，底层硬件也有可能这样做，例如变成下面这样：
	
	atomicVar = true;
	auto val = computeAValue();
	
这显然不是我们期望的，而如果使用了atomic则会对代码可以如何重新排序增加限制，限制之一是在源代码中不得将任何代码**提前**至后续会出现atomic型别变量的写入操作的位置，不仅告诉编译器必须保证编写的顺序，
还得保证生成的代码也依然是编写的顺序，以确保底层硬件按照这个顺序执行。

而volatile并不具备刚刚提到的保持顺序的特性，**volatile的用处是告诉编译器正在处理的内存不具备常规行为，不要对在此内存上的操作做任何优化**，常规行为的特征是：

* 如果向某个内存位置写入了值，该值一直保留，直到被覆盖

	例如：
	
		int x;
		auto y = x;
		y = x;
	
	编译器消除冗余代码：
		
		int x;
		auto y= x;
	
* 如果向某内存位置写入值，一段时间未读取该内存位置，那么再次写入该内存位置，则第一次写入可以消除
	
	例如：
	
		x = 10;
		x = 20;
	
	编译器消除冗余代码：
		
		x = 20;
		
我们暂且称不具备常规行为的内存为特种内存，对于特种内存编译器做出这样的优化是不能接受的，特种内存操作通常用于与外部设备（显示器、打印机、网络端口等）通信，例如刚刚提到的例子：
	
	x = 10;
	x = 20;
	
对于外部设备可能是两个不同的命令，而这时使用volatile就可以告诉编译器不要对在此内存上的操作做任何优化，所以修改如下：
	
	volatile int x;
	x = 10;
	x = 20;
	
在处理特种内存时必须保留看似冗余的代码，而std::atomic型别对象不适用于这种工作，编译器可以消除std::atomic型别上的冗余操作，例如：
	
	std::atomic<int> x;
	std::atomic<int> y(x.load());
	y = x.load();
	
	x = 10;
	x = 20;

优化后：

	std::atomic<int> y(x.load());
	x = 20;
	
### 最后
--------------------------------------------------

* std::atomic对于并发程序设计有用，且不用互斥量，但不能用于访问特种内存
* volatile 对于访问特种内存有用，但不能用于并发程序设计

虽然两者用于不同的目的，当然也有两者同时适用的场景，例如：

	// 针对val的操作是原子的，并且不可被优化掉
	volatile std::atomic<int> val;


	