#ifndef ANALYSIS_H
#define ANALYSIS_H
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<algorithm>
#include<list>
#include"Window.h"
#include"Settings.h"
#include"Exception.h"
#include"Log.h"

class HandAnalysis
{
private:

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
	void refine_contour();
	void finger_tips();
	void finger_tips2();
	void find_thumb();
	void update_roi(CenteredRect& bounds);
public:
	cv::Mat distTransform;	//distance transform of binary image.
	CenteredRect proposed_roi; //region as indicated by the meanshift algorithm. 
	vector<cv::Point> fingers;
	vector<double> finger_height;
	cv::Point center, wrist;
	double radius;
	int thumb_indx;
	cv::Point min_height, max_height;
	cv::Mat frame, prob, thresh;

	HandAnalysis();
	void apply(cv::Mat &frame, cv::Mat &probImg, CenteredRect &bounds);
	void show();

};



#endif	//!ANALYSIS_H


