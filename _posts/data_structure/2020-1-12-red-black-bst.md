---
layout: second_template
title: 平衡树之红黑树
category: data_structure
tagline: "Supporting tagline"
tags : [data_structure]
permalink: red-black-bst
---

在二叉搜索树文章中提到了最坏情况，这种情况是不能接受的，例如在游戏中的战力值排行榜，数值并不是那么随机，所以更期望在一颗含有N个节点的树中，树的高度为~lgN，
这样就能保证所有查找都能在~lgN次比较内结束

在谈红黑树之前先了解一下2-3树，下图是一颗2-3树

![Alt text][id]

[id]: assets/themes/my_blog/img/2-3Tree.jpg

从图中看出，一个2节点含有一个键两条链接，一个3节点含有两个键三条链接，**一颗完美平衡的2-3查找树种的所有空链接到根节点的距离都应该相同**