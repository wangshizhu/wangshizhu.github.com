#ifndef HOLD_H
#define HOLD_H

#include "client_http.hpp"
#include <boost/asio/strand.hpp>
#include <boost/thread.hpp>
#include "SimpleWebClientMgr.h"
using namespace std;
using WebClientMgr::CSimpleWebClientMgr;
class Hold
{
public:
	Hold();

	void Stop();

	void Join();

	void ReqCallBackEx(const SimpleWeb::error_code &ec,const std::string & strContent);

	void Req();

	std::shared_ptr<boost::asio::io_service> GetHoldIOService() { return m_ioService; };

private:
	std::shared_ptr<boost::asio::io_service> m_ioService;
	std::shared_ptr<boost::asio::io_service::work> m_ioWork;
	boost::shared_ptr<boost::thread> m_thread;
	std::shared_ptr<CSimpleWebClientMgr> m_shareWebClientMgr;
};


#endif 
