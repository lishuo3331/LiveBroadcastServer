//
// Created by rjd
//

#ifndef LIVEBROADCASTSERVER_MUTEX_H
#define LIVEBROADCASTSERVER_MUTEX_H

#include <pthread.h>
class Mutex
{
public:
	Mutex();
	~Mutex();

	void Lock();

	void Unlock();

	pthread_mutex_t* GetPthreadMutex();
private:
	pthread_mutex_t mutex_;
};

class MutexLockGuard
{
public:
	explicit MutexLockGuard(Mutex& mutex):
		mutex_(mutex)
	{
		mutex.Lock();
	}

	~MutexLockGuard()
	{
		mutex_.Unlock();
	}

private:
	Mutex& mutex_;
};

#define MutexLockGuard(x) error "missing guard object name"
#endif //LIVEBROADCASTSERVER_MUTEX_H
