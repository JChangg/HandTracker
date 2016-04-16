#ifndef DEMO_H
#define DEMO_H
#include "Constants.h"
#include <GL/glut.h>
#include <math.h>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <string>
#include <iostream>
using namespace STATE;

namespace graphics
{

	void setup(int argc, char ** argv);

	struct cvParams
	{
		cv::Point center, prev_max, prev_min, max, min;
		cvParams();
		cvParams(cv::Point center, cv::Point prev_max, cv::Point prev_min, cv::Point max, cv::Point min);
	};

	void updateParams(StaticState newState, cvParams param);
};



#endif // ! DEMO_H
