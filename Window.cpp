#include "Window.h"

///////////////////////////////////////////////////////////////////////////////////////////////////


cv::Point CenteredRect::get_corner(cv::Point center, cv::Size dim)
{
	return center - ((cv::Point) dim / 2);
}


CenteredRect::CenteredRect() : cv::Rect() {}

CenteredRect::CenteredRect(cv::Point& center, cv::Size& dim) :
	cv::Rect(get_corner(center, dim), dim) {}

CenteredRect::CenteredRect(int x, int y, int width, int height) :
	cv::Rect(x, y, width, height) {}

CenteredRect::CenteredRect(cv::Point& tl, cv::Point& br) : cv::Rect(tl, br) {}


CenteredRect::CenteredRect(cv::Rect& rect) : cv::Rect(rect) {}


cv::Point CenteredRect::center() 
{
	return cv::Point(x + width / 2.0, y + height / 2.0);
}


void CenteredRect::enlarge(cv::Point pt, double factor)
{

	cv::Point topLeft = (tl() - pt) * factor + pt;
	cv::Size size = (cv::Size) ((br() - pt) * factor + pt - topLeft);

	x = topLeft.x;
	y = topLeft.y;

	height = size.height;
	width = size.width;


}





vector<cv::Point> randPoints(cv::Point tl, cv::Point br, int num)
{
	cv::RNG rng = cv::RNG();
	vector<cv::Point> pts = vector<cv::Point>();
	for (int i = 0; i < num; i++)
	{
		int x = rng.uniform(tl.x, br.x);
		int y = rng.uniform(tl.y, br.y);
		cv::Point pt = cv::Point(x, y);
		pts.push_back(pt);
	}
	return pts;
}

	
Window::Window(cv::Point& center, cv::Size& dim):center(center), outer(CenteredRect(center, dim))
{
	cv::Point inner_center = center + cv::Point(0, dim.height / 6.0 );
 	inner = CenteredRect(inner_center, cv::Size(dim.width * 0.5, dim.height / 3.0));
}

Window::Window(CenteredRect& inner, CenteredRect& outer) :inner(inner), outer(outer) {
	center = (outer.tl() + outer.br()) / 2;
}

void Window::draw(cv::Mat& img)
{
	
	cv::rectangle(img, outer, cv::Scalar(255, 0, 0), 1);
	cv::rectangle(img, inner, cv::Scalar(0, 255, 0), 1);
}



void Window::move(cv::Point delta)
{	
	inner += delta;
	outer += delta;
	center += delta;
}

void Window::scale(double factor)
{	
	inner.enlarge(center, factor);
	outer.enlarge(center, factor);
}

cv::MatND Window::histogram(cv::Mat img)
{
	cv::Rect imgDim = cv::Rect(0, 0, img.cols, img.rows);
	cv::Mat sample(img, (cv::Rect) outer & imgDim);
	cv::Rect imgInn = (cv::Rect) inner & imgDim;
	cv::Point newOrigin = outer.tl();
	cv::Point tl = imgInn.tl() - newOrigin;
	cv::Point br = imgInn.br() - newOrigin;
	cv::Mat mask = cv::Mat::zeros(sample.size(), CV_8UC1);
	for (cv::Point pt : randPoints(tl, br, 10))
	{
		cv::bitwise_or(flood_fill(sample, pt, 20, 24), mask, mask);
	}

	cv::MatND hist = get_hist(sample, mask);
	
	return hist;
}





