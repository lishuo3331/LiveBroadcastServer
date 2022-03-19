//
// Created by rjd
//
#include <iostream>
#include <unistd.h>
#include "thread/ThreadPool.h"

int times = 0;
Mutex mutex;

void Func()
{
	MutexGuardLock lock(mutex);
	std::cout << times++ << std::endl;
	sleep(2);
}

int main()
{
	ThreadPool pool("UserMapper thread pool");

	pool.SetThreadNum(5);

	pool.Start();

	pool.Run(Func);
	pool.Run(Func);
	pool.Run(Func);

	pool.Stop();
}