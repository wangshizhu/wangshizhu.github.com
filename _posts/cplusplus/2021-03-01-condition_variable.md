---
layout: second_template
title: std::condition_variable
category: C++
tagline: "Supporting tagline"
tags : [C++]
permalink: condition_variable
---

[condition_variable]:https://zh.cppreference.com/w/cpp/thread/condition_variable
[Cameron_blog]:https://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++
[scott]:https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf

condition_variable是C++多线程同步原语，关于其API可以参考[这里][condition_variable]，在实际开发LOG系统时使用了condition_variable

LOG系统使用condition_variable背景

目录：

- [使用背景](#使用背景)
- [问题和解决方案](#问题和解决方案)
- [延伸](#延伸)

### 使用背景
--------------------------------------------------

LOG系统设想：

* 业务线程产生log数据
* LOG系统处理log数据
* LOG系统内部线程与业务线程属于两个不同的线程
* LOG系统内部线程发现有log数据则开始处理，没有则阻塞

基于这样的设想，需要：

* 一个线程间共享的消息队列
* 互斥锁
* 消息通知机制

两个线程工作流程：

1. 业务线程产生log数据，获取消息队列锁
2. 向消息队列写数据
3. 通知LOG系统内部线程

### 问题和解决方案
--------------------------------------------------

* 按照上面的工作流程，在第1步可能阻塞业务线程，造成业务线程处理能力下降
	
	当业务线程产生log数据而此时LOG系统内部线程正在处理堆积的log数据，所以第1步可能阻塞业务线程，造成业务线程处理能力下降
	
	**最好是不会阻塞业务线程，可以使用无锁队列，关于无锁队列可以看下面的[延伸](#延伸)小结**
	
* 调用notify_all函数通知时，造成“急促并等待”问题
	
	更完整的解释：
	
	> **通知线程**不必保有**等待线程**所保有的同一互斥上的锁；因为**被通知线程**将立即再次阻塞，**等待通知线程**释放锁。然而一些实现（尤其是许多 pthread 的实现）辨识此情形，在通知调用中，
	直接从条件变量队列转移等待线程到互斥队列，而不唤醒它，以避免此“急促并等待”场景
	
	> **在 std::condition_variable 上执行 notify_one 或 notify_all 不需要为通知保有锁**
	
	例如下代码：
	
		std::unique_lock<std::mutex> lock(mtx_);
		
		log_msg_.emplace_back(...);
		
		cv_.notify_one();
		
	更期望在通知之前将锁释放掉，所以可以将插入操作包进块中
		
		{
			std::unique_lock<std::mutex> lock(mtx_);
			
			log_msg_.emplace_back(...);
		}
		
		cv_.notify_one();
		
* notify_one 导致的虚假唤醒spurious wakeup
	
	从字面意思看这个函数会通知一个线程，实则不然，**当发生虚假唤醒，线程应该检测执行条件**，更完整的解释如下：
	
	> condition_variable 被通知时，时限消失或虚假唤醒发生，线程被唤醒，且自动重获得互斥。之后线程应检查条件，若唤醒是虚假的，则继续等待
	
	例如：
		
		...
		
		while (log_msg_.empty())
		{
			if (stop_)
			{
				return;
			}
			cv_.wait(lock);
		}
				
		FunType& func = *log_msg_.begin();
				
		func();
				
		log_msg_.pop_front();
		
		...

* wait系列函数的正确使用
	
	std::condition_variable **只可**与 std::unique_lock<std::mutex> 一同使用，执行 wait 、 wait_for 或 wait_until时，
	等待操作自动释放互斥，并悬挂线程的执行，例如：
	
		std::unique_lock<std::mutex> lock(mtx_);
		cv_.wait(lock);

### 延伸
--------------------------------------------------

上面[问题和解决方案](#问题和解决方案)小结，提到了无锁队列，使用无锁队列可以不阻塞业务线程，但是无锁队列设计起来很麻烦，但是可以作为学习目标。

使用无锁队列那么上面的代码：

	std::unique_lock<std::mutex> lock(mtx_);
	
	log_msg_.emplace_back(...);
	
	cv_.notify_one();
	
可以简化成：
	
	
	lock_free_queue_.enqueue(...);
	
	cv_.notify_all();
	
使用无锁队列是不是就可以不使用condition_variable了？假设不使用condition_variable的wait函数阻塞住LOG内部线程，那么LOG内部线程就需要不断检测条件，以此来快速将LOG写入
文件，这样就导致CPU飙升。所以还需要wait函数，如下：

	FunType func;
	bool ok = log_msg_lock_free_.try_dequeue(func);
	if (!ok)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		cv_.wait(lock);
		continue;
	}
		
	func();
	
由于使用的是无锁队列，所以当业务线程写完log数据后直接调用通知函数即可：

	cv_.notify_all();
	
	
关于无锁队列Lock-Free queue设计思想可以参考Maged M. Michael 和 Michael L. Scott 1996 年发表的[论文][scott]

无锁队列的实现有boost库实现的boost::lockfree::queue，有Intel的TBB，还有C++大神Cameron的moodycamel::ConcurrentQueue，
关于这些无锁队列的测试还有实现可以参考大神Cameron的这篇[文章][Cameron_blog]，文章里介绍了moodycamel::ConcurrentQueue的实现和其他无锁队列的测试数据对比
