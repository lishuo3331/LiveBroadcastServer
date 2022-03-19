//
// Created by rjd
//

#include "live/connection/PullConnection.h"

void PullConnection::SetCloseConnectionCallback(const ConnectionCallback& callback)
{
	close_connection_callback_ = callback;
}
