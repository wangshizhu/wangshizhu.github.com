---
layout: second_template
title: 工厂方法模式
category: design_pattern
tagline: "Supporting tagline"
tags : [design_pattern]
permalink: factory-method
---

* 测试环境
	
	vs2015

	C++11	

	win10 64位

* 解释
	
	以下两种示例对于调用端来讲都需要知道具体的实例类型，例如需要知道哪个Creator实例，或者传递Type of Product Instance

* UML类图

	![Alt text][id]

	[id]: assets/themes/my_blog/img/factory_method.jpg

* 代码示例

	按照UML类图，代码如下：

		#include "stdafx.h"
		#include <iostream>
		using namespace std;

		class Product
		{
		public:
			virtual ~Product()
			{
			}
			virtual void Operation()
			{
				cout << "Product::Operation" << endl;
			}
		};

		class ConcreteProductA : public Product
		{
		public:
			virtual ~ConcreteProductA()
			{
			}
			virtual void Operation()
			{
				cout << "ConcreteProductA::Operation" << endl;
			}
		};

		class ConcreteProductB : public Product
		{
		public:
			virtual ~ConcreteProductB()
			{
			}
			virtual void Operation()
			{
				cout << "ConcreteProductB::Operation" << endl;
			}
		};

		class Creator
		{
		public:
			virtual Product* CreateConcreteProduct()
			{
				cout << "Creator::CreateConcreteProduct" << endl;
				return nullptr;
			}
		};

		class ConcreteCreatorA : public Creator
		{
		public:
			virtual Product* CreateConcreteProduct()
			{
				return new ConcreteProductA();
			}
		};

		class ConcreteCreatorB : public Creator
		{
		public:
			virtual Product* CreateConcreteProduct()
			{
				return new ConcreteProductB();
			}
		};

		int main()
		{
			{
				ConcreteCreatorA objCCA;
				Product* pProduct = objCCA.CreateConcreteProduct();
				if (nullptr != pProduct)
				{
					pProduct->Operation();

					delete pProduct;
					pProduct = nullptr;
				}
			}

			system("pause");
		    return 0;
		}

	也可以这样写：

		Product* CreateConcreteProduct(int nType)
		{
			switch (nType)
			{
			case 1:
			{
				return new ConcreteProductA();
			}
			case 2:
			{
				return new ConcreteProductB();
			}
			default:
				break;
			}
			return nullptr;
		}

		{
			Product* pProduct = CreateConcreteProduct(2);
			if (nullptr != pProduct)
			{
				pProduct->Operation();

				delete pProduct;
				pProduct = nullptr;
			}
		}