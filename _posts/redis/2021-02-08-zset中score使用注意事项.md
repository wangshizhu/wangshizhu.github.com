---
layout: second_template
title: zset中score使用注意事项
category : Redis
tagline: "Supporting tagline"
tags : [Redis]
permalink: zset-score
---

zset语法如下：
	
	zset key score member

实际开发中使用zset做排序，在应用层score必须是个64位整型，例如下面场景：

1. 高8位代表一个排序规则
2. 次8位代表一个排序规则
3. 再一个16位代表一个排序规则
4. 低32位代表一个排序规则

按照上面的顺序生成一个64位整型，当144396670029746945
