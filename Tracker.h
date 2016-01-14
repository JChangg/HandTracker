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



void setup(cv::VideoCapture& stream);
#endif