#include "Background.h"

BackgroundSubtractor::BackgroundSubtractor(){}

BackgroundSubtractor::BackgroundSubtractor(int history, double varThreshold)
{
	bg = cv::createBackgroundSubtractorMOG2(history, varThreshold, false);
}

void BackgroundSubtractor::apply_frame(cv::Mat& img, cv::Mat& foreground, double learningRate)
{
	bg->apply(img, foreground, learningRate);
}

void BackgroundSubtractor::apply_frame(cv::Mat& img, cv::Mat& foreground, double learningRate, cv::Rect exclude)
{
	if (learningRate != 0) {
		
		cv::Mat background, img2, mask;
		bg->getBackgroundImage(background);
		cv::Rect area = exclude & cv::Rect(0, 0, background.cols, background.rows);
		cv::Mat patch = background(area);
		mask = cv::Mat::zeros(img.rows, img.cols, CV_8U);
		mask(area) = 1;
		img.copyTo(img2);
		background.copyTo(img2, mask);
		bg->apply(img2, foreground, learningRate);
	} else bg->apply(img, foreground, learningRate);
}

void BackgroundSubtractor::apply_frame(cv::Mat& img, cv::Mat& foreground, double learningRate,  std::vector<cv::Point> contour)
{
	if (learningRate != 0) {
		std::vector<std::vector<cv::Point>> contours(1);
		contours[0] = contour;
		cv::Mat background, img2, mask;
		bg->getBackgroundImage(background);
		mask = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);
		cv::drawContours(mask, contours, 0, cv::Scalar(255), CV_FILLED);
		img.copyTo(img2);
		background.copyTo(img2, mask);
		bg->apply(img2, foreground, learningRate);
	}
	else bg->apply(img, foreground, learningRate);
}

void BackgroundSubtractor::apply_frame(cv::Mat& img, cv::Mat& foreground, double learningRate, cv::Mat mask)
{
	if (learningRate != 0)
	{
		cv::Mat background, img2;
		bg->getBackgroundImage(background);
		img.copyTo(img2);
		background.copyTo(img2, mask);
		bg->apply(img2, foreground, learningRate);
	}
	else bg->apply(img, foreground, learningRate);
}

void BackgroundSubtractor::getBackground(cv::Mat& background)
{
	bg->getBackgroundImage(background);
}


BackgroundSubtractor::~BackgroundSubtractor() { bg.release(); }
