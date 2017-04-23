#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

void displayImage();
void displayCamera();
void displayVideo();

int main(int argc, char** argv)
{
	displayImage();
	displayVideo();

	cv::waitKey(0);                                          // Wait for a keystroke in the window
	return 0;
}

void displayImage()
{
	std::string imagePath = "../../resources/textures/dog.jpg";
	cv::Mat image;
	image = cv::imread(imagePath, CV_LOAD_IMAGE_COLOR);   // Read the file

	if (!image.data)                              // Check for invalid input
	{
		std::cout << "Could not open or find the image: " << imagePath << std::endl;
		char waitKey;
		std::cin >> waitKey;
		return;
	}

	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
	cv::imshow("Display window", image);                   // Show our image inside it.
}
void displayCamera()
{
	cv::VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
	{
		std::cerr << "Error, failed to open camera!" << std::endl;
		char waitKey;
		std::cin >> waitKey;
		return;
	}

	cv::Mat edges;
	cv::namedWindow("camera", 1);
	for (;;)
	{
		cv::Mat frame;
		cap >> frame; // get a new frame from camera
		cvtColor(frame, edges, CV_BGR2GRAY);
		imshow("camera", edges);
		if (cv::waitKey(30) >= 0) break;
	}
}

void displayVideo()
{
	const std::string fileName = "../../resources/videos/traffic.mp4";
	cv::VideoCapture cap(fileName); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
	{
		std::cerr << "Error, Could not open file: " << fileName << std::endl;
		char waitKey;
		std::cin >> waitKey;
		return;
	}
	std::cout << "Total frame count: " << cap.get(CV_CAP_PROP_FRAME_COUNT) << std::endl;

	double  fps = cap.get(CV_CAP_PROP_FPS);
	std::cout << "Frame rate: " << fps << std::endl;

	cv::Mat edges;
	cv::namedWindow("PlayVedio", 1);
	for (;;)
	{
		cv::Mat frame;
		cap >> frame; // get a new frame from camera
		if (!frame.data) break;
		cv::resize(frame, frame, cv::Size(640, 480), 0, 0, cv::INTER_CUBIC);
		cv::cvtColor(frame, edges, CV_BGR2GRAY);
		cv::imshow("PlayVedio", frame);
		if (cv::waitKey(50) >= 0) break;
	}
}