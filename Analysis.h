#ifndef ANALYSIS_H
#define ANALYSIS_H
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include"Window.h"
#include<algorithm>
#include<list>

class Analysis
{
private:
	CenteredRect bounding_box;
	cv::RotatedRect proposed_roi;
	vector<cv::Point> hull;
	cv::Mat frame;
	cv::Mat threshedimg;
	vector<cv::Vec4i> defects;

	cv::Point find_center();
	cv::Point find_center(cv::Rect& region);

	void threshold(cv::Mat& probImg);
	void threshold(cv::Mat& probImg, CenteredRect& mask);
	void handStructure();
	void condefects(vector<cv::Vec4i> convexityDefectsSet);

public:
	vector<cv::Point> contour;
	vector<cv::Point> fingers;

	cv::RotatedRect roi;
	cv::Point center;
	double radius;

	Analysis();

	void apply(cv::Mat &frame, cv::Mat &probImg, CenteredRect &bounds, cv::RotatedRect &roi);
	void display();

};


#endif	//!ANALYSIS_H


