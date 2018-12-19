---
layout: second_template
title: 冒泡排序及优化
category: algorithm
tagline: "Supporting tagline"
tags : [algorithm]
permalink: bubble_sort
---

* 测试环境
	
	vs2015

	C++11	

	win10 64位

* 描述

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

		void Print(int nUnSorted[], const int& nLength)
		{
			for (int i = 0; i < nLength; i++)
			{
				cout << nUnSorted[i] << " ";
			}
			cout << endl;
		}

		void BubbleSort(int nUnSorted[], const int& nLength)
		{
			if (nLength <= 1)
			{
				return;
			}
			for (int i = 0;i<nLength;i++)
			{
				for (int j = 0;j<nLength-1;j++)
				{
					if (nUnSorted[j] > nUnSorted[j+1])
					{
						nUnSorted[j] ^= nUnSorted[j + 1];
						nUnSorted[j + 1] ^= nUnSorted[j];
						nUnSorted[j] ^= nUnSorted[j + 1];
					}
				}
				Print(nUnSorted, nLength);
			}
		}

		int main()
		{
			int nUnSortedArray[] = { 5,8,6,3,9,2,1,7};

			{
				BubbleSort(nUnSortedArray, arraySize(nUnSortedArray));
			}

			system("pause");
		    return 0;
		}

	输出：

		5 6 3 8 2 1 7 9
		5 3 6 2 1 7 8 9
		3 5 2 1 6 7 8 9
		3 2 1 5 6 7 8 9
		2 1 3 5 6 7 8 9
		1 2 3 5 6 7 8 9
		1 2 3 5 6 7 8 9
		1 2 3 5 6 7 8 9

* 优化

	从上面的输出可以发现两个优化点：

	- 从第6轮开始数组已经有序，可以增加是否有序判断
	- 每一轮都使数组的有序区间增长，例如第1轮有序区间长度为1，第2轮有序区间长度为2，
	实际有序区间也有可能比轮数还要长，例如这样的数组{3,4,2,1,5,6,7,8}，记录一个无序区边界，也就是最后一次交换数值的索引

	优化后代码：

		void BubbleSort(int nUnSorted[], const int& nLength)
		{
			if (nLength <= 1)
			{
				return;
			}
			// 无序区边界
			int nUnsortBoundary = nLength - 1;
			int nLastSwapIndex = 0;
			for (int i = 0;i<nLength && nUnsortBoundary > 0;i++)
			{
				bool bSwaped = false;
				for (int j = 0;j<nUnsortBoundary;j++)
				{
					if (nUnSorted[j] > nUnSorted[j+1])
					{
						nUnSorted[j] ^= nUnSorted[j + 1];
						nUnSorted[j + 1] ^= nUnSorted[j];
						nUnSorted[j] ^= nUnSorted[j + 1];
						bSwaped = true;
						nLastSwapIndex = j;
					}
				}

				nUnsortBoundary = nLastSwapIndex;
				Print(nUnSorted, nLength);

				if (!bSwaped)
				{
					break;
				}
			}
		}

	输出：

		5 6 3 8 2 1 7 9
		5 3 6 2 1 7 8 9
		3 5 2 1 6 7 8 9
		3 2 1 5 6 7 8 9
		2 1 3 5 6 7 8 9
		1 2 3 5 6 7 8 9