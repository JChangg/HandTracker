#include "Shape.h"
using namespace std;


void showimgcontours(cv::Mat &threshedimg, cv::Mat &original)
{
	vector<vector<cv::Point> > contours;
	vector<cv::Vec4i> hierarchy;
	int largest_area = 0;
	int largest_contour_index = 0;
	cv::findContours(threshedimg, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	//this will find largest contour
	for (int i = 0; i< contours.size(); i++) // iterate through each contour. 
	{
		double a = contourArea(contours[i], false);  //  Find the area of contour
		if (a>largest_area)
		{
			largest_area = a;
			largest_contour_index = i;                //Store the index of largest contour
		}

	}
	//search for largest contour has end

	if (contours.size() > 0)
	{
		drawContours(original, contours, largest_contour_index, CV_RGB(0, 255, 0), 2, 8, hierarchy);
		//if want to show all contours use below one
		//drawContours(original,contours,-1, CV_RGB(0, 255, 0), 2, 8, hierarchy);
	}
}
