//
// Created by rjd
//

#ifndef LIVEBROADCASTSERVER_CONDITION_H
#define LIVEBROADCASTSERVER_CONDITION_H

#include "thread/Mutex.h"
class Condition
{
public:
	explicit Condition(Mutex& mutex);
	~Condition();

	void WakeUpOne();

	void WakeUpAll();

	void Wait();
private:

	Mutex& mutex_;

	pthread_cond_t cond_;
};


#endif //LIVEBROADCASTSERVER_CONDITION_H
