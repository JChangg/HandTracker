#ifndef HIST_H
#define HIST_H
#include<opencv2/core/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace std;

cv::Mat flood_fill(cv::Mat& src, cv::Point seed, int lb, int ub);
cv::MatND get_hist(cv::Mat& img, cv::Mat& mask);
cv::MatND get_hist_hsv(cv::Mat& img, cv::Mat& mask);
cv::MatND back_project(cv::Mat& hsv, cv::MatND& hist);

#endif