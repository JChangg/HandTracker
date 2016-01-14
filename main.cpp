#include "Tracker.h"

#define LEFT_KEY 2424832
#define RIGHT_KEY 2555904
#define UP_KEY 2490368
#define DOWN_KEY 2621440

int main()
{
	cv::VideoCapture capWebcam(1);		// declare a VideoCapture object and associate to webcam, 0 => use 1st webcam

	if (!capWebcam.isOpened()) {			// check if VideoCapture object was associated to webcam successfully
		std::cout << "error: capWebcam not accessed successfully\n\n";	// if not, print error message to std out
		return(0);														// and exit program
	}

	cv::Mat imgOriginal;		// input image
								
	setup(capWebcam);
	std::system("pause");
	return 0;
}

