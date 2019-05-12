---
layout: second_template
title: 优先使用const_iterator而不是iterator
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: const_iterator
---

***
* #### 缺陷 ####

	实际编程时都会遇见遍历容器，找到目标后做出相应的逻辑动作，有些时候我们不涉及到修改实际内容，这时我们想到的就是增加const修饰词来达到禁止修改的效果，使用C++98时没有直接的办法获取const_iterator，可以通过强制类型转换和绑定value到一个const引用变量，同时，如果有这样的需求：找到目标值后插入一个值，那么这样就又会带来麻烦，C++98中，插入或者删除元素的定位只能使用iterator， const_iterator 是不行的

***
* #### 改善 ####

	C++11中const_iterator 既容易获得也容易使用。容器中成员函数cbegin和cend可以产生const_iterator，甚至非const的容器也可以这样做，STL成员函数通常使用const_iterator来进行定位（也就是说，插入和删除insert and erase）：

		{
			std::vector<int> vec = { 1,2,3 };
			auto it = std::find(vec.cbegin(),vec.cend(),3);
			
			vec.insert(it, 4);
		}

	C++11对于const_iterator的支持不够全面，当我们想编写最通用的库代码时，例如编写一个对容器查找并插入的函数，这时我们必须考虑传统容器或者自定义类似容器，最通用的代码会使用非成员函数（begin、end、cbegin、cend、rbegin），而不会假定提供了成员函数：

		template <class C,class V>
		void FindAndInsert(C& container, const V& targetVal,const V& insertVal)
		{
			auto it = std::find(std::cbegin(container), std::cend(container), targetVal);
			container.insert(it,insertVal)
		}

	上述代码在C++11中编译不通过，在C++14中可以通过，C++11只支持非成员函数的begin和end，而没有cbegin、cend、rbegin、rend、crbegin和crend。我们可以手动编写一个cbegin函数：

		template <class C>
		auto cbegin(const C& container)->decltype(std::begin(container))
		{
			return std::begin(container);
		}

	我们这里并没有调用cbegin成员函数是因为实参可以是任何表示类似容器的数据结构，并通过其引用到const型别的形参container来访问该实参。例如C是对应一个传统容器型别的`std::vector<int>`，则container的类型便是`const std::vector<int>&`，**调用C++11提供的非成员函数版本的begin函数并传入一个const容器会产生一个const_iterator,而上述例子cbegin返回的正是这个迭代器。这样实现的好处是它对于只提供了begin成员函数而未提供cbegin成员函数的容器也适用，这样就可以把这个非成员函数版本的cbegin应用在仅直接支持begin的容器上了，上述例子中C是一个内建数组时也适用，C++11的非成员函数begin为数组提供了特化版本，它返回一个指向到数组首元素的指针，由于const数组的元素都是const的，所以若给非成员函数begin传入一个const数组，则返回的指针是个指向到const的指针**


***
* #### 最后 ####

	1. 优先选用const_iterator,而非iterator

	2. 在最通用的代码中，优先选用非成员函数版本的begin、end和rbegin等，而不是其成员函数版本

