---
layout: second_template
title: 智能指针之shared_ptr
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: shared_ptr
---
[Person_Define]: /unique_ptr


一个通过	std::shared_ptr	访问的对象被指向它的指针通过共享所有权(shared ownership)方式来管理。没有一个特定 的	std::shared_ptr	拥有这个对象。
相反，这些指向同一个对象的	std::shared_ptr	相互协作 来确保该对象在不需要的时候被析构。
当最后一个	std::shared_ptr	不再指向该对象时(例 如，因为	std::shared_ptr	被销毁或者指向了其他对象)，std::shared_ptr	会在此之前摧毁这个对象

一个	std::shared_ptr	可以通过查询资源的引用计数(reference	count)来确定它是不是最后一 个指向该资源的指针，
引用计数是一个伴随在资源旁的一个值，它记录着有多少 个	std::shared_ptr	指向了该资源。std::shared_ptr	的构造函数会自动递增这个计数，
析构 函数会自动递减这个计数，而拷贝构造函数可能两者都做(比如，赋值操作	sp1=sp2	,sp1和sp2 都是std::shared_ptr类型，
它们指向了不同的对象，赋值操作使得sp1指向了原来sp2指向的 对象。赋值带来的连锁效应使得原来sp1指向的对象的引用计数减1，原来sp2指向的对象的引 用计数加1.)
如果	std::shared_ptr	在执行减1操作后发现引用计数变成了0，这就说明了已经没 有其他的	std::shared_ptr	在指向这个资源了，所以	std::shared_ptr	直接析构了它指向的空间

关于以下[类Person的定义][Person_Define]

### 引用计数的存在对性能产生的影响
--------------------------------------------------

- std::shared_ptrs是原生指针的两倍大小，因为它们内部除了包含了一个指向资源的原 生指针之外，同时还包含了指向资源的引用计数

- 引用计数的内存必须被动态分配
	
	概念上来说，引用计数会伴随着被指向的对象，但是被 指向的对象对此一无所知。因此，他们没有为引用计数准备存储空间，
	用	std::make_shared来创建	std::shared_ptr	的时候可以避免动态分配的开销，但是有些情况下std::make_shared也是不能被使用的。
	不管如何，引用计数都是存储为动态分配的数据

- 引用计数的递增或者递减必须是原子的

	因为在多线程环境下，会同时存在多个写者和 读者。
	例如，在一个线程中，一个	std::shared_ptr	指向的资源即将被析构(因此递减它 所指向资源的引用计数)，
	同时，在另外一个线程中，一个	std::shared_ptr	指向了同一 个对象，它此时正进行拷贝操作(因此要递增同一个引用计数)。原子操作通常要比非原子 操作执行的慢，
	所以尽管引用计数通常只有一个word大小，但是你可假设对它的读写相 对来说比较耗时

从另外一个std::shared_ptr move构造(Move_constructing)一个	std::shared_ptr	会使得源	std::shared_ptr	指向为null,
这就意味着新的std::shared_ptr取代了老的	std::shared_ptr	来指向原来的资源，所以就不需要再修改引 用计数了。

Move构造	std::shared_ptr	要比拷贝构造	std::shared_ptr	快：

copy需要修改引用 计数，然而Move构造不需要。对于赋值构造也是一样的。最后得出结论，move构造要比拷贝构 造快，move赋值要比copy赋值快

### 自定义deleter
--------------------------------------------------

像std::unique_ptr那样,std::shared_ptr也把delete作为它默认的资源析构机制。 但是它也支持自定义的deleter。然后，它支持这种机制的方式不同于	std::unique_ptr	。
**对于std::unique_ptr	,自定义的deleter是智能指针类型的一部分**，而对于std::shared_ptr,情况可就不一样了

	{
		auto TestDeleterForShare1 = [](Person* pPerson)
		{
			if (nullptr == pPerson)
			{
				return;
			}
			delete pPerson;
			pPerson = nullptr;
		};
	
		auto TestDeleterForShare2 = [](Person* pPerson)
		{
			if (nullptr == pPerson)
			{
				return;
			}
			delete pPerson;
			pPerson = nullptr;
		};
		std::shared_ptr<Person> sharePerson1(new Person,TestDeleterForShare1);
		std::shared_ptr<Person> sharePerson2(new Person, TestDeleterForShare2);
		std::vector<std::shared_ptr<Person>>	vpw{ sharePerson1,	sharePerson2 };
	
	}

虽然上述代码的两个deleter`TestDeleterForShare1``TestDeleterForShare2`类型不相同，而两个std::shared_ptr<Person>类型相同，所以可以放进同一个类型的容器中，
它们之间可以相互赋值，也都可以作为一个参数类型为std::shared_ptr<Person>	类型的函数的参数。

**所有的这些特性，具有不同类型的自定义deleter的std::unique_ptr全都办不到，因为自定义的deleter类型会影响到std::unique_ptr的类型**

### 当自定义deleter时share_ptr如何保证了自己的大小
--------------------------------------------------

与std::unique_ptr不同的其他的一点是，为std::shared_ptr	指定自定义的deleter不会改变std::shared_ptr	的大小。
不管deleter如何，一个std::shared_ptr	始终是两个pointer的大小。这可是个好消息，但是会让我们一头雾水。自定义的deleter可以是函数对象，
函数对象 可以包含任意数量的data，这就意味着它可以是任意大小。涉及到任意大小的自定义deleter 的	std::shared_ptr	如何保证它不使用额外的内存呢

使用额外的空间来完成上述目标。然而，这些额外的空间不属于	std::shared_ptr	的一部分。额外的空间被分配在堆上，
或者在	std::shared_ptr	的创建者使用了自定义的allocator之后，位于该allocator管理的内存中。一个std::shared_ptr	对象包含了一个指针，
指向了它所指对象的引用计数。但是却有一些误导性，因为引用计数是一个叫做控制块(control	block)的很大的数据结构。
**每一个由std::shared_ptr管理的对象都对应了一个控制块**。该控制块不仅包含了引用计数，还包含 了一份自定义deleter的拷贝(在指定好的情况下)。
如果指定了一个自定义的allocator,也会被包 含在其中。控制块也可能包含其他的额外数据

一个对象的控制块被第一个创建指向它的std::shared_ptr	的函数来设立。一般情况下，函数在创建一个	std::shared_ptr	时，
它不可能知道这时是否有其他的std::shared_ptr已经指向了这个对象，所以在创建控制块时，它会遵循以下规则：

- std::make_shared总是会创建一个控制块
	
	它制造了一个新的可以指向的对象，所以可以确定这个新的对象在std::make_shared	被调用时肯定没有相关的控制块（这也是我们为什么推荐使用std::make_shared）

- 当一个	std::shared_ptr	被一个独占性的指针(例如，一个std::unique_ptr或者std::auto_ptr)构建时，控制块被相应的被创建
	
	**独占性的指针并不使用控制块，所以被指向的对象此时还没有控制块相关联**
	
	构造的一个过程是，由std::shared_ptr来 接管了被指向对象的所有权，所以原来的独占性指针被设置为null

- 当一个	std::shared_ptr被一个原生指针构造时，它也会创建一个控制块

	如果你想要基于一个已经有控制块的对象来创建一个	std::shared_ptr	，
	你可能传递了一个std::shared_ptr或者std::weak_ptr作为	std::shared_ptr	的构造参数，而不是传递 了一个原生指针。	
	std::shared_ptr构造函数接受std::shared_ptr或者std::weak_ptr时，不会创建新的控制块，
	因为它们(指构造函数)会依赖传递给它们的智能指针是否已经指向了带有控制块的对象的情况

### 使用注意事项
--------------------------------------------------

当使用了一个原生的指针构造多个	std::shared_ptr时，这些规则的存在会使得被指向的对象 包含多个控制块，带来许多负面的未定义行为。
多个控制块意味着多个引用计数，多个引用 计数意味着对象会被摧毁多次(每次引用计数一次)

错误的示范：

	{
		Person *pPerson = new Person;
		std::shared_ptr<Person> sharePerson1(pPerson);
		std::shared_ptr<Person> sharePerson2(pPerson);
	}

1. 避免给std::shared_ptr构造函数传递原生指针。 通常的取代做法是使用std::make_shared。但是在上面的例子中，我们使用了自 定义的deleter,这对于std::make_shared是不可能的

2. 如果你必须要给std::shared_ptr构 造函数传递一个原生指针，那么请直接传递new语句

使用this指针时，有时也会产生因为使用原生指针作为std::shared_ptr构造参数而导致的产生多个控制块的问题，给一个	std::shared_ptr	传递this就相当于传递了一个原生指针。
如果你想要使得被std::shared_ptr管理的类安全的以this指针为参数创建一个std::shared_ptr	,就必须要继承一个基类的模板std::enabled_from_this
			

### std::enable_shared_from_this
--------------------------------------------------

std::enable_shared_from_this是一个基类模板。它的类型参数永远是它要派生的子类类型，所以Person可以继承自std::enable_shared_from_this<Person>，
std::enable_shared_from_this定义了一个成员函数来创建指向当前对象的std::shared_ptr, 但是它并不重复创建控制块。这个成员函数的名字是shared_from_this，
当你实现一个成员函数，用来创建一个std::shared_ptr来指向this指针指向的对象,可以在其中使用shared_from_this

shared_from_this内部实现是，它首先寻找当前对象的控制块，然后创建一个新的	std::shared_ptr来引用那个控制块。
**这样的设计依赖一个前提，就是当前的对象必须有一个与之相关的控制块。**为了让这种情况成真，
事先必须有一个std::shared_ptr	指向了当前的对象(比如说在这个调用shared_from_this的成员函数的外面)，
如果这样的std::shared_ptr不存在(即，当前的对象没有相关的控制块)，虽然shared_from_this通常会抛出异常，产生的行为仍是未定义的。

**为了阻止用户在没有一个std::shared_ptr指向该对象之前，使用一个里面调用shared_from_this的成员函数，
继承自std::enable_shared_from_this的子类通常会把它们的构造函数声明为private,并且让它们的使用者利用返回std::shared_ptr	的工厂函数来创建对象**

### 线程安全性
--------------------------------------------------

上面提到引用计数的原子性，然而就不能断定std::shared_ptr就是线程安全的，关于std::shared_ptr的线程安全性有如下总结：

* 毋庸置疑的是，多个线程同时**读同一个std::shared_ptr对象是安全的**
	
	必须确定调用的API是不是读取API
	
* 多个线程不能同时**写同一个std::shared_ptr对象**
	
	例如：线程1调用swap成员函数、reset成员函数、拷贝构造函数、赋值构造函数其中之一，而线程2可能触发了析构，此时有可能引用计数为0而导致资源被释放，
	虽然引用计数可以保证原子性，但是对于std::shared_ptr对象内部原生指针、引用计数指针的操作（swap、赋值等）并不能保证一个指令完成，而分成了多个语句完成，
	以赋值构造函数为例，假设有线程共享shared_ptr对象g，线程1局部变量shared_ptr对象n，线程2局部变量shared_ptr对象x：
	
		// 	线程共享shared_ptr对象g
		std::shared_ptr<Foo> g(new Foo);
		
		// 线程1局部变量shared_ptr对象n
		std::shared_ptr<Foo> n(new Foo);
		
		// 线程2局部变量shared_ptr对象x
		std::shared_ptr<Foo> x;
	
	此时g、n、x如下图：
	
	![Alt text][shared_ptr_safe_1]
	
	[shared_ptr_safe_1]: assets/themes/my_blog/img/shared_ptr_safe_1.png
	
	当线程2执行x=g时，**并且只复制完内部原生指针，还没有复制引用计数指针**，如下图：
	
	![Alt text][shared_ptr_safe_2]
	
	[shared_ptr_safe_2]: assets/themes/my_blog/img/shared_ptr_safe_2.png
	
	此时切换至线程1执行g=n，**并且复制完内部原生指针和引用计数指针**，复制内部原生指针：
	
	![Alt text][shared_ptr_safe_3]
	
	[shared_ptr_safe_3]: assets/themes/my_blog/img/shared_ptr_safe_3.png
	
	复制n的引用计数指针，导致n的引用计数加 1，并且析构了g原来持有内存，因为在没有复制n的引用计数指针之前，g的引用计数为1：
	
	![Alt text][shared_ptr_safe_4]
	
	[shared_ptr_safe_4]: assets/themes/my_blog/img/shared_ptr_safe_4.png
	
	程序切换至线程2，继续未完成的操作——复制g的引用计数指针：
	
	![Alt text][shared_ptr_safe_5]
	
	[shared_ptr_safe_5]: assets/themes/my_blog/img/shared_ptr_safe_5.png
	
	**那么此时x的原生指针是空悬指针，引用计数指针和g相同，引用计数为3**
	
	线程1和线程2同时操作了同一个shared_ptr对象g，触发了多线程的race condition，
	所以**多个线程操作同一个shared_ptr对象是不安全的**
	
* 多个线程可以同时**读写不同的std::shared_ptr对象**
	
	其实有了上面的分析理解这段话就很容易了——多个线程可以同时读写不同的std::shared_ptr对象，例如：
		
		// 线程间共享的std::shared_ptr对象
		// 假设此对象在程序生命周期不会调用shared_ptr的非const成员函数
		std::shared_ptr<Test> shared = std::make_shared<Test>();
		...
		
		// 线程1
		std::shared_ptr<Test> t1(shared);
		...
		std::shared_ptr<Test> t1_tmp = std::make_shared<Test>();
		t1.swap(t1_tmp);
		
		// 线程2
		std::shared_ptr<Test> t2(shared);
		...
		std::shared_ptr<Test> t2_tmp = std::make_shared<Test>();
		t2.swap(t2_tmp);
	
	上面的例子中【线程1的t1对象】和【线程2的t2对象】属于【线程间共享变量shared】的不同对象，那么在线程1和线程2内操作t1和t2是**安全的**

### 不必担忧
--------------------------------------------------

一个控制块可能只有几个字节大小，尽管自定义的deleters和allocators可能会使得它更大。通 常控制块的实现会比你想象中的更复杂。
它利用了继承，用到虚函数(确保指向的对象能正确销毁)这就意味着使用std::shared_ptr	会因为控制块使用虚函数而导致一定的机器开销，还有引用计数的原子操纵

**通常情况下，std::shared_ptr被std::make_shared所创建，使用默认的deleter和 默认的allocator,控制块也只有大概三个字节大小。
它的分配基本上是不耗费空间的。解引用一个std::shared_ptr花费的代价不 会比解引用一个原生指针更多。
执行一个需要操纵引用计数的过程(例如拷贝构造、拷贝赋值或者析构)需要两个原子操作，但是这些操作通常只会映射到个别的机器指令，
尽管相对于普通的非原子指令他们可能更耗时，当在被std::shared_ptr管理的对象被销毁时才会调用控制块中虚函数的机制**

* 基类析构函数可以不加virtual

	假设有Base类和其子类Derive，如下：
	
		class Base
		{
		public:
			Base(){}
			~Base(){}
		};
		
		class Derive : public Base
		{
		public:
			Derive(){}
		}
	
		std::shared_ptr<Derive> derive = std::make_shared<Derive>();
		
		{
			std::shared_ptr<Base> base = derive;
			derive.reset(); //derive 的引用计数为1
		}
		
	从块里出来后base仍然能安全地管理 derive 对象的生命期，并安全完整地释放 derive，
	因为引用计数指针指向的cotrol block 记住了 derive 的实际类型
		
* 多继承
	
	假设Base类是其子类Derive的多个基类之一，按照上面的结论可以得出：可以安全的释放Derive
	
		class Derive : public Base,OtherBase
		{
		public:
			Derive(){}
		}
	
		std::shared_ptr<Derive> derive = std::make_shared<Derive>();
		
		{
			std::shared_ptr<Base> base = derive;
			derive.reset(); //derive 的引用计数为1
		}
		
	从块里出来后base仍然能安全地管理 derive 对象的生命期，并安全完整地释放 derive，
	释放的是引用计数指针指向的cotrol block里的 derive实际类型
	
**shared_ptr对象内部包含两个指针，一个是原生指针，一个是引用计数指针（严格的说是指向一个control block的指针），所以从上面的结论得出，
当资源被释放时，释放的是shared_ptr对象内引用计数所指向cotrol block里的ptr，并不是shared_ptr对象管理的原生指针**

那么这样的表达式也成立
	
	std::shared_ptr<void> v = derive;
	
并且能够安全释放

### std::shared_ptr和std::unique_ptr
--------------------------------------------------

[介绍unique_ptr的文章][Person_Define]里提到了很容易从std::unique_ptr“升级”到std::shared_ptr

如果你把一个资源的生命周期管理交给了std::shared_ptr	，后面没有办法在变化了。即使引用计数的值是1，为了让std::unique_ptr	来管理它，
你也不能重新声明资源的所有权。资源和指向它的std::shared_ptr	之间的契约不可改变

std::shared_ptr	在数组上面的应用没有std::unique_ptr强大。std::shared_ptr的API设计为指向单个的对象。没有像std::shared_ptr<T[]>这样 的用法。
经常有一些自作聪明的程序员使用std::shared_ptr<T>	来指向一个数组,指定了一个 自定义的deleter来做数组的删除操作(即delete[])。却是个坏主意， 原因有两点：
	
1. std::shared_ptr没有重载操作符[],所以如果是通过数组访问需要通过基于指针的运算来进行

2. std::shared_ptr supports	derived-to-base	pointer conversions	that make sense	for	single objects,	
but	that	open holes in the type system when applied to arrays.(For this reason, the std::unique_ptr<T[]>	API	prohibits	such conversions.)

3. 鉴于C++11标准给了比原生数组更好的选择(例如，std::array,	std::vector,std::string	),给数组来声明一个智能指针通常是不当设计的表现

### 最后
--------------------------------------------------

1. std::shared_ptr为了管理任意资源的共享式内存管理提供了自动垃圾回收的便利

2. std::shared_ptr是std::unique_ptr的两倍大，除了控制块，还有需要原子引用计数操作引起的开销

3. 资源的默认析构一般通过delete来进行，但是自定义的deleter也是支持的。deleter的类型对于std::shared_ptr	的类型不会产生影响

4. 避免从原生指针类型变量创建std::shared_ptr
