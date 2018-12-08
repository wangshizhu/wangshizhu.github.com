#include "WrapperSimpleWeb.h"

using WebClientMgr::HttpClient;
using WebClientMgr::HttpsClient;
using WebClientMgr::IOService;
using WebClientMgr::HeaderMap;
using WebClientMgr::Callback;
using WebClientMgr::WebReqClientBase;
using WebClientMgr::WebReqHttpClient;
using WebClientMgr::WebReqHttpsClient;

WebReqClientBase::WebReqClientBase()
{
}

WebReqClientBase::~WebReqClientBase()
{

}

void WebReqClientBase::RequstWithGet(const std::string& strParam, Callback cb)
{

}

void WebReqClientBase::RequstWithPost(const std::string& strUrl, const std::string& strPostData, const HeaderMap& mapHeaderMap, Callback cb)
{

}

WebReqHttpClient::WebReqHttpClient(IOService service,const std::string& strIPPort,const long& nReqTimeout, const long& nConnectTimeout)
{
	m_ptrHttpClient = std::make_unique<HttpClient>(strIPPort);
	m_ptrHttpClient->io_service = service;
	m_ptrHttpClient->config.timeout = nReqTimeout;
	m_ptrHttpClient->config.timeout_connect = nConnectTimeout;
}

WebReqHttpClient::~WebReqHttpClient()
{

}

void WebReqHttpClient::HttpRequstCallback(Callback cb, std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> rsp, const SimpleWeb::error_code &ec)
{
	cb(ec, (ec.value() != 0) ? ec.message() : rsp->content.string());
}

void WebReqHttpClient::RequstWithGet(const std::string& strParam, Callback cb)
{
	m_ptrHttpClient->request("get", strParam, std::bind(&WebReqHttpClient::HttpRequstCallback, this, cb, std::placeholders::_1, std::placeholders::_2));
}

void WebReqHttpClient::RequstWithPost(const std::string& strParam, const std::string& strPostData, const HeaderMap& mapHeaderMap, Callback cb)
{
	m_ptrHttpClient->request("post", strParam, strPostData, mapHeaderMap, std::bind(&WebReqHttpClient::HttpRequstCallback, this, cb, std::placeholders::_1, std::placeholders::_2));
}


WebReqHttpsClient::WebReqHttpsClient(IOService service, const std::string& strIPPort, const long& nReqTimeout, const long& nConnectTimeout)
{
	m_ptrHttpsClient = std::make_unique<HttpsClient>(strIPPort);
	m_ptrHttpsClient->io_service = service;
	m_ptrHttpsClient->config.timeout = nReqTimeout;
	m_ptrHttpsClient->config.timeout_connect = nConnectTimeout;
}

WebReqHttpsClient::~WebReqHttpsClient()
{

}

void WebReqHttpsClient::HttpsRequstCallback(Callback cb, std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTPS>::Response> rsp, const SimpleWeb::error_code &ec)
{
	cb(ec, (ec.value() != 0) ? std::string() : rsp->content.string());
}

void WebReqHttpsClient::RequstWithGet(const std::string& strParam, Callback cb)
{
	m_ptrHttpsClient->request("get", strParam, std::bind(&WebReqHttpsClient::HttpsRequstCallback, this, cb, std::placeholders::_1, std::placeholders::_2));
}

void WebReqHttpsClient::RequstWithPost(const std::string& strParam, const std::string& strPostData, const HeaderMap& mapHeaderMap, Callback cb)
{
	m_ptrHttpsClient->request("post", strParam, strPostData, mapHeaderMap, std::bind(&WebReqHttpsClient::HttpsRequstCallback, this, cb, std::placeholders::_1, std::placeholders::_2));
}