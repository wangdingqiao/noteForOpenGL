#ifndef _OPENGL_VIDEO_OBJECT_H_
#define _OPENGL_VIDEO_OBJECT_H_

// “˝»ÎOpenCVø‚  ”∆µ‰÷»æ
#include <opencv2/opencv.hpp>
#include <queue>

class VideoObject
{
public:
	VideoObject() :_width(400),_height(400){}
public:
	bool openVideo(const std::string& fileName, const int width, const int height)
	{
		_cap.open(fileName);
		if (!_cap.isOpened())  // check if we succeeded
		{
			std::cerr << "VideoRenderObject::Error, Could not open file: " << fileName << std::endl;
			return false;
		}
		_width = width;
		_height = height;
		return true;
	}

	double getFrameCount()
	{
		return _cap.get(CV_CAP_PROP_FRAME_COUNT);
	}

	double getFPS()
	{
		return _cap.get(CV_CAP_PROP_FPS);
	}

	bool getNextFrame(unsigned char* buffer, const int size)
	{
		cv::Mat frame;
		_cap >> frame;
		if (!frame.data)
		{
			return false;
		}
		cv::resize(frame, frame, cv::Size(_width, _height), 0, 0, cv::INTER_CUBIC);
		cv::cvtColor(frame, frame, CV_BGR2RGBA);
		std::memcpy(buffer, frame.data, size);
		return true;
	}
private:
	cv::VideoCapture _cap;
	int _width, _height;
};

#endif