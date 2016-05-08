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
#include"Analysis.h"
#include"Background.h"
#include"Recognition.h"
#include"Settings.h"
#include"Log.h"

class Tracker
{
private:
	CenteredRect tracking_window;
	cv::RotatedRect tracking_window_fitted;
	HandAnalysis hand_processer; 
	//StateClassifier classifier;
	HMMClassifier classifier;
public:
	BackgroundSubtractor bg;
	cv::MatND hist_model;
	cv::MatND hist_model_original;
	Tracker();

	Tracker(CenteredRect& initial_window, cv::MatND& hist_model,
		BackgroundSubtractor& bg);

	void set_window(CenteredRect& window);


	void process_frame(cv::Mat& input_BGR, cv::Mat& input_HSV, cv::Mat& output_foreground,
		cv::Mat& output_backproj, cv::Mat& output);


};



#endif