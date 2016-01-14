#include "Background.h"

void bgsf()
{
	cv::Mat src, tgt, hsv;
	cv::VideoCapture capWebcam(1);
	int k = 0;
	cv::Ptr<cv::BackgroundSubtractor> bg = cv::createBackgroundSubtractorMOG2(2000, 16, false);
	while (capWebcam.isOpened() && k != 27) {
		capWebcam.read(src);
		cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
		cv::flip(src, src, 1);
		bg->apply(src, tgt, -1);
		cv::GaussianBlur(tgt, tgt, cv::Size(5, 5), 0, 0);
		cv::imshow("s", src);
		cv::imshow("bg", tgt);
		k = cv::waitKey(1);
	}
}