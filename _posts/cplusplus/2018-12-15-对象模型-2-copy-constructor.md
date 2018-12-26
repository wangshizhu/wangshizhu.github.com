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

	如果class 没有提供一个显示的copy constructor，其内部是以所谓的默认成员逐次初始化(default memberwise initialization)手法完成的，也就是把每个内建的或派生的data member的值从某个object拷贝到另一个object。

	这个拷贝操作并不会其中的member class object,而是以递归的方式再次实施default memberwise initialization，其完成方式就好像分别设定每个data member

	例如：

		class Word
		{
		private:
			int m_nID;
			String m_strDescribe;
		}

	上面class Word含有一个class string object， 这时Word Object的default memberwise initialization会拷贝m_nID;再于String
	member object m_strDescribe身上递归实施memberwise initialization

	copy constructor 也像default constructor的描述一样————————在必要的时候才由编译器产生出来，也同样分为trivial 和 nontrivial两种。只有nontrivial的实例才会被合成于程序中，那么决定一个copy constructor是否为trivial的标准在于class是否展现出所谓的“bitwise copy semantics”

	一个class object可用两种方式复制得到，一种是被初始化（上面提到的），一种是被指定（assignment），从概念上讲这两个操作分别是以copy constructor 和 copy assignment operator完成的

* 位逐次拷贝(bitwise copy semantics)

	如果一个类：

		class Word
		{
		public:
			Word(const char* pDescribe);
		private:
			int m_nID;
			char* m_pDescribe;
		}

	这个类表现出了bitwise copy semantics,所以不需要合成出一个default copy constructor

	对这个做一下修改：

		class Word
		{
		public:
			Word(const String& strDescribe);
		private:
			int m_nID;
			String m_strDescribe;
		}

	而String显示的声明了copy constructor,此时编译器必须合成出一个copy constructor,nonclass member data依次拷贝，同时调用member class String object的
	copy constructor，合成出类似的代码：

		inline Word::Word(const Word& objWord)
		{
			m_strDescribe.String::String(objWord.m_strDescribe);
			m_nID = objWord.m_nID;
		}

* 没有bitwise copy semantics的情况

	什么时候一个class不展现出“bitwise copy semantics”?

	1. 当class内含有一个member object，而这个class声明了一个copy constructor时，不论这个class是显示的声明还是被编译器合成（像上面的例子）

	2. 当 class继承自一个base class而后者存在一个copy constructor时，同样不论这个class是显示的声明copy constructor还是被编译器合成copy constructor

	3. 当class声明一个或多个virtual function时

	4. 当class派生自一个继承链，其中有一个或多个virtual base classes时

