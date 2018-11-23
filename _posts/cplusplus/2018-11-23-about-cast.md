---
layout: second_template
title: static_cast和dynamic_cast
category : C++
tagline: "Supporting tagline"
tags : [C++]
permalink: cast
---

* 测试环境
	
	vs2015

	C++11	

* static_cast和dynamic_cast区别

	相同点：
	
		在类的继承体系中均支持上行转换和下行转换 

	不同点：
	
		static_cast不支持运行时类型检查，支持编译时类型检查
		
		dynamic_cast支持运行时类型检查，所以，static_cast 转换安全性不如 dynamic_cast 转换
		
		通常使用 static_cast 转换数值数据类型，例如将枚举型转换为整型或将整型转换为浮点型

		dynamic_cast 只适用于指针或引用

* 代码示例
	
	`
	class Vehicle
	{
	public:
		Vehicle()
		{
			m_nMaxSpeed = 10;
		}
		virtual ~Vehicle()
		{
		}
	public:
		int m_nMaxSpeed;
	};
	class Car : public Vehicle
	{
	public:
		Car()
		{
			m_nWheels = 4;
			m_nMaxSpeed = 100;
		}
		void DrivedFun()
		{
			cout << "DrivedFun" << endl;
		}
	public:
		int m_nWheels;
	};
	class Texi : public Car
	{
	public:
		Texi()
		{
		}
	};
	class Lorry : public Vehicle
	{
	public:
		Lorry()
		{
			m_nMaxCapacity = 2000;
		}
	public:
		int m_nMaxCapacity;
	};
	void CastTest(Vehicle *pVehicle)
	{
		Car *pCar = static_cast<Car*>(pVehicle);
		Lorry *pLorry = dynamic_cast<Lorry*>(pVehicle);
		if (nullptr == pCar)
		{
			cout << "pCar is null" << endl;
		}
		else
		{
			cout << "MaxSpeed of pCar is " << pCar->m_nMaxSpeed << endl;
			cout << "Wheel of pCar is " << pCar->m_nWheels << endl;
		}
		if (nullptr == pLorry)
		{
			cout << "pLorry is null" << endl;
		}
		else
		{
			cout << "pLorry is " << pLorry->m_nMaxSpeed << endl;
		}
	}
	int main()
	{
		{
			Vehicle *pVehicle = new Vehicle;
			CastTest(pVehicle);
		}
		cout << endl;
		{
			Car *pCar = new Car;
			CastTest(pCar);
		}
		cout << endl;
		{
			Lorry *pLorry = new Lorry;
			CastTest(pLorry);
		}
		system("pause");
		return 0;
	}
	`

	输出:

	1. 当我们调用CastTest函数时传入的是实实在在的基类指针

		`
		MaxSpeed of pCar is 10
		Wheel of pCar is -33686019
		pLorry is null
		`
	2. 当我们调用CastTest函数时传入的是子类Car的指针

		`
		MaxSpeed of pCar is 100
		Wheel of pCar is 4
		pLorry is null
		`
	3. 当我们调用CastTest函数时传入的是子类Lorry的指针

		`
		MaxSpeed of pCar is 10
		Wheel of pCar is 2000
		pLorry is 10
		`

	此示例演示了下行转换，当下行转换时子类有不在基类内的成员函数和成员变量

	static_cast 

	不执行运行时类型检查，虽然static_cast下行转换时**指针不为空**

	上面第1、3种调用方式：

	意味着我们通过static_cast进行下行转换时，通过指针是否为NULL是无效的

	当通过子类指针访问子类成员变量时会引起未定义的行为，这是非常危险的

	上面第2种调用方式：

	外部真真实实的传入子类指针时不会产生未定义的行为

	dynamic_cast

	执行运行时类型检查

	我们可以通过指针是否为NULL判断转换是否有效而做相关逻辑处理

	所以当进行下行转换时dynamic_cast是安全的

	***一个有意思的问题：***

	当我们按照上面第3种调用方式时

	会发现Car::m_nWheels的值 **=** Lorry::m_nMaxCapacity的值

	如果我们在Lorry::m_nMaxCapacity的声明处**上一行或者下一行**
	增加一个整型成员变量会发生什么，
	这里涉及到C++对象模型的知识，在接下来的章节会介绍

* static_cast

	任何表达式都可以通过 static_cast 运算符显式转换为 void 类型

	目标 void 类型可以选择性地包含 const、volatile

	static_cast 运算符无法转换掉 const、volatile

	static_cast 运算符将 null 指针值转换为目标类型的 null 指针值

	static_cast 运算符可以将整数值显式转换为枚举类型。 如果整型值不在枚举值的范围内，生成的枚举值是不确定的

	static_cast 可用于将 int 转换为 char。 但是，得到的 char 可能没有足够的位来保存整个 int 值 ,
	同样，这需要程序员来验证 static_cast 转换的结果是否安全

	static_cast 多用于类层次结构中的上行转换

* dynamic_cast
	
	`
	dynamic_cast<type_id>(expression)
	`

	type-id 必须是一个**指针**或**引用**到以前已定义的类类型的引用或**指向 void 的指针**。 
	如果 type-id 是指针，则expression 的类型必须是指针，如果 type-id 是引用，则为左值。

	上行转换:

	如果 type-id 是指向 expression的明确的可访问的直接或间接基类的指针，
	则结果是指向 type-id 类型的唯一子对象的指针。

	**向上转换是一种隐式转换**。

	如果 type-id 为 void*，则做运行时进行检查确定 expression的实际类型。 
	结果是指向 by expression 的完整的对象的指针。

	如果 type-id 不是 void*，则做运行时进行检查以确定是否由 expression 指向的对象可以转换为由 type-id指向的类型。

	`
	void DynamicUpCast(Texi* pTexi)
	{
		Car* pCar = dynamic_cast<Car*>(pTexi);
		if (nullptr == pCar)
		{
			cout << "cast to pCar faild" << endl;
		}
		else
		{
			cout << "cast to pCar success" << endl;
		}
		Vehicle* pVehicle = dynamic_cast<Vehicle*>(pTexi);
		if (nullptr == pVehicle)
		{
			cout << "cast to pVehicle faild" << endl;
		}
		else
		{
			cout << "cast to pVehicle success" << endl;
		}
	}
	{
		Texi* pTexi = new Texi;
		DynamicUpCast(pTexi);
	}
	`

	输出：

	`
	cast to pCar success
	cast to pVehicle success
	`

	在多继承类层次结构中，子类转换到基类（跨父类时）会发生转换不明确的错误，例如一下示例中Moto转换到Vehicle时，
	当使用dynamic_cast进行上行转换时，虽然能编译通过，但转换后指针为空；
	当使用static_cast进行上行转换时，编译不通过，发生转换不明确的错误。

	`
	class Bicycle : public Vehicle
	{
	public:
		Bicycle()
		{
		}
	};
	class Electrombile : public Vehicle
	{
	public:
		Electrombile()
		{
		}
	};
	class Moto : public Electrombile, public Bicycle
	{
	public:
		Moto()
		{
		}
	};
	{
		Moto* pMoto = new Moto;
		//Vehicle* pVehicle = static_cast<Vehicle*>(pMoto);
		Vehicle* pVehicle = dynamic_cast<Vehicle*>(pMoto);
		if (nullptr == pVehicle)
		{
			cout <<"moto-vehicle faild"<<endl;
		}
	}
	`

	多继承的正确转换：

	`
	{
		Moto* pMoto = new Moto;
		Bicycle* pBicycle = static_cast<Bicycle*>(pMoto);
		Vehicle* pVehicle = static_cast<Vehicle*>(pBicycle);
	}
	`

	**dynamic_cast用于相互转换**，只要被转换对象是相互转换对象的子类，例如下面示例，
	Moto是Bicycle、Electrombile的子类，我们尝试这样的转换，Moto -- Bicycle -- Electrombile

	`
	{
		Moto* pMoto = new Moto;
		Bicycle* pBicycle = static_cast<Bicycle*>(pMoto);
		Electrombile* pElectrombile = dynamic_cast<Electrombile*>(pBicycle);
		if (nullptr == pElectrombile)
		{
			cout << "cross cast faild" << endl;
		}
		else
		{
			cout << "cross cast success" << endl;
		}
	}
	`
	输出:

	`
	cross cast success
	`

	下行转换：

	如果 expression 类型是 type-id类型的基类，则做运行时检查来看是否 expression 确实指向 type-id类型的完整对象。 
	如果为 true，则结果是指向 type-id类型的完整对象的指针，如果为false,则转换的指针为NULL。正如我们上面的例子所示。

	当下行转换为引用时，如果失败会抛出一个继承自exception的bad_cast异常类。

	`
	void DynamicDownCastRef(Vehicle& objVehicle)
	{
		try
		{
			Texi& objTexi = dynamic_cast<Texi&>(objVehicle);
			cout << "down-cast to objTexi as ref success" << endl;
		}
		catch (std::bad_cast objBadCastException)
		{
			cout << "down-cast to objTexi as ref faild" << endl;
		}
	}
	{
		Vehicle objVehicle;
		DynamicDownCastRef(objVehicle);
	}
	`

	输出：

	`
	down-cast to objTexi as ref faild
	`
