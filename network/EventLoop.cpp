#include "network/EventLoop.h"
#include "network/Channel.h"
#include "network/multiplexing/MultiplexingBase.h"
#include "network/multiplexing/Epoll.h"
#include "network/multiplexing/Select.h"
#include "thread/CurrentThread.h"

EventLoop::EventLoop() noexcept :
	multiplexing_base_(nullptr),
	active_channels_(),
	looping_(false),
	thread_tid_(CurrentThread::GetTid()),
	pending_func_mutex_()
{
#ifdef _WIN32
	multiplexing_base_ = new Select;
#else
	multiplexing_base_ = new Epoll;
#endif
}

EventLoop::~EventLoop()
{
	delete multiplexing_base_;
}

void EventLoop::Loop()
{
	looping_ = true;

	while (looping_)
	{
		active_channels_.clear();

		multiplexing_base_->LoopOnce(20, &active_channels_);

		HandleActiveChannel();
		HandlePendingFunc();
	}
}

void EventLoop::Stop()
{
	looping_ = false;
}

void EventLoop::Update(Channel* channel)
{
	multiplexing_base_->UpdateChannel(channel);
}

void EventLoop::HandleActiveChannel()
{

	for (Channel* channel : active_channels_)
	{
		channel->HandleEventWithGuard();
	}
}

void EventLoop::RunInLoop(const EventLoop::EventLoopFunction& function)
{
	{
		MutexLockGuard guard(pending_func_mutex_);
		pending_func_.push_back(function);
	}
}

bool EventLoop::IsInLoopThread() const
{
	return CurrentThread::GetTid() == thread_tid_;
}

void EventLoop::HandlePendingFunc()
{
	std::vector<EventLoopFunction> pending_func;
	{
		/**
		 * 减少锁持有的时间
		 */
		MutexLockGuard guard(pending_func_mutex_);
		pending_func.swap(pending_func_);
	}

	for (auto& func : pending_func)
	{
		func();
	}
}
