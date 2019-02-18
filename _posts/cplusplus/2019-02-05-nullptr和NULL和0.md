---
layout: second_template
title: 优先使用nullptr而不是NULL和0
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: nullptr&NULL&0
---
[decltype_page]:/decltype

***
* #### nullptr和NULL和0 ####
	
	在看其他开源项目或者自己编码过程中都遇见过对指针赋值为NULL、0、nullptr的情况，先通过一个例子看看他们的区别：

		void Fun(int nParam)
		{
			cout << "Fun(int nParam)" << endl;
		}

		void Fun(bool bParam)
		{
			cout << "Fun(bool bParam)" << endl;
		}

		void Fun(void* pParam)
		{
			cout << "Fun(void* pParam)" << endl;
		}

		{
			Fun(0);
			Fun(NULL);
			Fun(nullptr);
		}

	上述的重载函数和调用，输出：

		Fun(int nParam)
		Fun(int nParam)
		Fun(void* pParam)

	对于调用`Fun(0);`参数0的类型毋庸置疑的是int，当对一个指针赋值为0，编译器勉强将 0 解释为空指针

	而对于调用`Fun(NULL);`参数NULL对于调用者很明显的意图是要调用函数`Fun(void* pParam)`，但是从输出来看调用的却是函数`void Fun(int nParam)`。这就带来了调用非预期的行为。NULL的定义标准允许赋予NULL一个除了int以外的整数类型

	从上面两处调用可以看出0和NULL都不具备指针类型，通过上面对0和NULL的分析也正是我们在使用C++98时不被允许重载指针和整数类型的原因，在使用C++11以上版本时我们可以去掉这个开发规范，但是前提是必须避免使用0和NULL尽量使用nullptr

	**nullptr的优势是它不再是一个整数类型。诚实的讲，它也不是一个指针类型，但是你可以把它想象成一个可以指向任意类型的指针。 nullptr 的类型实际上
是std::nullptr_t，std::nullptr_t定义为nullptr的类型，这是一个完美的循环定义。 std::nullptr_t可以隐式的转换为所有的原始的指针类型，这使得 nullptr表现的像可以指向任意类型的指针**

	对于上面调用`Fun(nullptr);`可以避免上述调用`Fun(NULL);`所产生的调用歧义行为，而且也使得代码的可读性更高。关于nullptr的优势在模板函数上显得更明显，例如下面的例子：

		int Fun1(CPerson* pPerson)
		{
			return  0;
		}

		template<class FunType,class ParamType>
		auto TestNullptrTemplate(FunType func,ParamType ptr)->decltype(func(ptr))
		{
			return func(ptr);
		}

		{
			Fun1(0);
			Fun1(NULL);
			/*TestNullptrTemplate(Fun1, 0);
			TestNullptrTemplate(Fun1, NULL);*/
			TestNullptrTemplate(Fun1, nullptr);
		}

	对于上面的两处调用`Fun1(0);Fun1(NULL);`参数0和NULL都会被当做空指针传给函数Fun1，因此可以编译通过，而通过模板调用函数Fun1时，这样的传参`TestNullptrTemplate(Fun1, 0);TestNullptrTemplate(Fun1, NULL);`是不能通过编译的，0和NULL都被推导成整型，这和函数Fun1的参数类型`CPerson*`不符，而这样的调用`TestNullptrTemplate(Fun1, nullptr);`是没有问题的，当 nullptr 传递给TestNullptrTemplate ，ptr的类型被推导
为std::nullptr_t。当ptr被传递给Fun1，有一个由 std::nullptr_t 到 Widget* 的隐形转
换，因为 std::nullptr_t 可以隐式转换为任何类型的指针

	上面的例子中设计到了尾随返回类型`->decltype(func(ptr))`，关于尾随返回类型技术的说明可以参考这篇[文章][decltype_page]

	通过上面的两个例子分析得出结论：当我们需要用到空指针时，使用nullptr而不是0或者NULL


