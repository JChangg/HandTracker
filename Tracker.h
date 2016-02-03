#ifndef TRACKER_H
#define TRACKER_H
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/video/tracking.hpp>
#include<opencv2/video/background_segm.hpp>
#include<stdexcept>
#include<iostream>
#include<string>
#include<vector>
#include"Hist.h"
#include"Window.h"
#include"Contour.h"
#include"Background.h"
#include "Recognition.h"

class Tracker
{
private:
	CenteredRect tracking_window;
	cv::RotatedRect tracking_window_fitted;
	Analysis hand_processer = Analysis();
	Classifier classifier = Classifier();
public:
	double alpha, beta;
	BackgroundSubtractor bg;
	cv::MatND hist_model;
	Tracker();

	Tracker(CenteredRect& initial_window, cv::MatND& hist_model,
		BackgroundSubtractor& bg = BackgroundSubtractor(1000, 16),
		double alpha = 0.7);

	void set_window(CenteredRect& window);

	void set_alpha(double alpha);

	void process_frame(cv::Mat& input_BGR, cv::Mat& input_HSV, cv::Mat& output_foreground,
		cv::Mat& output_backproj, cv::Mat& output);
};


cv::Mat threshold(cv::Mat& src, CenteredRect& mask);

#endif