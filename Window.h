#ifndef WINDOW_H
#define WINDOW_H
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<stdexcept>
#include<iostream>
#include<string>
#include<vector>
#include"Hist.h"

using namespace std;


class FailedToRead : public std::exception
{
public:
	FailedToRead(const char * _Message) : std::exception(_Message) {}
};



class CenteredRect : public cv::Rect
{

private: 
	static cv::Point get_corner(cv::Point center, cv::Size dim);
public:
	CenteredRect();
	CenteredRect(int x, int y, int width, int height);
	CenteredRect(cv::Point& center, cv::Size& dim);
	CenteredRect(cv::Point& tl, cv::Point& br);
	CenteredRect(cv::Rect& rect);

	void enlarge(cv::Point pt, double factor);
	cv::Point center();

};



class Window
{
public:
	CenteredRect inner, outer;
	cv::Point center;

	Window(cv::Point& center, cv::Size& dim);
	Window(CenteredRect& inner, CenteredRect& outer);

	void draw(cv::Mat& img);
	

	void move(cv::Point delta);

	void scale(double factor);

	cv::MatND histogram(cv::Mat img);
};


#endif
