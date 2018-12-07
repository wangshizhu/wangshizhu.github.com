#include "SimpleWebReqManager.h"
#include "Hold.h"

extern Hold gObjHold;

SimpleWebReqManager::SimpleWebReqManager()
{
	m_ioService = std::make_shared<boost::asio::io_service>();
	m_ioWork = std::make_shared<boost::asio::io_service::work>(*m_ioService);
	m_thread.reset(new boost::thread(boost::bind(&SimpleWebReqManager::Run, this)));

	/*m_RedPacketClient.reset(new SimpleWeb::Client<SimpleWeb::HTTP>("120.76.194.200:7480"));
	m_RedPacketClient->io_service = m_ioService;
	m_RedPacketClient->config.timeout_connect = 0;
	m_RedPacketClient->config.timeout = 0;*/
}

void SimpleWebReqManager::Run()
{
	cout << "===" << endl;
	Req();
}

void SimpleWebReqManager::Req()
{
	while (1)
	{
		gObjHold.GetHoldIOService()->post([=] {gObjHold.Req(); });
		Sleep(10);
	}
}

void SimpleWebReqManager::Join()
{
	if (!m_thread)
	{
		return;
	}
	m_thread->join();

	gObjHold.Join();
}

void SimpleWebReqManager::Stop()
{
	if (!m_ioService)
	{
		return;
	}
	m_ioService->stop();
}

