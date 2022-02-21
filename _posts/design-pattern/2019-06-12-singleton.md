---
layout: second_template
title: 单例模式
category: design_pattern
tagline: "Supporting tagline"
tags : [design_pattern]
permalink: singleton
---

[stack]:https://stackoverflow.com/questions/34457432/c11-singleton-static-variable-is-thread-safe-why
[OSDEV]:https://wiki.osdev.org/C++

单例模式：确保一个类有且仅有一个实例

### 测试环境
--------------------------------------------------
	
vs2015

win10 64位

### 代码示例
--------------------------------------------------
	
非线程安全、基于继承实现

	template <class T>
	class Singleton
	{
	public:
		Singleton()
		{
			assert(!m_pT);
			m_pT = static_cast<T*>(this);
		}
		virtual ~Singleton()
		{
			assert(m_pT);
			m_pT = nullptr;
		}
	
		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;
	
	public:
		static T* GetInstancePtr()
		{
			return m_pT;
		}
	
	private:
		static T* m_pT;
	};
	
	template <class T> T*  Singleton<T>::m_pT = nullptr;
	
	class CSingleTest : public Singleton<CSingleTest>
	{
	
	};
	
	{
		CSingleTest objSingleTest;
	}

非继承实现、手动释放资源

	template <class T>
	class CSingleton
	{
	public:
		template<typename... Args>
		static T* GetInstance(Args&&... args)
		{
			if (nullptr != m_ptrT)
			{
				return m_ptrT;
			}
			std::lock_guard<std::mutex> lock(m_mutex);
			if (nullptr != m_ptrT)
			{
				return m_ptrT;
			}
	
			m_ptrT = new T(std::forward<Args>(args)...);
	
			return m_ptrT;
		}
	
		static void ReleaseInstance()
		{
			if (nullptr == m_ptrT)
			{
				return;
			}
			delete m_ptrT;
			m_ptrT = nullptr;
		}
	
		~CSingleton() = default;
	private:
		CSingleton() {}
		CSingleton(const CSingleton&) = delete;
		CSingleton& operator=(const CSingleton&) = delete;
	private:
		static T* m_ptrT;
		static std::mutex m_mutex;
	};
	
	template<class T> T* CSingleton<T>::m_ptrT = nullptr;
	template<class T> std::mutex CSingleton<T>::m_mutex;

非继承实现

	template <class T>
	class CSingleton
	{
	public:
		template<typename... Args>
		static T* GetInstance(Args&&... args)
		{
			if (nullptr != m_ptrT)
			{
				return &(*m_ptrT);
			}
			std::lock_guard<std::mutex> lock(m_mutex);
			if (nullptr != m_ptrT)
			{
				return &(*m_ptrT);
			}
	
			m_ptrT.reset(new T(std::forward<Args>(args)...));
	
			return &(*m_ptrT);
		}
	
		~CSingleton() = default;
	private:
		CSingleton() {}
		CSingleton(const CSingleton&) = delete;
		CSingleton& operator=(const CSingleton&) = delete;
	private:
		static std::unique_ptr<T> m_ptrT;
		static std::mutex m_mutex;
	};
	
	template<class T> std::unique_ptr<T> CSingleton<T>::m_ptrT = nullptr;
	template<class T> std::mutex CSingleton<T>::m_mutex;

静态局部变量示例

	template <class T>
	class CSingleton
	{
	public:
		static T* GetInstance()
		{
			static T instance();
			return &instance;
		}
	
		~CSingleton() = default;
	private:
		CSingleton() {}
		CSingleton(const CSingleton&) = delete;
		CSingleton& operator=(const CSingleton&) = delete;
	};

测试代码示例:

	class CSingleTest
	{
	public:
		CSingleTest()
		{
			cout << "CSingleTest()" << endl;
		}
		CSingleTest(int nTmp)
		{
			cout << "CSingleTest(int nTmp)" << endl;
		}
		CSingleTest(int nTmp, string strName)
		{
			cout << "CSingleTest(int nTmp,string strName)" << endl;
		}
		~CSingleTest()
		{
			cout << "~CSingleTest()" << endl;
		}
		void Test()
		{
			cout << "Test()" << endl;
			cout << (void*)this << endl;
		}
	};
	
	int main()
	{
		{
			CSingleton<CSingleTest>::GetInstance()->Test();
	
			CSingleton<CSingleTest>::GetInstance(10)->Test();
	
			CSingleton<CSingleTest>::GetInstance(10,"singleton")->Test();
		}
	
		system("pause");
	    return 0;
	}

### 分析
--------------------------------------------------

上述4种实现各有优缺点

1. 第1种：非线程安全、基于继承实现

	在多线程的情况下不能保证只有一个实例，但是可以阻止实例间的复制拷贝、复制赋值操作，运行期多创建实例触发断言

2. 第2种：非继承实现、手动释放资源

	调用方式`CSingleton<CSingleTest>::GetInstance()`

	这种实现增加了线程安全（著名的解决方案DCLP Double-Checked Locking Pattern，也并非线程安全）使用两次判断来解决线程安全问题并且提高效率

	增加了可以选择合适构造函数进行初始化，解决了多种类型的单例可能需要创建多个类型的单例

	所持有的资源必须手动释放

	不会阻止实例间的复制拷贝、复制赋值操作

	**关于DCLP的分析可以参考Scott Meyers 和 Andrei Alexandrescu写的《C++ and the Perils of Double-Checked Locking》**

3. 第3种：在第2种方案上增加了非手动释放资源

4. 第4种：改用局部静态变量

	这种实现可以保证线程安全，不论在window平台下还是Linux平台下（取决于所使用的编译器），同时满足多种类型的单例可能需要创建多个类型的单例，但是每次获取时都需要加锁，效率上损失，以我所在的平台为例，下面是程序运行到`static T instance(std::forward<Args>(args)...);`时要执行的代码：

		// Control access to the initialization expression.  Only one thread may leave
		// this function before the variable has completed initialization, this thread
		// will perform initialization.  All other threads are blocked until the
		// initialization completes or fails due to an exception.
		extern "C" void __cdecl _Init_thread_header(int* const pOnce)
		{
		    _Init_thread_lock();
		
		    if (*pOnce == Uninitialized)
		    {
		        *pOnce = BeingInitialized;
		    }
		    else
		    {
		        while (*pOnce == BeingInitialized)
		        {
		            // Timeout can be replaced with an infinite wait when XP support is
		            // removed or the XP-based condition variable is sophisticated enough
		            // to guarantee all waiting threads will be woken when the variable is
		            // signalled.
		            _Init_thread_wait(XpTimeout);
		
		            if (*pOnce == Uninitialized)
		            {
		                *pOnce = BeingInitialized;
		                _Init_thread_unlock();
		                return;
		            } 
		        }
		        _Init_thread_epoch = _Init_global_epoch;
		    }
		
		    _Init_thread_unlock();
		}

	更多关于局部静态变量的线程安全问题可以参考这个[解释][stack]，或者这篇[文章][OSDEV]，
	**上述第2、3、4种实现方式不会阻止实例间的复制拷贝、复制赋值操作，若想阻止这些操作可以借助boost库的类noncopyable或者自己实现一个类似的noncopyable**


