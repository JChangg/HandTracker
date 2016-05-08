#include "Hist.h"


cv::Mat flood_fill(cv::Mat& src, cv::Point seed, int lb=20, int ub=20)
{
	int newMaskVal = 255; 
	cv::Scalar newVal = cv::Scalar(120, 120, 120);

	int connectivity = 8;
	int flags = connectivity + (newMaskVal << 8) + cv::FLOODFILL_FIXED_RANGE + cv::FLOODFILL_MASK_ONLY;

	cv::Mat mask2 = cv::Mat::zeros(src.rows + 2, src.cols + 2, CV_8UC1);
	cv::floodFill(src, mask2, seed, newVal, 0, cv::Scalar(lb, lb, lb), cv::Scalar(ub, ub, ub), flags);
	cv::Mat mask = mask2(cv::Range(1, mask2.rows - 1), cv::Range(1, mask2.cols - 1));

	return mask;
}




cv::MatND get_hist(cv::Mat& img, cv::Mat& mask)
{
	cv::MatND hist;
	cv::Mat hsv;
	cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);

	int h_bins = 30; int s_bins = 32;
	int histSize[] = { h_bins, s_bins };

	float h_range[] = { 0, 179 };
	float s_range[] = { 0, 255 };
	const float* ranges[] = { h_range, s_range };

	int channels[] = { 0, 1 };

	/// Get the Histogram and normalize it
	calcHist(&hsv, 1, channels, mask, hist, 2, histSize, ranges, true, false);

	normalize(hist, hist, 0, 255, cv::NORM_MINMAX, -1, cv::Mat());

	return hist;
}

cv::MatND get_hist_hsv(cv::Mat & hsv, cv::Mat & mask)
{
	cv::MatND hist;
	int h_bins = 30; int s_bins = 32;
	int histSize[] = { h_bins, s_bins };

	float h_range[] = { 0, 179 };
	float s_range[] = { 0, 255 };
	const float* ranges[] = { h_range, s_range };

	int channels[] = { 0, 1 };

	/// Get the Histogram and normalize it
	calcHist(&hsv, 1, channels, mask, hist, 2, histSize, ranges, true, false);

	normalize(hist, hist, 0, 255, cv::NORM_MINMAX, -1, cv::Mat());

	return hist;
}







cv::MatND back_project(cv::Mat& hsv, cv::MatND& hist)
{
	int channels[] = { 0, 1 };
	float h_range[] = { 0, 179 };
	float s_range[] = { 0, 255 };
	const float* ranges[] = { h_range, s_range };
	cv::MatND backproj;
	cv::calcBackProject(&hsv, 1, channels, hist, backproj, ranges, 1, true);
	cv::normalize(backproj, backproj, 0, 255, cv::NORM_MINMAX, CV_8UC1);
	return backproj;
}