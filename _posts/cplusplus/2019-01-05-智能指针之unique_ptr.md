---
layout: second_template
title: 智能指针之unique_ptr
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: unique_ptr
---

在C++11标准中规定了四个智能指针:

std::auto_ptr

std::unique_ptr

std::shared_ptr

std::weak_ptr

它们都用来设计辅助管理动态分配对象的生命周期，即确保这些对象在正确 的时间(包括发生异常时)用正确的方式进行回收，以确保不会产生内存泄露

C++98尝试用std::auto_ptr来标准化，后来成为C++11中的std::unique_ptr的行为，为了达到目 标，move语法是不可少的，但是，C++98当时还没有move语法，
所以做了个妥协方案:利用 拷贝操作来模拟move，而如果拷贝一个std::auto_ptr会将它设置为 null，而且std::auto_ptr不能在容器中使用，
导致在实际编码中很少应用std::auto_ptr，std::unique_ptr做到了std::auto_ptr所能做到的所有事情，而且它的实现还更高效

### std::unique_ptr
--------------------------------------------------

**std::unique_ptr 具现了独占语义,一个非空的 std::unique_ptr 永远拥
有它指向的对象，move一个 std::unique_ptr 会将所有权从源指针转向目的指针(源指针指向
为null)。拷贝一个 std::unique_ptr 是不允许的，假如说真的可以允许拷贝 std::unique_ptr ,
那么将会有两个 std::unique_ptr 指向同一块资源区域，每一个都认为它自己拥有且可以摧毁那块资源，导致多次释放。因此，std::unique_ptr 是一个move-only类型。当它面临析构时，一个非空的std::unique_ptr 会摧毁它所拥有的资源。默认情况下，
std::unique_ptr 会使用delete来释放它所包裹的原生指针指向的空间**

默认情况下，析构函数会使用delete。但是，我们也可以在它的构造过程中指定特定的析构方法(custom deleters):
当资源被回收时，传入的特定的析构方法(函数对象，或者是特定的 lambda表达式)会被调用

当 std::unique_ptr 用到了自定义的deleter时，**函数指针类型**的deleter会使得 std::unique_ptr 的大小增长到一个字节到两个字节。对于deleters是函数对象的 std::unique_ptr ,大小的改变依赖于函数对象内部要存储多少状态。无状态的函数对象(如，没有captures的lambda expressions) 不会导致额外的大小开销。这就意味着当一 个自定义的deleter既可以实现为一个函数对象或者一个无捕获状态的lambda表达式时，所以lambda是第一优先选择。带有过多状态的函数对象的deleters是使得 std::unique_ptr 的大小得到显著的增加

关于std::unique_ptr的大小增加的测试代码：


	class Person
	{
	public:
		Person()
		{

		}
		Person(int nAge,string strName)
		{
			m_nAge = nAge;
			m_strName = strName;
		}
		virtual ~Person()
		{

		}

		void PrintInfo()
		{
			cout << m_strName << endl;
			cout << m_nAge << endl;
		}
	private:
		int m_nAge;
		string m_strName;
	};

	auto DeletePerson = [](Person* pPerson)
	{
		cout << "DeletePerson" << endl;
		if (nullptr == pPerson)
		{
			return;
		}
		delete pPerson;
		pPerson = nullptr;
	};

	void DeletePersonAsFunPointer(Person* pPerson)
	{
		cout << "DeletePersonAsFunPointer" << endl;
		if (nullptr == pPerson)
		{
			return;
		}
		delete pPerson;
		pPerson = nullptr;
	};

	template<typename...Ts>
	std::unique_ptr<Person, decltype(DeletePerson)> CreatePerson(int nType,Ts&&... params)
	{
		std::unique_ptr<Person, decltype(DeletePerson)> pPerson(nullptr,DeletePerson);

		cout << "CreatePerson sizeof of unique_ptr:" << sizeof(pPerson) << endl;

		pPerson.reset(new Person(std::forward<Ts>(params)...));
		return pPerson;
	};

	template<typename...Ts>
	std::unique_ptr<Person, decltype(DeletePersonAsFunPointer)* > CreatePersonWithFuntionPointer(int nType, Ts&&... params)
	{
		std::unique_ptr<Person, decltype(DeletePersonAsFunPointer)*> pPerson(nullptr, DeletePersonAsFunPointer);

		cout << "CreatePersonEx sizeof of unique_ptr:" << sizeof(pPerson) << endl;

		pPerson.reset(new Person(std::forward<Ts>(params)...));
		return pPerson;
	};

	{
		auto tmp1 = CreatePerson(1,10, "zhangsan");

		auto tmp2 = CreatePersonWithFuntionPointer(2,10, "zhangsan");
	}

输出：

	CreatePerson sizeof of unique_ptr:4
	CreatePersonEx sizeof of unique_ptr:8
	DeletePersonAsFunPointer
	DeletePerson

**std::unique_ptr 会产生两种格式，一种是独立的对象(std::unique_ptr)，另外一种是数组(std::unique_ptr<T[]> )。
因此，std::unique_ptr指向的内容从来不会产生任何歧义性。它的API是专门为了你使用的格式来设计的。例如，单对象格式中没有过索引操作符(操作符[]),数组格式则没有解引用操作符(操作符"*"和操作符"->")**

C++11使用 std::unique_ptr 来表述独占所有权。但是它的一项最引人注目的特性就是它可以轻易且有效的转化为 std::shared_ptr


1. std::unique_ptr 是一个具有开销小，速度快， move-only 特定的智能指针，使用独占拥有方式来管理资源

2. 默认情况下，释放资源由delete来完成，也可以指定自定义的析构函数来替代。但是具有丰富状态的deleters和以函数指针作为deleters增大了 std::unique_ptr的存储开销

3. 很容易将一个std::unique_ptr转化为std::shared_ptr