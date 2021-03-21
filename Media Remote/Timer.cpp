#include <windows.h>
#include "Timer.h"

CTimer::CTimer()
{
	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);
	Seconds = 100;
	StartTime = EndTime = 0;
}


CTimer::CTimer(unsigned int Secs)
{
	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);
	if(Secs > 0)
		Seconds = Secs;
	else
		Seconds = 100;
	StartTime = EndTime = 0;
}




void CTimer::SetStartTime()
{
	QueryPerformanceCounter((LARGE_INTEGER *)&StartTime);
}


void CTimer::SetStartTime(__int64 Time)
{
	StartTime = Time;
}


void CTimer::SetEndTime()
{
	QueryPerformanceCounter((LARGE_INTEGER *)&EndTime);
}


void CTimer::SetSeconds(unsigned int Secs)
{
	if(Secs > 0)
		Seconds = Secs;
}


void CTimer::SetMaxSeconds()
{
	Seconds = 4294967295;
}


__int64 CTimer::GetStartTime()
{
	return StartTime;
}


__int64 CTimer::GetEndTime()
{
	return EndTime;
}


__int64 CTimer::GetDifference()
{
	return EndTime-StartTime;
}


unsigned int CTimer::GetSeconds()
{
	return Seconds;
}


__int64 CTimer::CalculateNumberSeconds()
{
	return Frequency*Seconds;
}