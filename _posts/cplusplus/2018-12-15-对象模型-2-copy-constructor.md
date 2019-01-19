---
layout: second_template
title: copy-constructor的构造操作
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: copy-constructor
---

* * *
* 测试环境
	
	vs2015

	C++11	

	win10 64位

* * *
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

* * *
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

	copy constructor 也像default constructor的描述一样——在必要的时候才由编译器产生出来，也同样分为trivial 和 nontrivial两种。只有nontrivial的实例才会被合成于程序中，那么决定一个copy constructor是否为trivial的标准在于class是否展现出所谓的“bitwise copy semantics”

	一个class object可用两种方式复制得到，一种是被初始化（上面提到的），一种是被指定（assignment），从概念上讲这两个操作分别是以copy constructor 和 copy assignment operator完成的

* * *
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

* * *
* 没有bitwise copy semantics的情况

	什么时候一个class不展现出“bitwise copy semantics”?

	1. 当class内含有一个member object，而这个class声明了一个copy constructor时，不论这个class是显示的声明还是被编译器合成（像上面的例子）

	2. 当 class继承自一个base class而后者存在一个copy constructor时，同样不论这个class是显示的声明copy constructor还是被编译器合成copy constructor

	3. 当class声明一个或多个virtual function时

	4. 当class派生自一个继承链，其中有一个或多个virtual base classes时

	重点说明3、4情况

	- 当class声明一个或多个virtual function时

	`
	class ZooAnimal
	{
	public:
		ZooAnimal()
		{
		}
		virtual ~ZooAnimal()
		{
		}
		virtual void animate()
		{
		}
		virtual void draw()
		{
		}
	};
	class Bear : public ZooAnimal
	{
	public:
		Bear()
		{
		}
		void animate()
		{
		}
		void draw()
		{
		}
		virtual void dance()
		{
		}
	};
	int main()
	{
		{
			Bear objBearA;
			Bear objBearB = objBearA;
			ZooAnimal za = objBearA;
		}
		system("pause");
	    return 0;
	}
	`

	以上述代码为例，我们发现class Bear object objBearA与class Bear object objBearB的vptr相等，以及vptl内的函数指针也相同，
	因此以同类型的object作为另一个object的初值时可以直接靠"bitwise copy semantics"完成。

	同时`ZooAnimal za = objBearA;`发现class ZooAnimal object za的vptr与class Bear object objBearA不同，如果还是直接靠"bitwise copy semantics"完成就会导致vptr相等，这时编译器合成出ZooAnimal copy constructor，
	并且显示的设定za的vptr指向ZooAnimal class的virtual table，而不是从=的右边object直接拷贝过来

	- 当class派生自一个继承链，其中有一个或多个virtual base classes时

	**virtual base class的存在需要特别处理。一个class object如果以另一个object作为初值，而后者有一个virtual base class subject，那么也会使"bitwise copy semantics"失效**

	每一个编译器对于虚拟继承的支持都必须让"derived class object"中的virtual base class subobject位置在执行期就准备好，维护位置的完整性是编译器的责任。而"bitwise copy semantic"可能会破坏这个位置，所以编译器必须在它自己合成出来的copy constructor中做出选择

	在上面第3种情况总结出"bitwise copy semantics"失效是发生于“一个class object以其derived classes的某个object作为初值”之时

	`
	class Raccoon : public virtual ZooAnimal
	{
	public:
		Raccoon()
		{
		}
	};
	`
	
	编译器所产生的代码：

	1. 用以调用ZooAnimal的default constructor

	2. 浆Raccoon的vptr初始化

	3. 定位出Raccoon中的ZooAnimal subobject

	`
	class Raccoon : public virtual ZooAnimal
	{
	public:
		Raccoon()
		{
		}
	};
	class RedPanda : public Raccoon
	{
	public:
		RedPanda()
		{
		}
	};
	{
		RedPanda objectRP;
		Raccoon objectR = objectRP;
	}
	`

	上面代码`RedPanda objectRP; Raccoon objectR = objectRP;`为了正确的设定objectR的初值，编译器必须合成一个copy constructor，安插一些代码设定virtual base class pointer/offset的初值，并且对每一个members执行必要的memberwise初始化操作，以及执行其他内存相关操作
	

