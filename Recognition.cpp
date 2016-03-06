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
	case State::OPEN_HAND:
		if (!stationary) current = State::MOVE_OPEN_HAND;
		else if (closed) current = State::CLOSED_HAND;
		break;
	case State::MOVE_OPEN_HAND:
		graphics::move_cursor(center);
		if (stationary) current = State::OPEN_HAND;
		break;
	case State::CLOSED_HAND:
		graphics::grab();
		if (stationary && !closed)  current = State::OPEN_HAND;
		else if (!stationary && !closed)
		{
			current = State::MOVE_OPEN_HAND;
			graphics::ungrab();
		}
		else if (!stationary && closed) current = State::MOVE_CLOSED_HAND;
		break;
	case State::MOVE_CLOSED_HAND:
		graphics::move_cube(center);
		if (!closed) {
			current = State::CLOSED_HAND;
			graphics::ungrab();
		}
		else current = State::MOVE_CLOSED_HAND;
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
	case State::OPEN_HAND:
		s = "OPEN_HAND";
		break;
	case State::MOVE_OPEN_HAND:
		s = "MOVE_OPEN_HAND";
		break;
	case State::CLOSED_HAND:
		s = "CLOSED_HAND";
		break;
	case State::MOVE_CLOSED_HAND:
		s = "MOVE_CLOSED_HAND";
		break;
	default:
		break;
	}

	std::cout << s << std::endl;
}




inline bool StateClassifier::update_center()
{
	bool station = (dist(hand_analyser.center, this->center) < hand_analyser.radius * 0.2);
	if (station) this->center = (hand_analyser.center + this->center) / 2;
	else this->center = hand_analyser.center;
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
	stat = OPEN;
	dynamic = MOVE;
}

StateClassifier::StateClassifier(HandAnalysis & h): hand_analyser(h)
{
	this->center = hand_analyser.center;
	this->radius = hand_analyser.radius;
	this->stat = OPEN;
	this->dynamic = MOVE;
}

void StateClassifier::apply(HandAnalysis& hand)
{
	hand_analyser = hand;
	update_move_state();

	StaticState prev_state = stat;
	

	switch (dynamic)
	{
	case MoveState::MOVE:
		// update position
		graphics::move_cursor(center);
		graphics::update_cube_position();
		break;
	case MoveState::STATION:
		stat = getStaticState();
		if (stat != prev_state)
		{
			switch (stat)
			{
			case OPEN:
				// do nothing
				graphics::ungrab();
				break;
			case CLOSED:
				graphics::grab();
				break;
			case SCROLL:
				// scroll 
				graphics::ungrab();
				break;
			case PINCH:
				graphics::ungrab();
				original_value = current_value;
				break;
			case POINTER:
				graphics::ungrab();
				break;
			}
		}
		else
		{
			switch (stat)
			{
			case OPEN:
				// do nothing
				break;
			case CLOSED:
				break;
			case SCROLL:
				// scroll 
				break;
			case PINCH:
				graphics::rescale(current_value / original_value);
				break;
			case POINTER:
				break;
			}
		}
		break;
	case MoveState::START:
		// start static state movement.
		graphics::move_cursor(center);
		graphics::update_cube_position();
		break;
	}
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
		return StaticState::POINTER;
	}

	else if (hand_analyser.fingers.size() == 2)
	{
		double fheight1 = hand_analyser.finger_height[0];
		double fheight2 = hand_analyser.finger_height[1];
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

		if (maxheight - minheight > 0.5 * hand_analyser.radius)
		{
			current_value = dist(hand_analyser.fingers[max_index], 
								hand_analyser.fingers[min_index]);
			return StaticState::PINCH;
		}
		else return StaticState::SCROLL;

	}
}

void StateClassifier::printState()
{
	std::string s, m;
	switch (stat)
	{
	case OPEN:
		s = "OPEN";
		break;
	case CLOSED:
		s = "CLOSED";
		break;
	case SCROLL:
		s = "SCROLL";
		break;
	case PINCH:
		s = "PINCH";
		break;
	case POINTER:
		s = "POINTER";
		break;
	default:
		break;
	}

	switch (dynamic)
	{
	case MOVE:
		m = "MOVE";
		break;
	case STATION:
		m = "STATION";
		break;
	case START:
		m = "START";
		break;
	}

	std::cout << "S = " << s << ", D = " << m << std::endl;
}

