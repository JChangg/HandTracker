#ifndef RECOGN_H
#define RECOGN_H
#include "Constants.h"

#include<opencv2\core.hpp>
#include<vector>
#include<iostream>
#include<math.h>
#include<string>
#include<fstream>
#include"Analysis.h"
#include"Demo.h"
#include"HMM.h"
using namespace STATE;


class StateClassifier
{
private:
	HandAnalysis hand_analyser;
	cv::Point center; 
	cv::Point max, min;
	double radius;
	inline bool update_center();
	void update_move_state();
public:
	StaticState stat;
	MoveState dynamic;
	StateClassifier();
	StateClassifier(HandAnalysis &h);
	void apply(HandAnalysis& hand);
	StaticState getStaticState();
	std::string str();
};


class HMMClassifier
{
private:
	HandAnalysis hand_analyser;
	cv::Point center;
	cv::Point max_point, min_point;
	double radius;
	HMM machine; 
	cv::Mat prob;
	inline bool update_center();
	void update_move_state();
public:
	StaticState stat;
	MoveState dynamic;
	HMMClassifier();
	HMMClassifier(HandAnalysis &h);
	void apply(HandAnalysis& hand);
	std::string str();
	std::string str2();
};





#endif // !RECOGN_H
