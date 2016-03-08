#ifndef BACKGROUND_H
#define BACKGROUND_H
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/video/background_segm.hpp>
#include<vector>

class BackgroundSubtractor
{
public:
	BackgroundSubtractor();
	BackgroundSubtractor(int history, double varThreshold);
	void apply_frame(cv::Mat& img, cv::Mat& foreground, double learningRate);
	void apply_frame(cv::Mat& img, cv::Mat& foreground, double learningRate, cv::Mat mask);
	void apply_frame(cv::Mat& img, cv::Mat& foreground, double learningRate, cv::Rect exclude);
	void apply_frame(cv::Mat& img, cv::Mat& foreground, double learningRate, std::vector<cv::Point> contour);

	void getBackground(cv::Mat& background);

	~BackgroundSubtractor();

private:
	cv::Ptr<cv::BackgroundSubtractor> bg;
};



#endif // !BACKGROUND_H
