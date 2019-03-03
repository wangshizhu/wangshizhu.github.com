---
layout: second_template
title: enum和enum-class
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: enum&enum-class
---

***
* #### enum和enum-class ####

	enum官方称作无作用域的枚举

	enum class 是有作用域的枚举类

	例如：

		{
			enum EColor
			{
				black,
				white,
				red
			};
			int red = 0; // 错误的
		}
	
	一般而言，在花括号里面声明的变量名会限制在括号外的可见性。但无作用域的枚举enum中的枚举元素并不成立，上面例子中black、white、red已经污染了整个代码块，所以当定义`int red = 0;`是错误的，**有作用域的enum类则不会出现这样的情况**

***
* #### 类型转换 ####

	无作用域的 enum 会将枚举元素隐式的转换为整数类型（从整数出发，还可以转换为浮点类型）我们可能写过这样的代码：

		int nType = 0;
		if(EColor::black == nType)
		{
			...
		}

	**有作用域枚举类不存在从枚举元素到其他类型的隐式转换，但是可以通过cast显式转换，例如：`static_cast<int>(EColor::black)`**

***
* #### 提前声明 ####

	有定义域的enum可以被提前声明的，即可以不指定枚举元素而进行声明：

		enum class EColor;

	而无定义域的enum不可以提前声明，但可以通过这种方式达到提前声明的效果：

		enum EColor : std::int16_t;

	**一切枚举型别在C++里都会由编译器来选择一个整数型别作为底层型别，在枚举型别被使用以前编译器需要知道其底层型别**，正因为这一原因无定义域的enum的提前声明并指定类型才被允许

	对于上面的无定义域enum EColor编译器会选择char作为底型层别，因为仅仅有三个值需要表达。但对于枚举型别取值范围大的情况，例如：

		enum Status
		{
			success = 0,
			failed = 1,
			executing = 0xFFFFFFFF
		}

	编译器对于Status的底层型别通常选择足够表示枚举量取值的最小底层型别，C++11的有定义域enum之所以可以提前声明是因为底层型别是已知的，默认的有定义域的枚举型别底层型别是int，当然也可以像上面一样指定型别`enum class Status : std::uint32_t`

***
* #### 善于利用优势 ####

	实际开发中经常根据实际需求利用语言天然的优势，对于无定义域的enum型别也并无任何优势，例如访问C++11中的元组std::tuple中的元素时，可以利用无定义域enum的隐式转换优势，例如：

		// 名字、邮件地址、年龄
		using UserInfo = std::tuple<std::string,std::string,std::size_t>;
		UserInfo ui;
		// 直接访问
		auto name = std::get<0>(ui);

	对于上面的代码示例直接通过字面常量0访问名字使代码的可读性变差，修改一下：

		enum UserInfoField
		{
			uiName,
			uiEmailAddr,
			uiAge
		};
		auto name = std::get<uiName>(ui);

	这便是利用了无定义域enum隐式转换，转换结果就是std::get要求的型别，而采用有定义域的enum就不得不多写代码

		enum class UserInfoField
		{
			uiName,
			uiEmailAddr,
			uiAge
		};
		auto name = std::get<static_cast<std::size_t>(UserInfoField::uiName)>(ui)

	也可以写一个模板函数获取枚举底层型别从而省去了cast操作，同时传入get函数的模板形参必须在编译期就计算出来：

		template<typename E>
		constexpr typename std::underlying_type<E>::type ToUType(E enumerator) noexcept
		{
			return static_cast<typename std::underlying_type<E>::type>(enumerator);
		}

	关于上面模板函数中的constexpr和noexcept我们放在以后文章说明，对于上面模板函数是C++11的写法，C++14的写法：

		template<typename E>
		constexpr std::underlying_type_t<E> ToUType(E enumerator) noexcept
		{
			return static_cast<std::underlying_type_t<E>>(enumerator);
		}

	或者：

		template<typename E>
		constexpr auto ToUType(E enumerator) noexcept
		{
			return static_cast<std::underlying_type_t<E>>(enumerator);
		}

	最后我们可以这样获取元组内的值`auto name = std::get<ToUType(UserInfoField::uiName)>(ui)`

	上述是我们选择有定义域enum所需要写的代码，和无定义域enum相比我们是要做额外的工作，总之各有各的优点

***
* #### 最后 ####

	C++98风格的enum是没有作用域的enum

	有作用域的枚举体的枚举元素仅仅对枚举体内部可见。只能通过类型转换（cast）转换为其他类型

	有作用域和没有作用域的enum都支持指定潜在类型。有作用域的enum的默认潜在类型是int。没有作用域的enum没有默认的潜在类型

	有作用域的enum总是可以前置声明的。没有作用域的enum只有当指定潜在类型时才可以前置声明




