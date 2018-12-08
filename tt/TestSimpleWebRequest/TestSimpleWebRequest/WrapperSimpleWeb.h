#ifndef WRAPPER_SIMPLE_WEB_H
#define WRAPPER_SIMPLE_WEB_H

#include "client_http.hpp"
#include "client_https.hpp"
#include <boost/asio/strand.hpp>
#include "SimpleWebClientMgr.h"

namespace WebClientMgr
{
	using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
	using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;
	using IOService = std::shared_ptr<boost::asio::io_service>;
	using HeaderMap = SimpleWeb::CaseInsensitiveMultimap;
	using Callback = std::function<void(const SimpleWeb::error_code&, const std::string& strContent)>;

	class WebReqClientBase
	{
	public:
		WebReqClientBase();

		virtual ~WebReqClientBase();

		// 以get的方式请求
		virtual void RequstWithGet(const std::string& strParam, Callback cb);

		// 以post的方式请求
		virtual void RequstWithPost(const std::string& strUrl, const std::string& strPostData, const HeaderMap& mapHeaderMap, Callback cb);
	};

	class WebReqHttpClient : public WebReqClientBase
	{
	public:
		WebReqHttpClient(IOService service, const std::string& strIPPort, const long& nReqTimeout, const long& nConnectTimeout);

		virtual ~WebReqHttpClient();

		// 以get的方式请求
		virtual void RequstWithGet(const std::string& strParam, Callback cb);

		// 以post的方式请求
		virtual void RequstWithPost(const std::string& strParam, const std::string& strPostData, const HeaderMap& mapHeaderMap, Callback cb);

	private:
		// http回调
		void HttpRequstCallback(Callback cb, std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> rsp, const SimpleWeb::error_code &ec);

	private:
		std::unique_ptr<HttpClient> m_ptrHttpClient;
	};


	class WebReqHttpsClient : public WebReqClientBase
	{
	public:
		WebReqHttpsClient(IOService service, const std::string& strIPPort, const long& nReqTimeout, const long& nConnectTimeout);

		virtual ~WebReqHttpsClient();

		// 以get的方式请求
		virtual void RequstWithGet(const std::string& strParam, Callback cb);

		// 以post的方式请求
		virtual void RequstWithPost(const std::string& strParam, const std::string& strPostData, const HeaderMap& mapHeaderMap, Callback cb);

	private:
		// https回调
		void HttpsRequstCallback(Callback cb, std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTPS>::Response> rsp, const SimpleWeb::error_code &ec);

	private:
		std::unique_ptr<HttpsClient> m_ptrHttpsClient;
	};
}

#endif 
