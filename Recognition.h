#ifndef RECOGN_H
#define RECOGN_H

#include<opencv2\core.hpp>
#include<vector>
#include<iostream>
#include<math.h>
#include<string>
#include"Analysis.h"
#include"Demo.h"



enum StaticState
{
	OPEN, CLOSED, SCROLL, PINCH, POINTER
};

enum MoveState
{
	MOVE, STATION, START
};



class StateClassifier
{
private:
	HandAnalysis hand_analyser;
	cv::Point center;
	double radius;
	double current_value;
	double original_value;
	inline bool update_center();
	void update_move_state();
public:
	StaticState stat;
	MoveState dynamic;
	StateClassifier();
	StateClassifier(HandAnalysis &h);
	void apply(HandAnalysis& hand);
	StaticState getStaticState();
	void printState();

};

#endif // !RECOGN_H
