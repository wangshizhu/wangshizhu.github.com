#include "Hold.h"

Hold gObjHold;

Hold::Hold()
{
	m_ioService = std::make_shared<boost::asio::io_service>();
	m_ioWork = std::make_shared<boost::asio::io_service::work>(*m_ioService);
	m_thread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, m_ioService)));

	m_shareWebClientMgr = std::make_shared<CSimpleWebClientMgr>(m_ioService,5,5);
}

void Hold::Join()
{
	if (!m_thread)
	{
		return;
	}
	m_thread->join();
}

void Hold::Stop()
{
	if (!m_ioService)
	{
		return;
	}
	m_ioService->stop();
}

void Hold::Req()
{
	m_shareWebClientMgr->RequstWithGet("http://120.76.194.200:7480/xlhy-activity-inside/", std::bind(&Hold::ReqCallBackEx, this, std::placeholders::_1, std::placeholders::_2));
}

void Hold::ReqCallBackEx(const SimpleWeb::error_code &ec, const std::string & strContent)
{
	cout << ec.message() << endl;
	int nCode = ec.value();
	cout << nCode << endl;

	if (!nCode)
	{
		cout << strContent << endl;
	}

	cout << endl;
}