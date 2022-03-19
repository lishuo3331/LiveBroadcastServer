#ifndef _WIN32

#ifndef NETWORK_EPOLL_H
#define NETWORK_EPOLL_H

#include <sys/epoll.h>

#include "network/multiplexing/MultiplexingBase.h"

class Epoll : public MultiplexingBase
{
public:
	typedef std::vector<epoll_event> EventVector;
	Epoll();

	void LoopOnce(int timeout, ChannelVector* active_channels) override;

	void UpdateChannel(Channel *channel) override;

private:
	int epfd_;

	EventVector event_vector_;

	void FillActiveSocketVector(int nums, ChannelVector* active_channels);

	void EpollControl(int ctl, int fd, void* ptr, uint32_t ev);

	void RemoveChannel(Channel* channel);

	void UpdateChannelInternal(int ctl, Channel* channel);
};
#endif // !NETWORK_EPOLL_H
#endif // !WIN32