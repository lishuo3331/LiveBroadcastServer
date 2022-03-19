//
// Created by rjd
//

#include <cassert>
#include "utils/Logger.h"
#include "thread/Thread.h"

void* ThreadAttr::ThreadFunction(void* arg)
{
	if (arg)
	{
		ThreadAttr* attr = static_cast<ThreadAttr*>(arg);
		if (attr)
		{
			LOG_INFO << "thread: " << attr->name << "started";
			attr->func();
			LOG_INFO << "thread: " << attr->name << "stopped";
		}
	}
	return nullptr;
}

Thread::Thread(const ThreadFunc& func, const std::string& name):
	started_(false),
	joined_(false),
	attr_({func, name})
{

}

Thread::~Thread()
{
	if (started_ && !joined_)
	{
		pthread_detach(thread_);
	}
}

void Thread::Start()
{
	assert(!started_);
	started_ = true;

	if (pthread_create(&thread_, nullptr, ThreadAttr::ThreadFunction, &attr_) == 0)
	{

	}
	else
	{
		started_ = false;
		LOG_FATAL << "start thread failed";
	}
}

int Thread::Join()
{
	assert(started_ && !joined_);
	joined_ = true;
	return pthread_join(thread_, nullptr);
}
