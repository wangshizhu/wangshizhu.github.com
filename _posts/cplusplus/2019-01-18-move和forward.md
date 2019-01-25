---
layout: second_template
title: std::move和std::forward
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: move&forward
---
[move_constructor]: /move-constructor
* * *

* #### 解释 ####

	std::move和std::forward只是执行转换的函数模板

	std::move无条件的将它的参数转换成一个右值，而std::forward当特定的条件满足时，才会执行它的转换

* * *
* #### std::move ####

	std::move 一种实现样例:

		template <class T>
		std::remove_reference_t<T>&& move_define(T&& param)
		{
			return static_cast<std::remove_reference_t<T>&&>(param);
		}

	参数T&& param并不一定接受的参数类型就是右值引用，但是函数返回值的"&&"部分表明std::move返回的是一个右值引用，我们知道，
	如果T的类型恰好是一个左值引用，T&&的类型就会也会是左值引用。关于它为什么是左值引用我们以后讨论，所以阻止这种事情的发生，
	我们通过类型萃取，去除T身上的引用，确保返回值是个右值引用，**它不会去除const修饰符，如果param带有const修饰符，那么返回值是带有
	const修饰符的右值引用，我们之前讨论过移动构造函数(关于移动构造函数可以参考这篇[文章][move_constructor])的参数是非const的右值引用，所以当以
	const右值引用构造对象时，并不会调用移动构造函数，因为const左值引用可以被const右值匹配上所以调用了拷贝构造函数，总结下来：如果你想对这些对象执行move操作，就不要把它们声明为const，对const对象的move请求通常会悄悄的执行到copy操作上**，下面我们通过代码示例验证我们的总结：

		class CTest
		{
		public:
			CTest()
			{
				cout << "default constructor" << endl;
			}
			CTest(const CTest& param)
			{
				cout << "copy constructor" << endl;
			}
			CTest(CTest&& param)
			{
				cout << "move constructor" << endl;
			}
		};

		void TestFun(const CTest& objTestParam)
		{
			CTest objTest(std::move(objTestParam));
		}

		{
			CTest objTest;
			TestFun(objTest);
		}

	先以const左值引用作为参数：

		default constructor
		copy constructor

	再以非const左值引用作为参数：

		void TestFun(CTest& objTestParam)
		{
			CTest objTest(std::move(objTestParam));
		}

	输出：

		default constructor
		move constructor

	两次输出验证了上面的总结

* * *
* #### std::forward ####

	和std::move无条件地将它的参数转化为右值不同,std::forward在特定的条件下才会执行转化。std::forward是一个有条件的转化，forward用的最多的场景是做参数转发，我们当然是希望我们传入左值/右值，那么传入目标函数的参数依然是左值/右值

		void Process(CTest& objTestLR)
		{
			cout << "Process(CTest& objTestLR)" << endl;
		}

		void Process(CTest&& objTestRR)
		{
			cout << "Process(CTest&& objTestRR)" << endl;
		}

		template<class T>
		void InvokeProcess(T&& param)
		{
			Process(std::forward<T>(param));
		}

	对于上面的代码按照下面的调用方式进行调用：

		{
			CTest objTest;
			InvokeProcess(objTest);
			InvokeProcess(std::move(objTest));
		}

	至于forward怎么知道传入的参数是左值还是右值我们以后在讨论

* * *
* #### std::move和std::forward ####
	
	**std::move只需要一个函数参数, std::forward不只需要一个函数参数,还需要一个模板类型参数。这就意味着std::move比std::forward用起来更方便,
	免去了让我们传递一个表示函数参数是否是一个右值的类型参数。消除了传递错误类型的可能性。**

	**std::move的使用表明了对右值的无条件的转换，然而，当std::forward只对被绑定了右值的reference进行转换。std::move就是为了移动语意操作而生，
	而std::forward,就是将一个对象转发(或者说传递)给另外一个函数，同时保留此对象的左值性或右值性，
	std::move和std::forward在runtime时什么都不做**
