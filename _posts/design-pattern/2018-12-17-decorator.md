---
layout: second_template
title: 装饰模式
category: design_pattern
tagline: "Supporting tagline"
tags : [design_pattern]
permalink: decorator
---

装饰模式：

动态地给一个对象添加一些额外的职责，就增加功能来说，装饰模式比生成子类更加灵活

装饰类利用SetComponent来对对象进行包装，这样每个装饰对象的实现就和如何使用这个对象分离开了，每个装饰对象只关心自己的功能，
不需要关心如何被添加到对象链当中

装饰模式的优点：

简化原有的类，核心职责和非核心职责区分开来

### UML类图
--------------------------------------------------

![Alt text][id]

[id]: assets/themes/my_blog/img/decorator.jpg

### 代码示例
--------------------------------------------------

按照UML类图，代码如下：

	#include "stdafx.h"
	#include <iostream>
	using namespace std;

	class Component
	{
	public:
		virtual ~Component()
		{

		}
		virtual void Operation()
		{
			cout << "Component::Operation" << endl;
		}
	};

	class ConcreteComponent : public Component
	{
	public:
		virtual ~ConcreteComponent()
		{

		}
		virtual void Operation()
		{
			cout << "ConcreteComponent::Operation" << endl;
		}
	};

	class Decorator : public Component
	{
	public:
		Decorator()
		{
			m_pComponent = nullptr;
		}
		virtual ~Decorator()
		{

		}
		void SetComponent(Component* pComponent)
		{
			m_pComponent = pComponent;
		}
		virtual void Operation()
		{
			if (nullptr == m_pComponent)
			{
				return;
			}
			m_pComponent->Operation();
			cout << "Decorator::Operation" << endl;
		}

	protected:
		Component* m_pComponent;
	};

	class ConcreteDecoratorA : public Decorator
	{
	public:
		virtual void Operation()
		{
			Decorator::Operation();
			cout << "ConcreteDecoratorA::Operation" << endl;
		}
	};

	class ConcreteDecoratorB : public Decorator
	{
	public:
		virtual void Operation()
		{
			Decorator::Operation();
			cout << "ConcreteDecoratorB::Operation" << endl;
		}
	};

	int main()
	{
		{
			ConcreteComponent objConcreteComponent;

			ConcreteDecoratorA objConcreteDecoratorA;
			objConcreteDecoratorA.SetComponent(&objConcreteComponent);

			ConcreteDecoratorB objConcreteDecoratorB;
			objConcreteDecoratorB.SetComponent(&objConcreteDecoratorA);
			objConcreteDecoratorB.Operation();
		}
		system("pause");
		return 0;
	}

输出：

	ConcreteComponent::Operation
	Decorator::Operation
	ConcreteDecoratorA::Operation
	Decorator::Operation
	ConcreteDecoratorB::Operation