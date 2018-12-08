#ifndef SIMPLE_WEB_REQ_MANAGER_H
#define SIMPLE_WEB_REQ_MANAGER_H

#include "client_http.hpp"
#include <boost/asio/strand.hpp>
#include <boost/thread.hpp>

using namespace std;

class SimpleWebReqManager
{
public:
	SimpleWebReqManager();

	void Stop();

	void Join();

	void Run();

	//void ReqCallBack(std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> rsp,const SimpleWeb::error_code &ec);

	void Req();

private:
	std::shared_ptr<boost::asio::io_service> m_ioService;
	std::shared_ptr<boost::asio::io_service::work> m_ioWork;
	boost::shared_ptr<boost::thread> m_thread;

	//std::unique_ptr<SimpleWeb::Client<SimpleWeb::HTTP>> m_RedPacketClient;
};

#endif 
