---
layout: second_template
title: constexpr
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: constexpr
---


constexpr应用于对象时，其实就是加强版的const，但是应用于函数时，却有着不同的含义。所以分成两部分说明它



### constexpr作用于对象
--------------------------------------------------

**当它作用于对象时，具备const属性；同时在编译阶段已知。**在编译阶段就需要确定常量整型值的语境包括数组长度、整型模板实参（std::array型别对象的长度）、枚举常量值、
对齐规格等，这样一来编译器就能保证一个编译期的值。例如：

	constexpr auto arraySize = 10; // 10是一个编译期常量
	std::array<int,arraySize> data;

而const 并未提供和constexpr同样的保证，因为const对象并不一定经由编译期已知值来初始化。例如：

	int sz;
	...
	const auto arraySize = sz;
	std::array<int,arraySize> data; // 错误，arraySize的非编译期已知

**所以总结下来，所有constexpr对象都是const对象，而并不是所有的const对象都是constexpr对象。如果想让编译器提供保证，让变量拥有一个值，
用于要求编译期常量的语境，那么使用constexpr就能达到这个目的，而不是const**


### constexpr作用于函数
--------------------------------------------------
	
* constexpr函数可以用在要求编译期常量的语境中，若传给一个constexpr函数的实参值是在编译期已知的，则结果也会在编译期间计算出来，如果任何一个实参值在编译期未知，
则代码将无法通过编译

* 在调用constexpr函数时，若传入的值有一个或多个在编译期未知，则它的运作方式和普通函数没区别，也就是在运行期才能得到结果，
所以编写编译期常量函数和运行期函数可以用一个constexpr函数代替

例如我们自己实现一个类似std::pow函数，标准库函数pow的工作对象是浮点型别，我们自己实现的函数只需整型结果，其次，std::pow并不是constexpr，
所以如果我们想通过这个函数的返回值创建std::array是不可能的

	constexpr int pow(int nBase,int nExp) noexcept
	{
		return nExp == 0 ? 1 : base * pow(nBase,nExp - 1);
	}
	
	constexpr auto num = 5;
	
	std::array<int,pow(3,num)> results;

上面函数应该解读为：如果nBase、nExp是编译期常量，那么pow的结果就可以当做编译期常量；如果nBase、nExp中有一个不是编译期常量，则pow的返回结果就将在执行期计算；
所以这个函数在编译期常量语境和运行期语境中都适用

**只有当constexpr函数的参数在传入编译期常量时才能返回编译期结果，对于这样的语境C++11要求这样的函数不得多于一个可执行语句，即一条return语句。
所以上述自定义pow函数使用了条件运算符?:，避免了if-else，用到循环的地方用递归代替。而在C++14中这种限制放宽了**，所以也可以像下面这样写：

	constexpr int pow(int nBase,int nExp) noexcept
	{
		auto result = 1;
		for (int i=0;i<exp;++i)
		{
			result *= base;
		}
		return result;
	}
	
在型别方面，constexpr函数只限于传入和返回字面型别literal type，意思就是这样的型别能够在编译期可以决议的值，在C++11中所有的内建型别（除了void）都可以。
同时自定义的型别也可以（只要满足编译期能够确定其值就可以），可以让它的构造函数和其他成员函数是constexpr，例如游戏里面描述二维的点Point类：

	class CPoint
	{
	public:
		constexpr CPoint(double x, double y) noexcept : m_fX(x),m_fY(y)
		{
		}
	
		constexpr double getX() const noexcept 
		{
			return m_fX;
		}
	
		constexpr double getY() const noexcept
		{
			return m_fY;
		}
	
		void setX(double x) noexcept
		{
			m_fX = x;
		}
	
		void setY(double y) noexcept
		{
			m_fY = y;
		}
	
	private:
		double m_fX;
		double m_fY;
	};

这个类CPoint可以是字面性别，当传入的参数是编译期能够确定的值那么创建的对象也就在编译期能够确定其值，同时它的获取器getter也可以是constexpr，
对象能在编译期确定那么其成员也是编译期已知，所以下面代码在编译期也成立：

	constexpr CPoint midPoint(const CPoint obj1,const CPoint obj2) noexcept
	{
		return CPoint((obj1.getX()+obj2.getX())/2,(obj1.getY()+obj2.getY())/2);
	}
	
	constexpr auto mid = midPoint(p1,p2);
	
	std::array<int, static_cast<int>(mid.getX() * 10)> a;

上述类的实现里设置器setter并不是constexpr，原因在于他们修改对象内的成员数据，并且在C++11里constexpr是隐式的const，还有就是我们上面提到的返回值是void,
void并不是字面类型，但是在C++14里这样的限制被丢弃了，所以设置器setter可以这样写：

	constexpr void setX(double x) noexcept
	{
		m_fX = x;
	}
	constexpr void setY(double y) noexcept
	{
		m_fY = y;
	}


所以下面实现在编译期也成立：

	constexpr CPoint reflectPoint(const CPoint obj) noexcept
	{
		CPoint src;
		src.setX(-obj.getX());
		src.setY(-obj.getY());
	
		return src;
	}
	
	constexpr auto obj = reflectPoint(p);

obj的值在编译期就已知

### 最后
--------------------------------------------------
	
编译期能够做出决策是为了我们的软件运行的更快，但是这样编译时间就会变长

* constexpr是对象或者函数的修饰词

* constexpr对象或者函数有着更宽泛的语意比非constexpr

* 当参数是编译期能够确定值那么constexpr函数可以产生编译期的值

* 使用constexpr时要尽量有一个长期的设计保证，开发过程中如果去掉constexpr可能导致一系列的错误





	







