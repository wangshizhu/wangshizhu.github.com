#ifndef SIMPLE_WEB_CLIENT_MGR_H
#define SIMPLE_WEB_CLIENT_MGR_H

#include "client_http.hpp"
#include <boost/asio/strand.hpp>
#include "WrapperSimpleWeb.h"

#define WEB_REQ_HTTP "http://"
#define WEB_REQ_HTTPS "https://"

namespace WebClientMgr
{
	class WebReqClientBase;
	using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
	using HttpsClient = SimpleWeb::Client<SimpleWeb::HTTPS>;
	using IOService = std::shared_ptr<boost::asio::io_service>;
	using HeaderMap = SimpleWeb::CaseInsensitiveMultimap;
	using Callback = std::function<void(const SimpleWeb::error_code&, const std::string& strContent)>;

	class CSimpleWebClientMgr
	{
	public:
		CSimpleWebClientMgr() = delete;

		CSimpleWebClientMgr(CSimpleWebClientMgr const&) = delete;

		CSimpleWebClientMgr(CSimpleWebClientMgr const&&) = delete;

		CSimpleWebClientMgr& operator = (CSimpleWebClientMgr const&) = delete;

		explicit CSimpleWebClientMgr(IOService service);

		explicit CSimpleWebClientMgr(IOService service, const long& nReqTimeout, const long& nConnectTimeout);

		// ��get�ķ�ʽ����
		void RequstWithGet(const std::string& strUrl, Callback cb);

		// ��post�ķ�ʽ����
		void RequstWithPost(const std::string& strUrl, const std::string& strPostData, const HeaderMap& mapHeaderMap, Callback cb);

	private:
		// ��url������ip port����ʽ:"120.76.194.200:7480"
		std::string ParseIPPortFromUrl(const std::string& strUrl);

		// ��url��ȡhttp/https
		std::string GetHttpOrHttps(const std::string& strUrl);

		// ��url��ȡ����
		std::string GetParam(const std::string& strUrl);

		// ��ȡweb����ͻ���
		std::shared_ptr<WebReqClientBase> GetWebReqClient(const std::string& strIPPort, const std::string& strProto);

	private:
		IOService m_ioService;

		std::map<std::string, std::shared_ptr<WebReqClientBase>> m_mapWebReqClient;

		// ����ʱʱ�� ��λ(��)
		long m_nReqTimeout;

		// ���ӳ�ʱʱ�� ��λ(��)
		// �����ӳ�ʱʱ������Ϊ0ʱ����ʹ������ʱʱ�����
		long m_nConnectTimeout;
	};
}
#endif 
