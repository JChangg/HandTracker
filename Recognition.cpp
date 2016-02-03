#include "Recognition.h"


inline float dist(cv::Point& p, cv::Point& q)
{
	return cv::norm(p - q);
}


inline bool Classifier::updateCenter(cv::Point center)
{
	bool station = (dist(center, this->center) < radius * 0.2);
	this->center = (center + this->center) / 2;

	return station;
}

Classifier::Classifier()
{
	current = MOVE_OPEN_HAND;
}

void Classifier::apply(std::vector<cv::Point> tips, cv::Point center, double radius)
{
	this->radius = radius;

	bool closed = tips.size() <= 1;
	bool stationary = updateCenter(center);



	switch (current)
	{
	case OPEN_HAND:
		if (!stationary) current = MOVE_OPEN_HAND;
		else if (closed) current = CLOSED_HAND;
		break;
	case MOVE_OPEN_HAND:
		graphics::move_cursor(center);
		if (stationary) current = OPEN_HAND;
		break;
	case CLOSED_HAND:
		graphics::grab();
		if (stationary && !closed)  current = OPEN_HAND;
		else if (!stationary && !closed)
		{
			current = MOVE_OPEN_HAND;
			graphics::ungrab();
		}
		else if (!stationary && closed) current = MOVE_CLOSED_HAND;
		break;
	case MOVE_CLOSED_HAND:
		graphics::move_cube(center);
		if (!closed) {
			current = CLOSED_HAND;
			graphics::ungrab();
		}
		else current = MOVE_CLOSED_HAND;
		break;
	default:
		break;
	}

}


void Classifier::printState()
{
	std::string s;
	switch (current)
	{
	case OPEN_HAND:
		s = "OPEN_HAND";
		break;
	case MOVE_OPEN_HAND:
		s = "MOVE_OPEN_HAND";
		break;
	case CLOSED_HAND:
		s = "CLOSED_HAND";
		break;
	case MOVE_CLOSED_HAND:
		s = "MOVE_CLOSED_HAND";
		break;
	default:
		break;
	}

	std::cout << s << std::endl;
}