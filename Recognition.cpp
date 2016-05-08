#include "Recognition.h"

inline float dist(cv::Point& p, cv::Point& q)
{
	return cv::norm(p - q);
}

inline bool StateClassifier::update_center()
{
	double threshold;
	if (dynamic == MoveState::MOVE) threshold = 0.6;
	else threshold = 0.2;
	bool station = (dist(hand_analyser.center, this->center) < hand_analyser.radius * threshold);
	if (!station) this->center = hand_analyser.center;
	return station;
}


void StateClassifier::update_move_state()
{
	bool isStationary = update_center();
	switch (dynamic)
	{
	case MoveState::MOVE:
		if (isStationary) dynamic = MoveState::STATION;
		else dynamic = MoveState::MOVE;
		break;
	case MoveState::STATION:
		if (isStationary) dynamic = MoveState::STATION;
		else dynamic = MoveState::START;
		break;
	case MoveState::START:
		if (isStationary) dynamic = MoveState::STATION;
		else dynamic = MoveState::MOVE;
		break;
	}
}




StateClassifier::StateClassifier()
{
	center = cv::Point();
	radius = 1.0;
	stat = StaticState::OPEN;
	dynamic = MoveState::MOVE;
	//training.open("test2.txt", std::ios_base::app);
	
}

StateClassifier::StateClassifier(HandAnalysis & h): hand_analyser(h)
{
	this->center = hand_analyser.center;
	this->radius = hand_analyser.radius;
	this->stat = StaticState::OPEN;
	this->dynamic = MoveState::MOVE;
	//training.open("test2.txt", std::ios_base::app);
}

void StateClassifier::apply(HandAnalysis& hand)
{
	hand_analyser = hand;
	static cv::Point prev_center;
	update_move_state();
	StaticState prev_state = stat;
	cv::Point prev_max = max;
	cv::Point prev_min = min;
	if (dynamic == MoveState::STATION)
	{
		stat = getStaticState();
	} 

	graphics::cvParams param = graphics::cvParams(center, prev_max, prev_min, max, min);

	switch (prev_state)
	{
	case StaticState::OPEN:
		if (stat == StaticState::POINTER)
		{/*
			prev_center = center;
			training<< "Emission: " << std::endl;*/
		}
		break;
	case StaticState::CLOSED:
		if (stat != StaticState::CLOSED)
			stat = StaticState::OPEN;
		break;
	case StaticState::SCROLL:
		if (stat != StaticState::SCROLL)
			stat = StaticState::OPEN;
		break;
	case StaticState::PINCH:
		if (stat != StaticState::PINCH)
			stat = StaticState::OPEN;
		break;
	case StaticState::POINTER:
		if (stat != StaticState::POINTER)
		{
			stat = StaticState::OPEN;
		}/*
		else {
			static int i = 0;
			int ori = orientation(center - prev_center);
			prev_center = center;
			if (i % 2 == 0 && ori  != 0)
			{
				training << ori << std::endl;;
			}

		}*/
		break;
	}
	graphics::updateParams(stat, param);

	
}

StaticState StateClassifier::getStaticState()
{
	if (hand_analyser.fingers.size() >= 4)
	{
		return StaticState::OPEN;
	}

	else if (hand_analyser.fingers.size() == 0)
	{
		return StaticState::CLOSED;
	}

	else if (hand_analyser.fingers.size() == 1)
	{
		max = hand_analyser.fingers[0];
		return StaticState::POINTER;
	}

	else if (hand_analyser.fingers.size() == 2)
	{
		double fheight1 = hand_analyser.finger_height[0];
		double fheight2 = hand_analyser.finger_height[1];
		max = fheight1 > fheight2 ? hand_analyser.fingers[0]
			: hand_analyser.fingers[1];
		if (fheight1 > fheight2)
		{
			max = hand_analyser.fingers[0];
			min = hand_analyser.fingers[1];
		}
		else
		{
			max = hand_analyser.fingers[1];
			min = hand_analyser.fingers[0];
		}
		if (abs(fheight1 - fheight2) > 0.5 * hand_analyser.radius)
			return StaticState::PINCH;
		
		else return StaticState::SCROLL;
		
	}
	else if (hand_analyser.fingers.size() == 3)
	{
		double minheight = hand_analyser.finger_height[0], 
			maxheight = hand_analyser.finger_height[0];
		int min_index = 0, max_index = 0;
		if (hand_analyser.finger_height[1] < minheight)
		{
			minheight = hand_analyser.finger_height[1];
			min_index = 1;
		}
		else
		{
			maxheight = hand_analyser.finger_height[1];
			max_index = 1;
		}

		if (hand_analyser.finger_height[2] < minheight)
		{
			minheight = hand_analyser.finger_height[2];
			min_index = 2;
		}
		else
		{
			maxheight = hand_analyser.finger_height[2];
			max_index = 2;
		}

		max = hand_analyser.fingers[max_index];
		min = hand_analyser.fingers[min_index];
		if (maxheight - minheight > 0.5 * hand_analyser.radius)
		{
			return StaticState::PINCH;
		}
		else return StaticState::SCROLL;

	}
}


std::string StateClassifier::str()
{
	std::string s = staticStateString(stat);
	std::string m = dynamicStateString(dynamic);
	return "(" + s + ", " + m + ")";
}




const cv::Mat prior = (cv::Mat_<double>(5, 1) << 
	0, 
	0, 
	0, 
	0, 
	1);

const cv::Mat transition = (cv::Mat_<double>(5, 5) << 
	0.05, 0.0, 0.0, 0.0, 0.95,
	0.05, 0.95, 0.0, 0.0, 0.0,
	0.05, 0.0, 0.95, 0.0, 0.0,
	0.05, 0.0, 0.0, 0.95, 0.0,
	0.014, 0.008, 0.014, 0.014, 0.95);

const cv::Mat emission = (cv::Mat_<double>(5, 8) << 
	1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
	0.0, 0.1, 0.7, 0.2, 0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.2, 0.0, 0.0, 0.0, 0.6, 0.2,
	0, 0.05, 0.05, 0.05, 0.2, 0.6, 0.05, 0.05);





inline bool HMMClassifier::update_center()
{
	bool station = (dist(hand_analyser.center, this->center) < hand_analyser.radius * 0.1);
	if (station) this->center = (hand_analyser.center + this->center) / 2;
	else this->center = hand_analyser.center;
	return station;
}

void HMMClassifier::update_move_state()
{
	bool isStationary = update_center();
	switch (dynamic)
	{
	case MoveState::MOVE:
		if (isStationary) dynamic = MoveState::STATION;
		else dynamic = MoveState::MOVE;
		break;
	case MoveState::STATION:
		if (isStationary) dynamic = MoveState::STATION;
		else dynamic = MoveState::START;
		break;
	case MoveState::START:
		if (isStationary) dynamic = MoveState::STATION;
		else dynamic = MoveState::MOVE;
		break;
	}
}




HMMClassifier::HMMClassifier()
	:center(cv::Point()), stat(StaticState::OPEN), dynamic(MoveState::MOVE),
	machine(HMM(prior, transition, emission)), prob(prior)
{
	radius = 1.0;
}

HMMClassifier::HMMClassifier(HandAnalysis & h) : hand_analyser(h), 
	center(hand_analyser.center), stat(StaticState::OPEN), dynamic(MoveState::MOVE),
	machine(HMM(prior, transition, emission)), prob(prior)
{
	radius = hand_analyser.radius;
}

void HMMClassifier::apply(HandAnalysis& hand)
{	
	hand_analyser = hand;
	radius = hand_analyser.radius;
	update_move_state();
	if (dynamic != MoveState::MOVE)
	{

		int num = hand_analyser.fingers.size();
		bool thumb = hand_analyser.thumb_indx < 0;
		int emission = 0;

		switch (num)
		{
		case 0:
			emission = 0;
			break;
		case 1:
			emission = 1;
			break;
		case 2:
			if (thumb) emission = 6;
			else emission = 2;
			break;
		case 3:
			if (thumb) emission = 7;
			else emission = 3;
			break;
		case 4:
			emission = 4;
			break;
		case 5:
			emission = 5;
			break;
		default:
			emission = 5;
			break;
		}

		prob = machine.filter(prob, emission);
		double min = 0, max = 0; cv::Point pt(0);
		cv::minMaxLoc(prob, &min, &max, &cv::Point(0), &pt);
		//cout << prob << endl;
		if (max > 0.95)
		{
			stat = (StaticState)pt.y;
			graphics::cvParams param = graphics::cvParams();
			param.center = center;
			param.prev_min = min_point;
			param.prev_max = max_point;
			min_point = hand_analyser.min_height;
			max_point = hand_analyser.max_height;
			param.min = min_point;
			param.max = max_point;
			param.radius = radius;
			graphics::updateParams(stat, param);
		}
	}
	else
	{
		graphics::cvParams param = graphics::cvParams();
		param.center = center;
		param.prev_min = min_point;
		param.prev_max = max_point;
		min_point = hand_analyser.min_height;
		max_point = hand_analyser.max_height;
		param.min = min_point;
		param.max = max_point;
		param.radius = radius;
		graphics::updateParams(stat, param);
	}
}

std::string HMMClassifier::str()
{
	std::string s = staticStateString(stat);
	std::string m = dynamicStateString(dynamic);
	return "(" + s + ", " + m + ")";
}
std::string HMMClassifier::str2()
{
	return staticStateString(stat);
}
