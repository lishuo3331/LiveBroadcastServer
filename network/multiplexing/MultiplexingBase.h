//
// Created by rjd
//

#ifndef LIVEBROADCASTSERVER_MULTIPLEXINGBASE_H
#define LIVEBROADCASTSERVER_MULTIPLEXINGBASE_H

#include "network/Callback.h"

class MultiplexingBase
{
public:

	MultiplexingBase();
	virtual ~MultiplexingBase();

	virtual void LoopOnce(int timeout, ChannelVector* active_channels);

	virtual void UpdateChannel(Channel *channel);
};


#endif //LIVEBROADCASTSERVER_MULTIPLEXINGBASE_H
