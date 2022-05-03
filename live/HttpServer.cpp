//
// Created by rjd
//

#include "HttpServer.h"
#include "utils/Logger.h"
#include "utils/Format.h"
#include "utils/File.h"
#include "WebData.h"

HttpServer::HttpServer(EventLoop* loop, const std::string& server_name, const InetAddress& address) :
		TcpServer(loop, server_name, address),
		loop_(loop),
		server_address_(address)
{
	SetNewMessageCallback([this](auto&& PH1, auto&& PH2, auto&& PH3)
	{
		OnHttpMessage(PH1, PH2, PH3);
	});
}

void HttpServer::SetHttpRoot(const std::string& http_root)
{
	http_root_ = http_root;
}

void HttpServer::OnHttpMessage(const TcpConnectionPtr& connection_ptr, Buffer* buffer, Timestamp timestamp)
{
	std::string request_data = buffer->ReadAllAsString();

	std::string url = Format::GetUrl(request_data);
	LOG_INFO << connection_ptr->GetConnectionName() << " " << url;

	std::string response_text = GetResponseText(url);

	connection_ptr->Send("HTTP/1.1 200 OK\r\n"
						 "Content-Length: " + std::to_string(response_text.length()) + "\r\n\r\n" +
						 response_text);
}

std::string HttpServer::GetResponseText(const std::string& url)
{
	std::string response_text;
	if (!http_root_.empty())
	{
		response_text = GetResponseTextFromFile(url);
	}
	if (response_text.empty())
	{
		response_text = GetResponseTestFromVar(url);
	}
	return response_text;
}

std::string HttpServer::GetResponseTextFromFile(const std::string& url)
{
	std::string response_text;
	std::string file_path;
	if (url == "/flv.js")
	{
		file_path = http_root_ + "/flv.js";
	}
	else if (url == "/flv.css")
	{
		file_path = http_root_ + "/flv.css";
	}
	else
	{
		file_path = http_root_ + "/index.html";
	}
	if (!File::ReadFileAsString(file_path, response_text))
	{
		LOG_ERROR << "read file error " << file_path;
	}
	return response_text;
}

std::string HttpServer::GetResponseTestFromVar(const std::string& url)
{
	if (url == "/flv.js")
	{
		return js_data;
	}
	else if (url == "/flv.css")
	{
		return css_data;
	}
	else
	{
		return index_data;
	}
}
