---
layout: second_template
title: 单例模式
category: design_pattern
tagline: "Supporting tagline"
tags : [design_pattern]
permalink: singleton
---

* ### 测试环境 ###
	
	vs2015

	win10 64位

* ### 解释 ###

	确保一个类有且仅有一个实例

* ### 代码示例 ###
	
	以下实现非线程安全、基于继承实现

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

	下面是多线程代码示例:

		#include "stdafx.h"
		#include <iostream>
		#include <mutex>
		#include <string>
		using namespace std;

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
			static std::shared_ptr<T> m_ptrT;
			static std::mutex m_mutex;
		}; 

		template<class T> std::shared_ptr<T> CSingleton<T>::m_ptrT = nullptr;
		template<class T> std::mutex CSingleton<T>::m_mutex;

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
			CSingleTest(int nTmp,string strName)
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
			}
		};

		int main()
		{
			{
				CSingleton<CSingleTest>::GetInstance()->Test();
			}
			system("pause");
		    return 0;
		}