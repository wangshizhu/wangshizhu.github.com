---
layout: second_template
title: 遍历二叉树
category: data_structure
tagline: "Supporting tagline"
tags : [data_structure]
permalink: ergodic_binary_tree
---

* 二叉搜索树

		#include "stdafx.h"
		#include <iostream>
		#include <stack>
		#include <stdlib.h>
		#include <time.h>
		using namespace std;

		struct Node
		{
			Node *pLeftNode;
			int nData;
			Node *pRightNode;

			Node(int nRootData)
			{
				pLeftNode = nullptr;
				nData = nRootData;
				pRightNode = nullptr;
			}

			void AddNode(Node* pNewNode)
			{
				if (nullptr == pNewNode)
				{
					return;
				}
				if (pNewNode->nData < nData)
				{
					if (nullptr != pLeftNode)
					{
						pLeftNode->AddNode(pNewNode);
					}
					else
					{
						pLeftNode = pNewNode;
					}
				}
				else
				{
					if (nullptr != pRightNode)
					{
						pRightNode->AddNode(pNewNode);
					}
					else
					{
						pRightNode = pNewNode;
					}
				}
			}
		};

* 前序遍历
	
	- 访问根节点

	- 前序遍历左子树

	- 前序遍历右子树

	![Alt text][preorder]

	[preorder]: assets/themes/my_blog/img/preorder.jpg

	- 递归实现

	`
	void PreOrderWithRecursion(Node* pRoot)
	{
		if (nullptr == pRoot)
		{
			return;
		}
		cout << pRoot->nData << endl;
		PreOrderWithRecursion(pRoot->pLeftNode);
		PreOrderWithRecursion(pRoot->pRightNode);
	}
	`

	- 非递归实现

	`
	void PreOrderWithStack(Node* pRoot)
	{
		if (nullptr == pRoot)
		{
			return;
		}
		std::stack<Node*> nodeStack;
		while (pRoot != nullptr || !nodeStack.empty())
		{
			if (pRoot != nullptr)
			{
				cout << pRoot->nData << endl;
				nodeStack.push(pRoot);
				pRoot = pRoot->pLeftNode;
			}
			else
			{
				pRoot = nodeStack.top();
				nodeStack.pop();
				pRoot = pRoot->pRightNode;
			}
		}
	}		
	`

* 中序遍历

	- 中序遍历左子树

	- 访问根节点

	- 中序遍历右子树


	![Alt text][middleorder]

	[middleorder]: assets/themes/my_blog/img/middle_order.jpg

	对一个二叉搜索树进行中序遍历是一种按数据大小的顺序遍历

	- 递归实现

	`
	void MiddleOrderWithRecursion(Node* pRoot)
	{
		if (nullptr == pRoot)
		{
			return;
		}
		MiddleOrderWithRecursion(pRoot->pLeftNode);
		cout << pRoot->nData << endl;
		MiddleOrderWithRecursion(pRoot->pRightNode);
	}
	`

	- 非递归实现

	`
	void MiddleOrderWithStack(Node* pRoot)
	{
		if (nullptr == pRoot)
		{
			return;
		}
		std::stack<Node*> nodeStack;
		while (nullptr != pRoot || !nodeStack.empty())
		{
			if (nullptr != pRoot)
			{
				nodeStack.push(pRoot);
				pRoot = pRoot->pLeftNode;
			}
			else
			{
				pRoot = nodeStack.top();
				cout << pRoot->nData << endl;
				nodeStack.pop();
				pRoot = pRoot->pRightNode;
			}
		}
	}
	`

* 后序遍历

	- 后序遍历左子树

	- 后序遍历右子树

	- 访问根节点

	![Alt text][lastorder]

	[lastorder]: assets/themes/my_blog/img/last_order.jpg

	- 递归实现

	`
	void LastOrderWithRecursion(Node* pRoot)
	{
		if (nullptr == pRoot)
		{
			return;
		}
		LastOrderWithRecursion(pRoot->pLeftNode);
		LastOrderWithRecursion(pRoot->pRightNode);
		cout << pRoot->nData << endl;
	}
	`

	- 非递归实现

	`
	void LastOrderWithStack(Node *pRoot)
	{
		if (nullptr == pRoot)
		{
			return;
		}
		std::stack<Node*> tmpStack, outputStack;
		tmpStack.push(pRoot);
		while (!tmpStack.empty())
		{
			Node *curr = tmpStack.top();
			outputStack.push(curr);
			tmpStack.pop();
			if (curr->pLeftNode)
				tmpStack.push(curr->pLeftNode);
			if (curr->pRightNode)
				tmpStack.push(curr->pRightNode);
		}
		while (!outputStack.empty())
		{
			cout << outputStack.top()->nData << endl;
			outputStack.pop();
		}
		cout << endl;
	}
	`