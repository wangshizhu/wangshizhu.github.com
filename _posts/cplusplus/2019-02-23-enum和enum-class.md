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

	而无定义域的enum不可以提前声明，但可以通过这种方式达到提钱声明的效果：

		enum EColor : std::int16_t;




