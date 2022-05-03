//
// Created by rjd
//

#ifndef LIVEBROADCASTSERVER_HTTPSERVER_H
#define LIVEBROADCASTSERVER_HTTPSERVER_H

#include "network/TcpServer.h"

class HttpServer : public TcpServer
{
public:

    HttpServer(EventLoop* loop, const std::string& server_name, const InetAddress& address);

	void SetHttpRoot(const std::string& http_root);

private:
    EventLoop* loop_;
    InetAddress server_address_;
	std::string http_root_;
	void OnHttpMessage(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp);

	std::string GetResponseText(const std::string& url);

	std::string GetResponseTextFromFile(const std::string& url);

	static std::string GetResponseTestFromVar(const std::string& url);
};


#endif //LIVEBROADCASTSERVER_HTTPSERVER_H
