#include "SimpleWebClientMgr.h"

CSimpleWebClientMgr::CSimpleWebClientMgr(IOService service)
{
	m_ioService = service;

	m_mapWebReqClient.clear();

	m_nReqTimeout = 0;

	m_nConnectTimeout = 0;
}

CSimpleWebClientMgr::CSimpleWebClientMgr(IOService service, const long& nReqTimeout, const long& nConnectTimeout)
{
	m_ioService = service;

	m_mapWebReqClient.clear();

	m_nReqTimeout = nReqTimeout;

	m_nConnectTimeout = nConnectTimeout;
}

void CSimpleWebClientMgr::RequstWithGet(const std::string& strUrl, Callback cb)
{
	std::string strIPPort = ParseIPPortFromUrl(strUrl);
	if (strIPPort.size() == 0)
	{
		cb(boost::system::errc::make_error_code(boost::system::errc::bad_address),std::string());
		return;
	}

	std::string strProto = GetHttpOrHttps(strUrl);
	std::string strParam = GetParam(strUrl);

	std::shared_ptr<WebReqClientBase> shareWebReqClient = GetWebReqClient(strIPPort, strProto);
	shareWebReqClient->RequstWithGet(strParam, std::move(cb));
}

void CSimpleWebClientMgr::RequstWithPost(const std::string& strUrl, const std::string& strPostData, const HeaderMap& mapHeaderMap, Callback cb)
{
	std::string strIPPort = ParseIPPortFromUrl(strUrl);
	if (strIPPort.size() == 0)
	{
		cb(boost::system::errc::make_error_code(boost::system::errc::bad_address), std::string());
		return;
	}

	std::string strProto = GetHttpOrHttps(strUrl);
	std::string strParam = GetParam(strUrl);

	std::shared_ptr<WebReqClientBase> shareWebReqClient = GetWebReqClient(strIPPort, strProto);
	shareWebReqClient->RequstWithPost(strParam, strPostData, mapHeaderMap, std::move(cb));
}

std::shared_ptr<WebReqClientBase> CSimpleWebClientMgr::GetWebReqClient(const std::string& strIPPort, const std::string& strProto)
{
	auto itWebReqClient = m_mapWebReqClient.find(strIPPort);
	if (itWebReqClient != m_mapWebReqClient.end())
	{
		return itWebReqClient->second;
	}

	if (strProto == WEB_REQ_HTTP)
	{
		std::shared_ptr<WebReqHttpClient> shareHttpClient = std::make_shared<WebReqHttpClient>(m_ioService, strIPPort, m_nReqTimeout, m_nConnectTimeout);
		m_mapWebReqClient.insert(std::make_pair(strIPPort, shareHttpClient));
	}
	else
	{
		std::shared_ptr<WebReqHttpsClient> shareHttpsClient = std::make_shared<WebReqHttpsClient>(m_ioService, strIPPort, m_nReqTimeout, m_nConnectTimeout);
		m_mapWebReqClient.insert(std::make_pair(strIPPort, shareHttpsClient));
	}
	return m_mapWebReqClient[strIPPort];
}

std::string CSimpleWebClientMgr::ParseIPPortFromUrl(const std::string& strUrl)
{
	std::string strProto = GetHttpOrHttps(strUrl);
	std::size_t  nProtoHeadLen = strProto.size();
	std::size_t  nLen = strUrl.size();
	if (0 == nProtoHeadLen)
	{
		return std::string();
	}

	std::string strIpport = strUrl.substr(nProtoHeadLen, nLen);
	std::size_t nIPPortEnd = strIpport.find("/");
	if (nIPPortEnd == std::string::npos)
	{
		return std::string();
	}

	return strIpport.substr(0, nIPPortEnd);
}

std::string CSimpleWebClientMgr::GetHttpOrHttps(const std::string& strUrl)
{
	std::string strHttp(WEB_REQ_HTTP);
	std::string strHttps(WEB_REQ_HTTPS);

	std::size_t protoHttpEndPos = strUrl.find(strHttp);
	if (protoHttpEndPos != std::string::npos)
	{
		return strHttp;
	}

	std::size_t protoHttpsEndPos = strUrl.find(strHttps);
	if (protoHttpsEndPos != std::string::npos)
	{
		return strHttps;
	}

	return std::string();
}

std::string CSimpleWebClientMgr::GetParam(const std::string& strUrl)
{
	std::string strProto = GetHttpOrHttps(strUrl);
	std::string strIpport = strUrl.substr(strProto.size(), strUrl.size());
	std::size_t nIPPortEnd = strIpport.find("/");

	return strIpport.substr(nIPPortEnd, strIpport.size());
}