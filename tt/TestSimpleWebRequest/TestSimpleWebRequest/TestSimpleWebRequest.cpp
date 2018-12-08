// TestSimpleWebRequest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "client_http.hpp"
#include <boost/asio/strand.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include "SimpleWebReqManager.h"
using namespace boost;
using namespace std;


int main()
{
	SimpleWebReqManager swrq;

	swrq.Join();

    return 0;
}

