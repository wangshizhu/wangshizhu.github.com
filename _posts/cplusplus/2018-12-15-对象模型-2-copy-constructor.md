---
layout: second_template
title: copy-constructor的构造操作
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: copy-constructor
---

* 测试环境
	
	vs2015

	C++11	

	win10 64位

* Copy Constructor

	有三种情况会以一个object内容作为另一class object的初值

	- 显示的初始化操作 

	`
	class X {}; 
	X objX; 
	X objX1 = objX;
	`

	- object 作为参数

	`
	void foo(X objX) 
	void bar()
	{
		X objTmp;
		foo(objTmp);
	}
	`

	- object作为返回值

	`
	X foo() 
	{ 
		X objTmp; 
		return objTmp;
	}
	`

* 默认成员逐次初始化