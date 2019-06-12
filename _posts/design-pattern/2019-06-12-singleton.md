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