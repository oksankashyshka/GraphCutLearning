//accurate timer
#ifndef _PROF_TIMER_HPP_
#define _PROF_TIMER_HPP_

#include <windows.h>

class ProfTimer
{
	LARGE_INTEGER mTimeStart,
		mTimeChecked,
		Frequency;
	double TimeElapsed;
public:
	ProfTimer() { QueryPerformanceFrequency(&Frequency); }
	//start working with timer
	void start() { QueryPerformanceCounter(&mTimeStart); }
	//finish checking time
	double check()
	{
		QueryPerformanceCounter(&mTimeChecked);
		return TimeElapsed = (double)(mTimeChecked.QuadPart - mTimeStart.QuadPart) /
														(double)Frequency.QuadPart;
	}
	//ger duration in msec between start() and check()
	double getDur() const { return TimeElapsed; }
};

#endif//_PROF_TIMER_HPP_
