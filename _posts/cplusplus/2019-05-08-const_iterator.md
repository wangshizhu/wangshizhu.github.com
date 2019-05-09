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
			if (it == vec.cend())
			{
				cout << "Dont find target val" << endl;
			}
			else
			{
				vec.insert(it, 4);
			}
		}