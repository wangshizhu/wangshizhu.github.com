---
layout: main_layout
title: 王仕柱的主页
tagline: Supporting tagline
---
{% include JB/setup %}

{% highlight python %}
class Functor:
	def __init__(self, func, *args):
		self.func = func
		self.args = args

	def __call__(self, *args):
		self.func(*(self.args + args))
{% endhighlight %}




