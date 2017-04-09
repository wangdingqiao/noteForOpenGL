#ifndef _FRAME_RATE_COUNTER_H_
#define _FRAME_RATE_COUNTER_H_

#include <string>
#include <sstream>

class FrameRateCounter
{
public:
	FrameRateCounter(double frameStarTime) 
		:lastFrameTime(frameStarTime), frames(0){}
	double getMilliSecondsPerFrame()
	{
		return 1000.0 / frames;
	}
	int getFramePerSeconds()
	{
		return frames;
	}
	std::wstring getFramePerSecondsAsString()
	{
		std::wstringstream ss;
		ss << "FPS: " << frames;
		return ss.str();
	}
public:
	void resetCounter(double frameTime, int framesCount=0)
	{
		this->lastFrameTime = frameTime;
		this->frames = framesCount;
	}
public:
	double lastFrameTime;
	int frames;
};


#endif