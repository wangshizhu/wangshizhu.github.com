---
layout: second_template
title: constexpr
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: constexpr
---

***
* ### constexpr ###

	constexpr应用于对象时，其实就是加强版的const，但是应用于函数时，却有着不同的含义。所以分成两部分说明它


*** 
* ### constexpr作用于对象 ###

	**当它作用于对象时，具备const属性；同时在编译阶段已知。**在编译阶段就需要确定常量整型值的语境包括数组长度、整型模板实参（std::array型别对象的长度）、枚举常量值、对齐规格等，
	这样一来编译器就能保证一个编译期的值。例如：

		constexpr auto arraySize = 10; // 10是一个编译期常量
		std::array<int,arraySize> data;

	而const 并未提供和constexpr同样的保证，因为const对象并不一定经由编译期已知值来初始化。例如：

		int sz;
		...
		const auto arraySize = sz;
		std::array<int,arraySize> data; // 错误，arraySize的非编译期已知

	**所以总结下来，所有constexpr对象都是const对象，而并不是所有的const对象都是constexpr对象。如果想让编译器提供保证，让变量拥有一个值，用于要求编译期常量的语境，那么使用constexpr就能达到这个目的，而不是const**


***
* ### constexpr作用于函数 ###

	




	







