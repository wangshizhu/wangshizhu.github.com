#ifndef __HTTP_CLIENT_POOL_H__
#define __HTTP_CLIENT_POOL_H__

#include "simplewebserver/client_http.hpp"
#include "simplewebserver/client_https.hpp"
#include "utils/string_utils.h"
#include "utils/timer.h"
#include "utils/timeutils.h"
#include <algorithm>
#include <chrono>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <boost/thread/thread.hpp>

namespace octans {
namespace net {
namespace http {

using HTTP = SimpleWeb::HTTP;
using HTTPS = SimpleWeb::HTTPS;
using HeaderMap = SimpleWeb::CaseInsensitiveMultimap;

template <class T> class Pool {
public:
    using HttpClient = SimpleWeb::Client<T>;
    using ClientPtr = std::shared_ptr<HttpClient>;
    using IdleClientPtr = std::pair<ClientPtr, int>;
    using Callback = std::function<void(bool, std::string)>;

    Pool(std::shared_ptr<boost::asio::io_service> ios, std::string address,
         int max = 64, int timeout = 3, int idle_timeout = 600)
        : ios_(ios),
          address_(address),
          max_(max),
          timeout_(timeout),
          idle_timeout_(idle_timeout),
          used_(0),
          timer_(*ios_),
          persist_client_(address_) {
        persist_client_.io_service = ios_;
        persist_client_.config.timeout = timeout_;
        timer_.Create(idle_timeout_, [this]() { OnIdleTimeout(); }, -1);
    }

    void Request(std::string method, std::string path, std::string content,
                 Callback cb, const HeaderMap &header) {
        utils::ToUpper(method);
        auto cli = GetClient();
        auto wrap = std::bind(&Pool::OnResponse, this, std::placeholders::_1,
                              std::placeholders::_2, cb, cli);
        try {
            if (cli != nullptr) {
                cli->request(method, path, content, header, wrap);
            } else {
                persist_client_.request(method, path, content, header, wrap);
            }
        } catch (const std::exception &e) {
            if (cb) {
                cb(false, e.what());
            }
            ReleaseIdle(cli);
        }
    }

private:
    void OnResponse(std::shared_ptr<typename HttpClient::Response> resp,
                    const SimpleWeb::error_code &ec, Callback cb,
                    ClientPtr cli) {
        if (ec) {
            cb(false, ec.message());
            ReleaseIdle(cli);
            return;
        }
        cb(true, resp->content.string());
        PutIdle(cli);
    }

    ClientPtr NewClient() {
        ClientPtr cli = std::make_shared<HttpClient>(address_);
        cli->io_service = ios_;
        cli->config.timeout = timeout_;
        ++used_;
        return cli;
    }

    ClientPtr GetIdle() {
        if (!idle_.empty()) {
            auto cli = idle_.back().first;
            idle_.pop_back();
            ++used_;
            return cli;
        }
        return nullptr;
    }

    ClientPtr GetClient() {
        // 有空闲连接
        auto cli = GetIdle();
        if (cli != nullptr) {
            return cli;
        }

        // 未到达最大空闲连接上限，创建新连接
        if (static_cast<int>(idle_.size()) + used_ < max_) {
            auto cli = NewClient();
            return cli;
        }

        return nullptr;
    }

    void ReleaseIdle(ClientPtr cli) {
        if (cli != nullptr) {
            --used_;
        }
    }

    void PutIdle(ClientPtr cli) {
        if (cli != nullptr) {
            --used_;
            idle_.push_front(std::make_pair(cli, utils::UTC()));
        }
    }

    void OnIdleTimeout() {
        auto now = octans::utils::UTC();
        auto timeout = now - idle_timeout_;
        // 找到第一个超时的
        auto it = std::find_if(idle_.begin(), idle_.end(),
                               [timeout](IdleClientPtr c) {
                                   return (c.second >= timeout) ? true : false;
                               });
        auto offset = it - idle_.begin();
        idle_.resize(offset);
    }

private:
    std::shared_ptr<boost::asio::io_service> ios_;
    std::string address_; // 地址
    int max_;             // 最大连接数
    int timeout_;         // 请求超时时间
    int idle_timeout_;    // 空闲超时时间
    int used_;            // 当前使用的idle数目
    utils::Timer timer_;
    std::deque<IdleClientPtr> idle_;
    HttpClient persist_client_;
}; // namespace http

} // namespace http


using service_ptr = std::shared_ptr<boost::asio::io_service>;
using work_ptr = std::shared_ptr<boost::asio::io_service::work>;
using thread_ptr = std::shared_ptr<boost::thread>;

class AsyncHttpThread
{
public:
	AsyncHttpThread()
	{
	}
	~AsyncHttpThread()
	{
	}

	bool Start()
	{
		m_ioHandler = std::make_shared<boost::asio::io_service>();
		m_ioWork = std::make_shared<boost::asio::io_service::work>(*m_ioHandler);
		m_thread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, m_ioHandler)));

		if (nullptr == m_ioHandler || nullptr == m_ioWork || nullptr == m_thread)
			return false;

		return true;
	}
	void Stop()
	{
		if (m_ioHandler)
		{
			m_ioHandler->stop();
		}
	}

	void Join()
	{
		if (m_thread)
		{
			m_thread->join();
		}
	}

	service_ptr GetIoService() { return m_ioHandler; }
private:
	service_ptr		m_ioHandler;
	work_ptr		m_ioWork;
	thread_ptr		m_thread;
};


} // namespace net
} // namespace octans

#endif // __HTTP_CLIENT_POOL_H__
