#include"Analysis.h"
#define _ROOT_TWO_ 1.41421356237309504880
#define _COS_30_ 0.86602540378
extern bool is_live;
extern cv::VideoWriter seg2, seg3;
inline float dist(cv::Point& p, cv::Point& q) 
{
	return cv::norm(p - q);
}

inline bool isAcute(cv::Point& p1, cv::Point& p2, cv::Point& q)
{
	return (p1 - q).ddot(p2 - q) >= 0.0;
}

inline int positive_modulo(int i, int n) {
	return (i % n + n) % n;
}

inline bool lessThan60(cv::Point& p1, cv::Point& p2, cv::Point& q)
{
	cv::Point v1 = p1 - q;
	cv::Point v2 = p2 - q;
	return v1.ddot(v2) >= 0.5 * cv::norm(v1) * cv::norm(v2);
}

inline bool lessThan120(cv::Point& p1, cv::Point& p2, cv::Point& q)
{
	cv::Point v1 = p1 - q;
	cv::Point v2 = p2 - q;
	return v1.ddot(v2) >= -0.5 * cv::norm(v1) * cv::norm(v2);
}

inline bool lessThan45(cv::Point& p1, cv::Point& p2, cv::Point& q)
{
	cv::Point v1 = p1 - q;
	cv::Point v2 = p2 - q;
	return v1.ddot(v2) >= -_ROOT_TWO_/2.0 * cv::norm(v1) * cv::norm(v2);
}

inline double cosine_angle(cv::Point& v1, cv::Point& v2)
{
	return v1.ddot(v2) / cv::norm(v1) / cv::norm(v2);
}

inline double cosine_angle(cv::Point& p1, cv::Point& p2, cv::Point& q)
{
	cv::Point v1 = p1 - q;
	cv::Point v2 = p2 - q;
	return cosine_angle(v1, v2);
}

bool kcurvature(vector<cv::Point>& contour, int index, int k_value)
{
	cv::Point q = contour[index];
	cv::Point p1 = contour[positive_modulo((index + k_value), contour.size())];
	cv::Point p2 = contour[positive_modulo((index + k_value), contour.size())];

	return lessThan60(p1, p2, q);

}


bool kcurvature_45(vector<cv::Point>& contour, int index, int k_value)
{
	cv::Point q = contour[index];
	cv::Point p1 = contour[positive_modulo((index + k_value), contour.size())];
	cv::Point p2 = contour[positive_modulo((index + k_value), contour.size())];

	return lessThan45(p1, p2, q);

}



inline double distance_from_line(cv::Point l1, cv::Point l2, cv::Point pt)
{
	cv::Point dir = l2 - l1;
	dir /= cv::norm(dir);
	cv::Point normal = cv::Point(-dir.x, dir.y);
	return normal.ddot(pt - l1);
}


inline bool point_above_wrist(cv::Point& wrist, cv::Point& center, cv::Point& pt)
{
	return isAcute(pt, center, wrist);
}


inline cv::Point tangent(cv::Point p1, cv::Point p2)
{
	cv::Point temp = p2 - p1; 
	return cv::Point(temp.y, -temp.x);
}


int closest_index(std::vector<cv::Point> pts, cv::Point& point)
{
	double min_dist = -1;
	int min_index = -1;
	for (int i = 0; i < pts.size(); i ++)
	{
		double dist = cv::norm(pts[i] - point);
		if (min_index < 0 || dist < min_dist)
		{
			min_dist = dist;
			min_index = i;
		}
	}
	return min_index;
}


vector<cv::Point> contourClustering(vector<cv::Point> contour, list<int> potential_indices)
{
	vector<cv::Point> fingers(0);

	int running_average = 0, running_count = 0;

	while (potential_indices.size() > 0 || running_count > 0)
	{
		if (potential_indices.size() == 0)
		{
			int index = running_average >= 0 ? running_average : contour.size() + running_average;
			fingers.push_back(contour[index]);
			running_average = 0;
			running_count = 0;
			break;
		}

		if (running_count == 0)
		{
			running_count++;
			running_average = potential_indices.front();
			potential_indices.pop_front();
		}
		else
		{
			int previous = potential_indices.back() - contour.size(),
				next = potential_indices.front();

			if (abs(running_average - previous) < 15)
			{
				int total = running_average * running_count + previous;
				running_count++; total /= running_count;
				potential_indices.pop_back();
			}
			else if (abs(next - running_average) < 15)
			{
				int total = running_average * running_count + next;
				running_count++; total /= running_count;
				potential_indices.pop_front();
			}
			else {
				int index = running_average >= 0 ? running_average : contour.size() + running_average;
				fingers.push_back(contour[index]);
				running_average = 0;
				running_count = 0;
			}
		}
	}
	return fingers;
}



void HandAnalysis::refine_contour()
{
	cv::Point axis = center - wrist;
	cv::Point tangent = cv::Point(axis.y, -axis.x);
	cv::Point pt1 = wrist - tangent * 5;
	cv::Point pt2 = wrist + tangent * 10;
	cv::LineIterator it(thresh, wrist, pt1, 8);
	cv::LineIterator it2(thresh, wrist, pt2, 8);
	cv::Point w1, w2;
	for (int i = 0; i < it.count; i++, ++it)
	{
		if (thresh.at<uchar>(it.pos()) == 0)
		{
			w1 = it.pos();
			break;
		}
	}
	for (int i = 0; i < it2.count; i++, ++it2)
	{
		if (thresh.at<uchar>(it2.pos()) == 0)
		{
			w2 = it2.pos();
			break;
		}
	}
	int a = closest_index(contour, w1);
	int b = closest_index(contour, w2);

	std::vector<cv::Point> newContour(0);

	for (int i = a; i <= (a > b ? b + contour.size() : b); i++)
		newContour.push_back(contour[i % contour.size()]);

	contour = newContour;
}

HandAnalysis::HandAnalysis()
{
	contour = vector<cv::Point>(0);
	fingers = vector<cv::Point>(0);
	hull = vector<cv::Point>(0);
}


void HandAnalysis::apply(cv::Mat &frame, cv::Mat &probImg, CenteredRect &bounds)
{
	this->frame = frame;
	this->prob = probImg;
	this->proposed_roi = bounds;
	
	threshold(); // thresholds the probImg and assigns the new image to thresh.
	max_contour();
	find_center_orientation();
	refine_contour();
	if (!is_live)
	{
		cv::Mat img = cv::Mat::zeros(frame.size(), CV_8U);
		vector<vector<cv::Point>> temp_contour(0);
		temp_contour.push_back(contour);
		cv::drawContours(img, temp_contour, 0, CV_RGB(255,255,255), -1);
		seg3.write(img);
	}
	max_hull();
	//finger_tips();
	finger_tips2();
	find_thumb();
	update_roi(bounds);
}

void HandAnalysis::show()
{
	if (contour.size() > 0)
	{

		vector<vector<cv::Point>> temp_contour(0);
		vector<vector<cv::Point>> temp_hull(0);
		temp_contour.push_back(contour);
		temp_hull.push_back(hull);
		cv::drawContours(frame, temp_contour, 0, CV_RGB(0, 255, 0), 2, 8);

		
		cv::drawContours(frame, temp_hull, 0, CV_RGB(0, 0, 255), 2, 8);
		for (cv::Point finger : fingers)
			cv::circle(frame, finger, 5, CV_RGB(255, 0, 0), 2);
		
		cv::circle(frame, center, 5, CV_RGB(255, 0, 255), 2);
		cv::circle(frame, center, radius * TIP_RADIUS, CV_RGB(255, 0, 255), 1);
		cv::circle(frame, center, radius, CV_RGB(255, 0, 255), 2);
		cv::line(frame, wrist, center, CV_RGB(255, 255, 0), 2);

		if (thumb_indx >= 0)
			cv::circle(frame, fingers[thumb_indx], 5, CV_RGB(187, 144, 212), -1);
			
	}

}

void HandAnalysis::threshold()
{

	cv::GaussianBlur(prob, thresh, cv::Size(5, 5), 0, 0);

	cv::threshold(thresh, thresh, 25, 255, cv::THRESH_BINARY);
	
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));

	cv::erode(thresh, thresh, element);
	cv::dilate(thresh, thresh, element);
}


void HandAnalysis::max_contour()
{
	vector<cv::Vec4i> hierarchy;
	vector<vector<cv::Point> > contours(0);


	cv::findContours(thresh, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	/// Find the convex hull,contours and defects for each contour
	double max_area = 0.0;
	int max_index = -1;
	cv::Rect region, max_region;
	for (int i = 0; i < contours.size(); i++)
	{
		double a = contourArea(contours[i], false);  //  Find the area of contour
		if (a > max_area)
		{
			region = cv::boundingRect(contours[i]);
			if ((region & proposed_roi).area() > proposed_roi.area() * 0.8)
			{
				max_area = a;
				max_index = i;                //Store the index of largest contour
				max_region = region;
			}
		}
	}

	if (max_index == -1)
	{
		std::cout << "Tracking Exception1" << endl;
		throw TrackingException();
	}
	contour = contours[max_index];
	bounding_box = cv::boundingRect(contour);
	// update thresholded binary image with the maximum contour. 
	thresh = cv::Mat::zeros(frame.size(), CV_8U);
	cv::drawContours(thresh, contours, max_index, CV_RGB(255, 255, 255), -1);
}




void HandAnalysis::max_hull()
{
	hull_indices = vector<int>(0);

	cv::convexHull(cv::Mat(contour), hull_indices, false);


	hull = vector<cv::Point>(hull_indices.size());

	list<int> temp = list<int>();
	
	for (int i : hull_indices)
		temp.push_back(i);

	contourClustering(contour, temp);
	hull_indices = vector<int>();
	
	for (int i : temp)
		hull_indices.push_back(i);

	for (int i = 0; i < hull_indices.size(); i++)
		hull[i] = contour[hull_indices[i]];

	if (hull_indices.size() > 3) {
		defects = vector<cv::Vec4i>(0);
		cv::convexityDefects(contour, hull_indices, defects);
	}

	else
	{
		std::cout << "Tracking Exception2" << endl;
		throw TrackingException();
	}

}

void HandAnalysis::find_center()
{
	CenteredRect roi = proposed_roi & cv::Rect(cv::Point(0, 0), frame.size());
	roi.enlarge(roi.center(), 5);
	roi = roi & cv::Rect(cv::Point(0, 0), frame.size());
	distTransform = cv::Mat::zeros(frame.size(), CV_32F);
	cv::distanceTransform(thresh(roi), distTransform(roi), CV_DIST_L2, 3, CV_32F);
	double a, b; cv::Point t;
	cv::minMaxLoc(distTransform(proposed_roi), &a, &b, &t, &center);

	center += proposed_roi.tl();
	radius = distTransform.at<float>(center);

	CenteredRect new_region = CenteredRect(center, cv::Size(radius * 2, radius * 2)) 
		& cv::Rect(cv::Point(0,0), frame.size());
	cv::Moments m = cv::moments(distTransform(new_region));
	center = cv::Point(m.m10 / m.m00, m.m01 / m.m00) + new_region.tl();
}


void HandAnalysis::find_wrist()
{
	cv::Mat dist;
	cv::normalize(distTransform, dist, 0, 1., cv::NORM_MINMAX);
	cv::Mat mask = cv::Mat::zeros(frame.size(), CV_32F);
	if (radius == 0)return;
	cv::circle(mask, center, radius, cv::Scalar(1), 10);
	cv::rectangle(mask, cv::Rect(center - cv::Point(radius, radius), 
		center + cv::Point(radius, 0)), CV_RGB(0, 0, 0), -1);

	cv::bitwise_and(mask, dist, mask);
	cv::blur(mask, mask, cv::Size(7,7));
	double a, b; cv::Point t;
	cv::minMaxLoc(mask, &a, &b, &t, &wrist);
	cv::Point axis = wrist - center;
	double temp = cv::sqrt(axis.x * axis.x + axis.y * axis.y);
	static cv::Point2f dir;
	cv::Point2f new_dir = cv::Point2f(axis.x/temp, axis.y/temp);
	if (dir == cv::Point2f(0,0) || dir.ddot(new_dir) > _COS_30_)
	{	// change in angle is smaller than 30 degrees.
		if (frame.size().height - center.y > 1.5 * radius)
			dir = new_dir;
	}

	wrist = center +(cv::Point) (dir * radius);
}

void HandAnalysis::find_center_orientation()
{
	find_center();
	find_wrist();
}



void HandAnalysis::finger_tips()
{
	vector<cv::Vec4i> filteredDefects(0);
	list<int> fingerTipIndices(0);
	for (cv::Vec4i def : defects)
	{
		cv::Point start(contour[def.val[0]]);
		cv::Point end(contour[def.val[1]]);
		cv::Point far(contour[def.val[2]]);

		double depth = def.val[3] / 256.0;
		if (depth > radius / 2.5)
			filteredDefects.push_back(def);
	}

	for (int i = 0; i < filteredDefects.size(); i++)
	{
		int size = contour.size();
		cv::Vec4i current = filteredDefects[i];
		if (kcurvature(contour, current.val[0], 1))
			fingerTipIndices.push_back(current.val[0]);

		if (kcurvature(contour, current.val[1], 1))
			fingerTipIndices.push_back(current.val[1]);
	}

	fingerTipIndices.sort();
	vector<cv::Point> potential(0);

	potential = contourClustering(contour, fingerTipIndices);

	// further remove false negatives
	fingers = vector<cv::Point>(0);
	finger_dist = vector<double>(0);
	finger_height = vector<double>(0);


	cv::Point axis = (center - wrist) / radius;

	for (cv::Point tip : potential)
	{
		double distance = dist(tip, center);
		if (distance > 1.6 * radius && distance < 4 * radius)
		{
			double v_distance = axis.ddot(tip - wrist);
			if (v_distance > 0)
			{
				finger_dist.push_back(distance);
				finger_height.push_back(v_distance);
				fingers.push_back(tip);
			}
		}
	}

}





void HandAnalysis::finger_tips2()
{
	vector<int> filteredDefects(0);
	fingers = vector<cv::Point>(0);
	finger_dist = vector<double>(0);
	finger_height = vector<double>(0);

	filteredDefects.push_back(1);

	for (cv::Vec4i def : defects)
	{
		cv::Point start(contour[def.val[0]]);
		cv::Point end(contour[def.val[1]]);
		cv::Point far(contour[def.val[2]]);
		double depth = def.val[3] / 256.0;
		if (depth > radius * 0.8)
		{
			filteredDefects.push_back(def.val[2]);
		}
	}
	filteredDefects.push_back(contour.size() - 1);
	std::sort(filteredDefects.begin(), filteredDefects.end());
	cv::Point axis = (center - wrist) / radius;

	for (int i = 0; i < filteredDefects.size() - 1; i++)
	{
		int indx1 = filteredDefects[i];
		int indx2 = filteredDefects[i + 1];
		double max_dist = 0.0;
		int max_indx = indx1;
		for (int j = indx1; j < indx2; j++)
		{
			double min_dist = dist(contour[j], center);
			if (min_dist > max_dist)
			{
				cv::Point pt = contour[j];
				
				double distance = dist(pt, center);
				if (distance > radius * TIP_RADIUS)
				{
					max_indx = j;
					max_dist = min_dist;
				}
			}
		}
		int k = cv::min(max_indx - indx1, indx2 - max_indx);
		if (kcurvature_45(contour, max_indx, 4))
		{
			cv::Point tip = contour[max_indx];
			double distance = dist(tip, center);
			double v_distance = axis.ddot(tip - wrist);
			if (v_distance > radius)
			{
				if (distance > radius * TIP_RADIUS)
				{
					finger_dist.push_back(distance);
					finger_height.push_back(v_distance);
					fingers.push_back(tip);
				}
			}
		}
	}
}

void HandAnalysis::find_thumb()
{
	thumb_indx = -1;
	if (fingers.size() == 0 || fingers.size() == 1) return;
	int last_indx = fingers.size() - 1;
	double max_value; int max_indx[2], min_indx[2];
	cv::minMaxIdx(finger_height, NULL, &max_value, min_indx, max_indx);
	
	min_height = fingers[min_indx[1]];
	max_height = fingers[max_indx[1]];
	double last_height = finger_height[last_indx];
	if (max_value - last_height > radius * TIP_THUMB_HEIGHT_DIFF)
	{
		if (dist(max_height, fingers[last_indx]) > radius)
		{
			thumb_indx = last_indx;
		}
	}
}

void HandAnalysis::update_roi(CenteredRect& bounds)
{
	bounds = CenteredRect(center, cv::Size(radius * _ROOT_TWO_, radius * _ROOT_TWO_));
}