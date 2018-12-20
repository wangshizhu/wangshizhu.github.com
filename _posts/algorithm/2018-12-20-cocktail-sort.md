---
layout: second_template
title: 鸡尾酒排序及优化
category: algorithm
tagline: "Supporting tagline"
tags : [algorithm]
permalink: cocktail_sort
---

[bubble_sort]: /bubble_sort

* 测试环境
	
	vs2015

	C++11	

	win10 64位

* 描述

	鸡尾酒排序的过程可以想象成钟摆（左-->右-->左-->右.....），鸡尾酒排序的优点是在特定的条件下减少排序的回合数，适用于大部分有序的数据环境下

	时间复杂度： N^2

	稳定性： 稳定

* 代码：
	
		#include "stdafx.h"
		#include <iostream>
		using namespace std;

		template<typename T, std::size_t N>
		constexpr std::size_t arraySize(T(&)[N]) noexcept
		{
			return N;
		}

		void Print(int nUnSorted[], const std::size_t& nLength)
		{
			for (std::size_t i = 0; i < nLength; i++)
			{
				cout << nUnSorted[i] << " ";
			}
			cout << endl;
		}

		void CocktailSort(int nUnSorted[], const std::size_t& nLength)
		{
			if (nLength <= 1)
			{
				return;
			}
			std::size_t nNum = nLength / 2;
			for (std::size_t i = 0 ;i < nNum; i++)
			{
				bool bSwaped = false;
				// 从左侧开始
				for (std::size_t nLeft = i; nLeft < nLength-i-1; nLeft++)
				{
					if (nUnSorted[nLeft] > nUnSorted[nLeft + 1])
					{
						nUnSorted[nLeft] ^= nUnSorted[nLeft+1];
						nUnSorted[nLeft+1] ^= nUnSorted[nLeft];
						nUnSorted[nLeft] ^= nUnSorted[nLeft+1];
						bSwaped = true;
					}
				}

				Print(nUnSorted, nLength);

				if (!bSwaped)
				{
					break;
				}
				bSwaped = false;
				// 从右侧开始
				for (std::size_t nRight = nLength-i-1; nRight > i; nRight--)
				{
					if (nUnSorted[nRight] < nUnSorted[nRight-1])
					{
						nUnSorted[nRight] ^= nUnSorted[nRight-1];
						nUnSorted[nRight-1] ^= nUnSorted[nRight];
						nUnSorted[nRight] ^= nUnSorted[nRight-1];
						bSwaped = true;
					}
				}

				Print(nUnSorted, nLength);

				if (!bSwaped)
				{
					break;
				}
			}
		}

		int main()
		{
			int nUnSortedArray[] = { 2,3,4,5,7,6,8,1 };

			{
				CocktailSort(nUnSortedArray, arraySize(nUnSortedArray));
			}

			system("pause");
			return 0;
		}

	输出：

		2 3 4 5 6 7 1 8
		1 2 3 4 5 6 7 8
		1 2 3 4 5 6 7 8

* 比较
	
	以上面代码中的数组{ 2,3,4,5,7,6,8,1 }为例，按照[冒泡排序][bubble_sort]文章中优化后的代码进行排序，会输出：

		2 3 4 5 6 7 1 8
		2 3 4 5 6 1 7 8
		2 3 4 5 1 6 7 8
		2 3 4 1 5 6 7 8
		2 3 1 4 5 6 7 8
		2 1 3 4 5 6 7 8
		1 2 3 4 5 6 7 8

* 优化

	按照[冒泡排序][bubble_sort]文章中优化的第2点我们也增加***无序区边界***

	只不过这个无序区边界是个双向的（左侧无序区边界、右侧无序区边界）

	此时鸡尾酒排序的过程可以想象成***具有固定大小能量缺失的钟摆***

	优化后代码：

		void CocktailSort(int nUnSorted[], const std::size_t& nLength)
		{
			if (nLength <= 1)
			{
				return;
			}
			std::size_t nNum = nLength / 2;
			int nRightLastSwap = 0;
			int nLeftLastSwap = 0;
			// 左侧无序区边界
			int nLeftUnsortBoundary = 0;
			// 右侧无序区边界
			int nRightUnsortBoundary = nLength - 1;
			for (std::size_t i = 0 ;i < nNum; i++)
			{
				bool bSwaped = false;
				// 从左侧开始
				for (std::size_t nLeft = nLeftUnsortBoundary; nLeft < nRightUnsortBoundary; nLeft++)
				{
					if (nUnSorted[nLeft] > nUnSorted[nLeft + 1])
					{
						nUnSorted[nLeft] ^= nUnSorted[nLeft+1];
						nUnSorted[nLeft+1] ^= nUnSorted[nLeft];
						nUnSorted[nLeft] ^= nUnSorted[nLeft+1];
						bSwaped = true;
						nRightLastSwap = nLeft;
					}
				}

				nRightUnsortBoundary = nRightLastSwap;
				
				Print(nUnSorted, nLength);

				if (!bSwaped)
				{
					break;
				}
				bSwaped = false;
				// 从右侧开始
				for (std::size_t nRight = nRightUnsortBoundary; nRight > nLeftUnsortBoundary; nRight--)
				{
					if (nUnSorted[nRight] < nUnSorted[nRight-1])
					{
						nUnSorted[nRight] ^= nUnSorted[nRight-1];
						nUnSorted[nRight-1] ^= nUnSorted[nRight];
						nUnSorted[nRight] ^= nUnSorted[nRight-1];
						bSwaped = true;
						nLeftLastSwap = nRight;
					}
				}

				nLeftUnsortBoundary = nLeftLastSwap;

				Print(nUnSorted, nLength);

				if (!bSwaped)
				{
					break;
				}
			}
		}