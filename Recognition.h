#ifndef RECOGN_H
#define RECOGN_H

#include<opencv2\core.hpp>
#include<vector>
#include<iostream>
#include<math.h>
#include<string>
#include"Graphics.h"

enum State
{
	OPEN_HAND, MOVE_OPEN_HAND, CLOSED_HAND, MOVE_CLOSED_HAND
};




class Classifier
{
private:
	std::vector<cv::Point> tips;
	double radius;
	inline bool updateCenter(cv::Point center);

public:
	State current;
	cv::Point center;

	Classifier();


	void apply(std::vector<cv::Point> tips, cv::Point center, double radius);
		
	void printState();
};

#endif // !RECOGN_H
