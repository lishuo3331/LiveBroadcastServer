//
// Created by rjd
//

#include "HttpServer.h"
#include "utils/Logger.h"
#include "utils/Format.h"
#include "utils/File.h"

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

	std::string response_text;
	std::string file_path;
	bool read_file = true;
	if (url == "/" || url == "/index.html")
	{
		file_path = http_root_ + "/index.html";
	}
	else if (url == "/flv.js")
	{
		file_path = http_root_ + "/flv.js";
	}
	else if (url == "/flv.css")
	{
		file_path = http_root_ + "/flv.css";
	}
	else
	{
		response_text = "-1";
		read_file = false;
	}
	if (read_file && !File::ReadFileAsString(file_path, response_text))
	{
		response_text = "Error";
		LOG_ERROR << "read file error " << file_path;
	}
	connection_ptr->Send("HTTP/1.1 200 OK\r\n"
						 "Content-Length: " + std::to_string(response_text.length()) + "\r\n\r\n" +
						 response_text);
}
