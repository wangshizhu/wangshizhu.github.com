---
layout: second_template
title: 智能指针之std::make_unique和std::make_shared
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: make_unique&make_shared
---

[initialize_way]: /initialize_way

[weak_ptr]: /weak_ptr

* * *
* ## 解释

	std::make_shared 是C++ 11标准的一部分

	std::make_unique 是C++ 14标准的一部分

	如果使用的是C++11可以自己包装一个简单的make_unique，像下面这样：

	`
	template<typename T, typename... Ts>
	std::unique_ptr<T> make_unique<Ts&&... params>
	{
		return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
	}
	`

	当自己包装make_unique时，注意把这个包装放到自定义的namespace下，避免和std冲突

	std::make_unique 以及 std::make_shared是3个make函数的其中2个：make函数接受任意数量的参数，
	然后将他们完美转发给动态创建的对象的构造函数，并且返回指向那个对象的智能指针。第三个make函数是 std::allocate_shared ,除了第一个参数是一个用来动态分配内存的allocator对象，它表现起来就像std::make_shared

* * *

* ## 使用make而不是new 

	下面以std::make_shared为代表说明，对应的说明同样适用于std::make_unique
	
	- 避免代码重复	

	使用new需要重复写一遍type，而使用make函数不需要。重复敲type违背了软件工程中的一项基本原则：代码重复应当避免。
	源代码里面的重复会使得编译次数增加,导致对象的代码变得臃肿，由此产生出的code base变得难以改动以及维护。它经常会导致产生不
一致的代码。一个code base中的不一致代码会导致bug

	- 使用make函数的原因是为了保证产生异常后程序的安全

	考虑如下代码：

	`
	int ComputeParam();
	void ProcessPerson(std::shared_ptr<Person> sharedPerson,int nParam);
	ProcessPerson(std::shared_ptr<Person>(new Person),ComputeParam())
	`

	这样的代码会产生因new引发的Person对象的内存泄露。函数的声明和调用函数的代码都使用了std::shared_ptr,设计std::shared_ptr的目的
	就是防止内存泄露。在函数被调用前，函数的参数必须被推算出来，所以在调用ProcessPerson的过程中，
	ProcessPerson开始执行之前，下面的事情必须要发生：

	1. "new Person"表达式必须被执行，即，一个Person必须在堆上被创建

	2. 负责管理new所创建的指针的 std::shared_ptr<Person> 的构造函数必须被执行

	3. ComputeParam必须被执行

	从上面的代码看出"new Person"动作必须在std::shared_ptr的构造函数被调用之前执行，编译器产生出这些操作不一定按照上面的顺序，有可能按照如下顺序：

	1. "new Person"表达式必须被执行，即，一个Person必须在堆上被创建

	2. ComputeParam必须被执行

	3. 负责管理new所创建的指针的 std::shared_ptr<Person> 的构造函数必须被执行

	如果按照这样的顺序时有可能调用ComputeParam函数时抛出异常，导致"new Person"内存泄漏，而使用std::make_shared 可以避免这个问题，
	**在runtime的时候，std::make_shared 或者ComputeParam都有可能被第一次调用。如果是std::make_shared先被调用,
	被动态分配的Person安全的存储在返回的 std::shared_ptr 中
(在ComputeParam被调用之前)。如果ComputeParam产生了异常， std::shared_ptr 的析构
函数会负责把它所拥有的Person回收。如果ComputeParam首先被调用并且产生出一个异
常，std::make_shared不会被调用，因此也不必担心动态分配的Person会产生泄漏的问题**

	- 效率比new高

	一个使用 std::make_shared （和直接使用new相比）的显著特性就是提升了效率。使用
	std::make_shared允许编译器利用简洁的数据结构产生出更简洁，更快的代码。考虑下面直
	接使用new的效果`std::shared_ptr<Person> sharePerson(new Person);`
	实际上它执行了两次内存分配。std::shared_ptr都指向了一个包含被指向对象的引用计数的控制块，控制块的分配工作在
	std::shared_ptr的构造函数内部完成。直接使用new，就需要一次为Person分配内存，第二次
	需要为控制块分配内存。如果使用的是std::make_shared, `auto sharePerson = std::make_shared<Person>();` 
	**一次分配足够了。这是因为std::make_shared分配了一整块空间，包含了Person对象和控制
块。这个优化减少了程序的静态大小，因为代码中只包含了一次分配调用，并且加快了代码
的执行速度，因为内存只被分配一次。此外，使用std::make_shared避免了在控制块中额外
添加的一些记录信息的需要，潜在的减少了程序所需的总内存消耗**
	
	上文的 std::make_shared 效率分析同样使用于std::allocate_shared，所以std::make_shared
的性能优点也可以延伸到std::allocate_shared函数

* * *
* ## 使用make的限制

	1. make函数都不支持指定自定义的deleter，但是std::unique_ptr以及std::shared_ptr都有构造函数来支持这样

	2. make函数转发参数给对象的构造函数，但是它使用的是括号“()”而非大括号“{}”,所以对于make函数不具备转发以大括号“{}”形式的参数给对象的构造函数。关于大括号{}和小括号()在初始化时的区别请参考之前的这篇[文章][initialize_way]，
	但是可以通过这种间接方式达到这种效果`auto initList = {100,200}; auto sharedPerson = std::make_shared<Person>(initList)`

	3. 对于std::unique_ptr的限制只是以上两种场景，而对于std::shared_ptr不只是以上两种场景。对于自定义了内存管理的类(类自身定义了 opeator new 和 operator delete) 通常仅用来分配和释放与该类同等大小的内存块。这样的类不适合使用std::make_shared创建对象，首先std::shared_ptr所支持的自定义分配器通过std::allocate_shared实现，而std::allocate_shared所要求的内存大小并不等于要动态分配对象的大小，而是该动态分配对象的大小加上控制块的大小，因此，使用make系列函数去为带有自定义版本的opeator new 和 operator delete的类创建对象是一个坏主意

	4. 使用std::make_shared相对于直接使用new的大小及性能优点源自于：std::shared_ptr的控制 块是和被管理的对象放在同一个内存区块中。当该对象的引用计数变成了0，该对象被销毁 （析构函数被调用）。但是，它所占用的内存直到控制块被销毁才能被释放，因为被动态分配的内存块同时包含了两者。控制块包含两个引用计数，第1个引用计数记录了多少个std::shared_ptr引用了当前的控制块，第2个引用计数记录了多少个std::weak_ptr引用了当前的控制块（关于std::weak_ptr的介绍可以参考这篇[文章][weak_ptr]）。第2个引用计数被称之为弱引用计数。std::weak_ptr通过检查控制块里的第1个引用计数来校验自己是否有效，假如第1个引用计数为0，则std::weak_ptr就已失效。**只要有一个std::weak_ptr还引用着控制块(即，第2个引用计数大于0)，控制块就会继续存在，包含控制块的内存就不会被回收。被std::shared_ptr的make函数分配的内存直至指向它的最后一个std::shared_ptr和最后一个std::weak_ptr都被销毁时，才会得到回收**，所以这里就出现最后一个std::shared_ptr的析构与最后一个std::weak_ptr析构之间的 间隔时间问题，该对象被析构与它所占用的内存被回收之间也会产生间隔