#ifndef ANALYSIS_H
#define ANALYSIS_H
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include"Window.h"
#include<algorithm>
#include<list>


class HandAnalysis
{
private:
	cv::Mat distTransform;	//distance transform of binary image.

	CenteredRect bounding_box; //bounding box.
	vector<cv::Point> hull, contour; 
	vector<int> hull_indices;
	vector<cv::Vec4i> defects; // this is the indices of the defect points. 
	vector<double> finger_dist;

	void threshold();
	void max_contour();
	void max_hull();
	void find_center();
	void find_wrist();
	void find_center_orientation();
	void finger_tips();
	void finger_tips2();
public:
	CenteredRect proposed_roi; //region as indicated by the meanshift algorithm. 
	vector<cv::Point> fingers;
	vector<double> finger_height;
	cv::Point center, wrist;
	double radius;
	cv::Mat frame, prob, thresh;

	HandAnalysis();
	void apply(cv::Mat &frame, cv::Mat &probImg, CenteredRect &bounds);
	void show();
};



#endif	//!ANALYSIS_H


