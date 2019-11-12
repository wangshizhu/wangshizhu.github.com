---
layout: second_template
title: interface
category: golang
tagline: "Supporting tagline"
tags : [golang]
permalink: interface
---

--------------------------------------------------
### 几种函数

* 顶级函数
	
		func TopLv(x int){}
	
* 值接收者方法

		func (obj Object) ValReceiver(){}
	
* 指针接收者方法

		func (this *Object) PointReceiver(){}

* 函数字面量
	
		var literal = func(x int) {}
		
--------------------------------------------------
### 值接收者方法和指针接收者方法的区别
	
先看下面代码：
	
	type Dog struct{
		Age int
		Nick string
	}
	
	func (obj Dog) GrowUp() int{
		obj.Age += 1
		return obj.Age
	}
	
	// func (this *Dog) GrowUp() int{
	// 	this.Age += 1
	// 	return this.Age
	// }
	
	func (this *Dog) GetNick() string {
		return this.Nick
	}
		
		
1. 当调用者以对象的方式调用值接收者方法GrowUp()时：
		
		func main() {
			d := Dog{Age:1,Nick:"wangwang"}
			fmt.Println(d)
			d.GrowUp()
			fmt.Println(d)
		}
		
	输出：
	
		{1 wangwang}
		{1 wangwang}
		
2. 当调用者以对象指针的方式调用值接收者方法GrowUp()时：
	
		func main() {
			d := &Dog{Age:1,Nick:"wangwang"}
			fmt.Println(d)
			d.GrowUp()
			fmt.Println(d)
		}
		
	输出：

		&{1 wangwang}
		&{1 wangwang}
		
3. 当调用者以对象的方式调用指针接收者方法GrowUp()时：
	
		func (this *Dog) GrowUp() int{
			this.Age += 1
			return this.Age
		}
		func main() {
			d := Dog{Age:1,Nick:"wangwang"}
			fmt.Println(d)
			d.GrowUp()
			fmt.Println(d)
		}
		
	输出：
		
		{1 wangwang}
		{2 wangwang}
		
4. 当调用者以对象指针的方式调用指针接收者方法GrowUp()时：

		func (this *Dog) GrowUp() int{
			this.Age += 1
			return this.Age
		}
		func main() {
			d := &Dog{Age:1,Nick:"wangwang"}
			fmt.Println(d)
			d.GrowUp()
			fmt.Println(d)
		}
		
	输出：
		
		&{1 wangwang}
		&{2 wangwang}

总结：

1. 不管接收者类型是对象类型还是指针类型，都可以通过对象类型或指针类型调用
2. 对于接收者类型是对象类型时，不管调用者是对象还是对象的指针那么都是这个对象的一份copy
3. 对于接收者类型是对象指针类型时，不管调用者是对象还是对象的指针那么都是对这个对象直接操作
4. 当调用者是对象指针类型而接收者类型是对象类型时，其实是对指针的解引用操作(*d).GrowUp()
5. 当调用者是对象类型而接收者类型是对象指针类型时，其实是对当前对象的引用操作(&d).GrowUp()

还有个事实：**当实现了接收者对象类型的方法，相当于自动实现了接收者是指针类型的方法；而实现了接收者是指针类型的方法，不会自动生成对应接收者是对象类型的方法**，如下代码：

	type IBehavior interface {
		GrowUp()int
	}
	
	type Dog struct{
		Age int
		Nick string
	}
	
	func (obj Dog) GrowUp() int{
		obj.Age += 1
		return obj.Age
	}
	
	// func (this *Dog) GrowUp() int{
	// 	this.Age += 1
	// 	return this.Age
	// }
	
	func main() {
		var d IBehavior = &Dog{Age:1,Nick:"wangwang"}
		fmt.Println(d)
		d.GrowUp()
		fmt.Println(d)
	}

输出：

	&{1 wangwang}
	&{1 wangwang}
	
而这种调用方式：

	func (this *Dog) GrowUp() int{
		this.Age += 1
		return this.Age
	}
	func main() {
		var d IBehavior = Dog{Age:1,Nick:"wangwang"}
		fmt.Println(d)
		d.GrowUp()
		fmt.Println(d)
	}

编译报错：
	
	.\main.go:31:6: cannot use Dog literal (type Dog) as type IBehavior in assignment:
		Dog does not implement IBehavior (GrowUp method has pointer receiver)
		
对于这样设计的一个合理解释就是：接收者是指针类型的方法，很可能在方法中会对接收者的属性进行更改操作，从而影响接收者；而对于接收者是对象类型的方法，在方法中对属性进行修改不会对接收者本身产生影响

--------------------------------------------------
### interface

上面代码其实提到了interface————`type IBehavior interface`，interface是GO的接口类型
> 接口类型是对其它类型行为的抽象和概括；因为接口类型不会和特定的实现细
节绑定在一起，通过这种抽象的方式我们可以让对象更加灵活和更具有适应能力

> 接口类型的独特之处在于它是满足隐式实现的鸭子类型
> Go语言对基础类型的类型一致性要求非常的严格，但是Go语言对于接口类型的转换则非常的灵活。对象和接口之
间的转换、接口和接口之间的转换都可能是隐式的转换

上一小节`var d IBehavior = Dog{Age:1,Nick:"wangwang"}`就发生了对象和接口之间的类型隐式转换

interface分为包含方法的接口iface、不包含任何方法的空接口eface

iface结构：

	type iface struct {
	    tab  *itab
	    data unsafe.Pointer
	}
	
	type itab struct {
	    inter  *interfacetype
	    _type  *_type
	    link   *itab
	    hash   uint32 // copy of _type.hash. Used for type switches.
	    bad    bool   // type does not implement interface
	    inhash bool   // has this itab been added to hash?
	    unused [2]byte
	    fun    [1]uintptr // variable sized
	}
	type interfacetype struct {
	    typ     _type
		// 定义了接口的包名
	    pkgpath name
		// 接口所定义的函数列表
	    mhdr    []imethod
	}
	type _type struct {
	    // 类型大小
	    size       uintptr
	    ptrdata    uintptr
	    // 类型的 hash 值
	    hash       uint32
	    // 类型的 flag，和反射相关
	    tflag      tflag
	    // 内存对齐相关
	    align      uint8
	    fieldalign uint8
	    // 类型的编号，有bool, slice, struct 等等等等
	    kind       uint8
	    alg        *typeAlg
	    // gc 相关
	    gcdata    *byte
	    str       nameOff
	    ptrToThis typeOff
	}
	

其实Go语言各种数据类型都是在 _type 字段的基础上，增加一些额外的字段来进行管理的：

	type arraytype struct {
	    typ   _type
	    elem  *_type
	    slice *_type
	    len   uintptr
	}
	
	type chantype struct {
	    typ  _type
	    elem *_type
	    dir  uintptr
	}
	
	type slicetype struct {
	    typ  _type
	    elem *_type
	}
	
	type structtype struct {
	    typ     _type
	    pkgPath name
	    fields  []structfield
	}

eface结构：

	type eface struct {
	    _type *_type
	    data  unsafe.Pointer
	}

两个结构中的data都描述了具体的值

上面提到对象和接口之间的转换、接口和接口之间的转换，那么判定一种类型是否满足某个接口时，Go 使用类型的方法集和接口所需要的方法集进行匹配，如果类型的方法集完全包含接口的方法集，则可认为该类型实现了该接口

当把实体类型赋值给接口的时候，会调用 conv 系列函数，例如空接口调用 convT2E 系列、非空接口调用 convT2I 系列。这些函数比较相似：

* 具体类型转空接口时，_type 字段直接复制源类型的 _type；调用 mallocgc 获得一块新内存，把值复制进去，data 再指向这块新内存。
* 具体类型转非空接口时，入参 tab 是编译器在编译阶段预先生成好的，新接口 tab 字段直接指向入参 tab 指向的 itab；调用 mallocgc 获得一块新内存，把值复制进去，data 再指向这块新内存。
* 而对于接口转接口，itab 调用 getitab 函数获取。只用生成一次，之后直接从 hash 表中获取

对于接口转接口convI2I原型：

	func convI2I(inter *interfacetype, i iface) (r iface) {
	    tab := i.tab
	    if tab == nil {
	        return
	    }
	    if tab.inter == inter {
	        r.tab = tab
	        r.data = i.data
	        return
	    }
	    r.tab = getitab(inter, tab._type, false)
	    r.data = i.data
	    return
	}
	
convI2I函数内getitab 函数会根据 interfacetype 和 _type 去**全局的 itab 哈希表**中查找，如果能找到，则直接返回；否则，会根据给定的 interfacetype 和 _type 新生成一个 itab，并插入到 itab 哈希表

--------------------------------------------------
### interface应用点

* 接口类型和 nil 作比较
	
	接口值的零值是指动态类型和动态值都为 nil。当仅且当这两部分的值都为 nil 的情况下，这个接口值为nil。以上面代码为例：
	
		var i IBehavior
		fmt.Println(i == nil)
		fmt.Printf("i: %T, %v\n", i, i)
		
		var d *Dog
		fmt.Println(d == nil)
	
		i = d
		fmt.Println(i == nil)
		fmt.Printf("i: %T, %v\n", i, i)
	
	输出：
	
		true
		i: <nil>, <nil>
		true
		false
		i: *main.Dog, <nil>

* 如何打印出接口的动态类型和值
		
		type DefIface struct {
		    itab, data uintptr
		}
	
		var a interface{} = nil

		var b interface{} = (*int)(nil)

		x := 5
		var c interface{} = (*int)(&x)

		ia := *(*DefIface)(unsafe.Pointer(&a))
		ib := *(*DefIface)(unsafe.Pointer(&b))
		ic := *(*DefIface)(unsafe.Pointer(&c))

		fmt.Println(ia, ib, ic)

		fmt.Println(*(*int)(unsafe.Pointer(ic.data)))
	
	输出：
	
		{0 0} {4964448 0} {4964448 824634170920}
		100
		
* 编译器自动检测类型是否实现接口
	
		type IBehavior interface {
			GrowUp()int
		}
		type Dog struct{
			Age int
			Nick string
		}
		// func (this *Dog) GrowUp() int{
		// 	this.Age += 1
		// 	return this.Age
		// }
		
		func main() {
			var _ IBehavior = (*Dog)(nil)
		}
		
	输出：
	
		.\main.go:67:6: cannot use (*Dog)(nil) (type *Dog) as type IBehavior in assignment:
			*Dog does not implement IBehavior (missing GrowUp method)

* 断言
	
	有时函数的形参是interface{}，那么有可能在函数体内要根据参数类型而做不同的逻辑处理。断言的语法：

		// 安全类型断言
		<目标类型的值>，<布尔参数> := <表达式>.( 目标类型 )  

		//非安全类型断言
		<目标类型的值> := <表达式>.( 目标类型 ) 
		
	如果使用了非安全类型断言并且与期望的类型不一致会导致panic

		var d IBehavior = Dog{Age:1,Nick:"wangwang"}
		t,ok := d.(Dog)
		if ok {
			fmt.Println("type correct",t)
		}
	
	输出：
	
		type correct {1 wangwang}
		
	断言在switch中的应用：
	
		func Function(v interface{}){
			switch t := v.(type){
			case nil:
				fmt.Printf("nil type [%p] [%T] %v\n",&t, t, t)
			case Dog:
				fmt.Printf("object dog [%p] [%T] %v\n",&t, t, t)
			case *Dog:
				fmt.Println("point dog [%p] [%T] %v\n",&t, t, t)
			default:
				fmt.Printf("%p %v\n", &t, t)
				fmt.Println("unknow type\n")
			}
		}
		func main(){
			var d IBehavior = Dog{Age:1,Nick:"wangwang"}
			Function(d)
		}
	
	输出：
	
		object dog [0xc000052440] [main.Dog] {1 wangwang}
		
--------------------------------------------------
### 实现继承、多态、重写

代码如下：

	type IAnimal interface {
		GrowUp() int
		Name() string
	}
	
	type AnimalBase struct {
		Age int
	}
	
	func (this *AnimalBase) Name() string{
		return "animal"
	}
	
	type Cat struct {
		AnimalBase
		Nick string
	}
	
	func (this *Cat) Name() string{
		return "cat"
	}
	
	func (this *Cat) GrowUp() int {
		this.Age += 1
		return this.Age
	}
	
	type Dog struct{
		AnimalBase
		Nick string
	}
	
	func (this *Dog) Name() string{
		return "dog"
	}
	
	func (this *Dog) GrowUp() int{
		this.Age += 1
		return this.Age
	}
	
	func WhatName(i IAnimal) string{
		return i.Name()
	}
	
	func GrowUp(i IAnimal) int {
		return i.GrowUp()
	}
	
	func main() {
	
		pDog := &Dog{}
		pDog.Age = 1
	
		name := WhatName(pDog)
		fmt.Println(name)
	
		pCat := &Cat{}
		pCat.Age = 1
		name = WhatName(pCat)
		fmt.Println(name)
	}
	
输出：
	
	dog
	cat
	
同时可以通过嵌入匿名接口或嵌入匿名指针对象来实现继承的做法其实是一种纯虚继承，匿名接口或匿名指针对象起到了像C++虚基类的作用，一种类型的行为描述，继承的只是接口指定的规范，真正的实现在运行的时候才被注入


	

	
