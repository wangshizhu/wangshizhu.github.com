---
layout: second_template
title: 抽象工厂模式
category: design_pattern
tagline: "Supporting tagline"
tags : [design_pattern]
permalink: abstract_factory
---

* 测试环境
	
	vs2015

	C++11	

	win10 64位

* 解释

	提供一个创建一系列相关或者相互依赖对象的接口，而无需指定它们具体的类

* UML类图

	![Alt text][id]

	[id]: assets/themes/my_blog/img/abstract_factory.jpg

* 代码示例

	按照UML类图，代码如下：

		class AbstractProductA;
		class AbstractProductB;

		class AbstractFactory
		{
		public:
			virtual ~AbstractFactory()
			{
			}
			virtual AbstractProductA* CreateProductA()
			{
				return nullptr;
			}
			virtual AbstractProductB* CreateProductB()
			{
				return nullptr;
			}
		};

		class ConcreteFactory1 : public AbstractFactory
		{
		public:
			AbstractProductA* CreateProductA();
			AbstractProductB* CreateProductB();
		};

		class ConcreteFactory2 : public AbstractFactory
		{
		public:
			AbstractProductA* CreateProductA();
			AbstractProductB* CreateProductB();
		};

		class AbstractProductA
		{
		public:
			virtual ~AbstractProductA()
			{
			}
			virtual void PrintInfo()
			{
				cout << "AbstractProductA::PrintInfo" << endl;
			}
		};
		class AbstractProductB
		{
		public:
			virtual ~AbstractProductB()
			{
			}
			virtual void PrintInfo()
			{
				cout << "AbstractProductB::PrintInfo" << endl;
			}
		};

		class ConcreteProductA1 : public AbstractProductA
		{
		public:
			virtual void PrintInfo()
			{
				cout << "ConcreteProductA1::PrintInfo" << endl;
			}
		};

		class ConcreteProductB1 : public AbstractProductB
		{
		public:
			virtual void PrintInfo()
			{
				cout << "ConcreteProductB1::PrintInfo" << endl;
			}
		};

		class ConcreteProductA2 : public AbstractProductA
		{
		public:
			virtual void PrintInfo()
			{
				cout << "ConcreteProductA2::PrintInfo" << endl;
			}
		};

		class ConcreteProductB2 : public AbstractProductB
		{
		public:
			virtual void PrintInfo()
			{
				cout << "ConcreteProductB2::PrintInfo" << endl;
			}
		};

		AbstractProductA* ConcreteFactory1::CreateProductA()
		{
			return new ConcreteProductA1;
		}
		AbstractProductB* ConcreteFactory1::CreateProductB()
		{
			return new ConcreteProductB1;
		}

		AbstractProductA* ConcreteFactory2::CreateProductA()
		{
			return new ConcreteProductA2;
		}
		AbstractProductB* ConcreteFactory2::CreateProductB()
		{
			return new ConcreteProductB2;
		}

		class Client
		{
		public:
			~Client()
			{
				if (nullptr != m_pAbstractProductA)
				{
					delete m_pAbstractProductA;
					m_pAbstractProductA = nullptr;
				}
				if (nullptr != m_pAbstractProductB)
				{
					delete m_pAbstractProductB;
					m_pAbstractProductB = nullptr;
				}
			}
			Client(AbstractFactory* pAbstractFactory)
			{
				m_pAbstractProductA = pAbstractFactory->CreateProductA();
				m_pAbstractProductB = pAbstractFactory->CreateProductB();
			}
			void PrintInfo()
			{
				if (nullptr != m_pAbstractProductA)
				{
					m_pAbstractProductA->PrintInfo();
				}
				if (nullptr != m_pAbstractProductB)
				{
					m_pAbstractProductB->PrintInfo();
				}
			}

		private:
			AbstractProductA* m_pAbstractProductA;
			AbstractProductB* m_pAbstractProductB;
		};

	输出：

		ConcreteProductA1::PrintInfo
		ConcreteProductB1::PrintInfo

	通过UML类图或者代码我们发现，具体的工厂创建具有特定实现的产品对象，也就是为创建不同的产品对象客户端应使用不同的具体工厂

	这个模式最大的好处是很容易交换产品系列，只需要改动Client的创建具体工厂就可以实现，同时通过代码发现由于Client持有产品基类指针，只是通过抽象接口操作具体的产品实例

	它的应用场景我们可以想到服务器与数据库交互的过程，数据库可以是SQLServer、MySQL等，对于上层业务我只需要知道我选择了哪种数据库