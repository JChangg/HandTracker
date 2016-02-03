#ifndef ROT_H
#define ROT_H
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include"Window.h"
#include<algorithm>
#include<list>

class Analysis
{
private:
	vector<cv::Point> hull;
	cv::RotatedRect proposed_roi;
	cv::Mat frame;
	cv::Mat threshedimg;

	cv::Point Analysis::find_center(cv::Point proposed_center, int width, int delta, int delta2);

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


#endif // !ROT_H


