#include <cassert>
#include <unistd.h>
#include <netdb.h>
#include "network/SocketOps.h"
#include "utils/Logger.h"

int socketops::CreateDefaultSocket(int family)
{
	return ::socket(family, SOCK_STREAM | SOCK_CLOEXEC, 0);
}

int socketops::CreateNonblockSocket(int family)
{
	return ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
}

int socketops::Htons(int port)
{
	return ::htons(port);
}

void socketops::Bind(int sockfd, const sockaddr& address)
{
	int ret = ::bind(sockfd, &address, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
	assert(ret != -1);
}

int socketops::Connect(int sockfd, const sockaddr& address)
{
	return ::connect(sockfd, &address, static_cast<socklen_t>(sizeof address));
}

void socketops::Listen(int sockfd)
{
	::listen(sockfd, 5);
}

void socketops::SetReusePort(int sockfd)
{
#ifndef _WIN32
	int on = 1;
	int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &on,
		static_cast<socklen_t>(sizeof on));
	assert(ret != -1);
#endif
}

void socketops::SetReuseAddr(int sockfd)
{
#ifdef _WIN32
	char on = 1;
#else
	int on = 1;
#endif
	int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on,
		static_cast<socklen_t>(sizeof on));
	assert(ret != -1);
}

int socketops::Accept(int sockfd, struct sockaddr* address)
{

	socklen_t len = INET6_ADDRSTRLEN;
	int fd = accept(sockfd, address, &len);

	if (fd < 0)
	{
		LOG_FATAL << "accept error " << errno << " " << strerror(errno);
	}

	return fd;
}

void socketops::Close(int sockfd)
{
	::close(sockfd);
}

ssize_t socketops::Send(int sockfd, const char* data, size_t length)
{
	return send(sockfd, data, length, 0);
}

void socketops::ShutdownWrite(int sockfd)
{
	::shutdown(sockfd, SHUT_WR);
}

sockaddr_in6 socketops::GetPeerAddr(int sockfd)
{
	struct sockaddr_in6 peer_addr;
	socklen_t len = static_cast<socklen_t>(sizeof peer_addr);
	getpeername(sockfd, (sockaddr*)&peer_addr, &len);
	return peer_addr;
}

int socketops::InetPton(int af, const char* from, void* to)
{
	return ::inet_pton(af, from, to);
}

bool socketops::NameToAddr4(const std::string& name, in_addr* addr)
{
	if (InetPton(AF_INET, name.c_str(), addr) == 0)
	{
		hostent* host = gethostbyname(name.c_str());
		if (host)
		{
			addr->s_addr = *((uint32_t*)host->h_addr);
		}
		else
		{
			LOG_ERROR << "parse name: " << name << " error";
			return false;
		}
	}
	return true;
}

void socketops::NameToAddr6(const std::string& name, in6_addr* addr)
{

}
