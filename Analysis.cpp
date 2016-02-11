#include"Analysis.h"
#define SIN_60_DEG 0.8660254038 


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


std::vector<cv::Point> circle_sample_pts(cv::Point center, double radius)
{
	std::vector<cv::Point> pts(0);
	pts.push_back(center + (cv::Point(0, 1) * radius));
	pts.push_back(center + (cv::Point(1, 0) * radius));
	pts.push_back(center + (cv::Point(0, -1) * radius));
	pts.push_back(center + (cv::Point(-1, 0) * radius));
	pts.push_back(center + cv::Point(SIN_60_DEG * radius, 0.5 * radius));
	pts.push_back(center + cv::Point(SIN_60_DEG * radius, -0.5 * radius));
	pts.push_back(center + cv::Point(-SIN_60_DEG * radius, 0.5 * radius));
	pts.push_back(center + cv::Point(-SIN_60_DEG * radius, -0.5 * radius));
	pts.push_back(center + cv::Point(0.5 * radius, SIN_60_DEG * radius));
	pts.push_back(center + cv::Point(0.5 * radius, -SIN_60_DEG * radius));
	pts.push_back(center + cv::Point(-0.5 * radius, SIN_60_DEG * radius));
	pts.push_back(center + cv::Point(-0.5 * radius, -SIN_60_DEG * radius));
	return pts;
}




int max_Point(std::vector<cv::Point> pts, cv::Mat& img)
{
	int maxIdx = -1;
	float max = 0.0;
	for (int i = 0; i < pts.size(); i ++)
	{
		cv::Point pt = pts[i];
		float intensity = img.at<float>(pt);
		if (intensity > max)
		{
			max = intensity;
			maxIdx = i;
		}

	}
	return maxIdx;
}


bool kcurvature(vector<cv::Point>& contour, int index, int k_value)
{
	cv::Point q = contour[index];
	cv::Point p1 = contour[positive_modulo((index + k_value), contour.size())];
	cv::Point p2 = contour[positive_modulo((index + k_value), contour.size())];

	return lessThan60(p1, p2, q);

}



cv::Point Analysis::find_center(cv::Rect& region)
{
	cv::Mat dist = cv::Mat::zeros(frame.size(), CV_32F);
	cv::distanceTransform(threshedimg(bounding_box), dist(bounding_box), CV_DIST_L2, 3, CV_32F);

	cv::Moments m = cv::moments(dist(region));
	cv::Point center = cv::Point(m.m10 / m.m00, m.m01 / m.m00);
	center += region.tl();
	radius = dist.at<float>(center);
	std::vector<cv::Point> pts = circle_sample_pts(center, radius * 2);

	cv::Point wrist = pts[max_Point(pts, dist)];
	cv::circle(frame, wrist, 5, CV_RGB(0, 0, 255), -1);
	return center;
}


cv::Point Analysis::find_center()
{
	cv::Mat dist;
	
	cv::distanceTransform(threshedimg(bounding_box), dist, CV_DIST_L2, 3, CV_32F);
	cv::Moments m = cv::moments(dist);
	cv::Point center = cv::Point(m.m10 / m.m00, m.m01 / m.m00);
	radius = dist.at<float>(center);


	center += bounding_box.tl();
	return center;
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






void Analysis::threshold(cv::Mat& probImg)
{

	cv::GaussianBlur(probImg, threshedimg, cv::Size(5, 5), 0, 0);

	cv::threshold(threshedimg, threshedimg, 25, 255, cv::THRESH_BINARY);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(8, 8));

	cv::erode(threshedimg, threshedimg, element);
	cv::dilate(threshedimg, threshedimg, element);
}





void Analysis::threshold(cv::Mat& probImg, CenteredRect& mask)
{
	cv::Mat tempMask = cv::Mat::zeros(probImg.size().height, probImg.size().width, CV_8U);
	bounding_box = mask & cv::Rect(cv::Point(0, 0), probImg.size());
	cv::Rect rmask = (cv::Rect) bounding_box;

	tempMask(rmask) = 255;

	cv::bitwise_and(probImg, tempMask, threshedimg);



	cv::GaussianBlur(threshedimg, threshedimg, cv::Size(5, 5), 0, 0);

	cv::threshold(threshedimg, threshedimg, 25, 255, cv::THRESH_BINARY);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(8, 8));

	cv::erode(threshedimg, threshedimg, element);
	cv::dilate(threshedimg, threshedimg, element);
}



void Analysis::handStructure()
{
	vector<cv::Vec4i> hierarchy;
	vector<vector<cv::Point> > contours(0);


	cv::findContours(threshedimg, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	/// Find the convex hull,contours and defects for each contour

	double max_area = 0.0;
	int max_index = -1;
	for (int i = 0; i < contours.size(); i++)
	{
		double a = contourArea(contours[i], false);  //  Find the area of contour
		if (a > max_area)
		{
			cv::Rect area = cv::boundingRect(contours[i]);
			if ((area & bounding_box).size().area() > bounding_box.size().area() / 2)
			{
				max_area = a;
				max_index = i;                //Store the index of largest contour
			}
		}
	}

	contour = contours[max_index];


	vector<int> hull_indices = vector<int>(0);
	
	cv::convexHull(cv::Mat(contour), hull_indices, false);


	hull = vector<cv::Point>(hull_indices.size());
	for (int i = 0; i < hull_indices.size(); i++)
		hull[i] = contour[hull_indices[i]];

	cv::Rect region = bounding_box;
	roi = cv::fitEllipse(hull);
	bounding_box = roi.boundingRect() & cv::Rect(cv::Point(0, 0), frame.size());;

	threshedimg = cv::Mat::zeros(frame.size(), CV_8U);
		cv::drawContours(threshedimg, contours, max_index, CV_RGB(255, 255, 255), -1);


	defects = vector<cv::Vec4i>(0);
	if (hull_indices.size() > 3) {
		cv::convexityDefects(contours[max_index], hull_indices, defects);
		center = find_center(region);
		condefects(defects);
	}


}


void Analysis::condefects(vector<cv::Vec4i> convexityDefectsSet)
{
	double limit = roi.size.width * 0.5;

	vector<cv::Vec4i> filteredDefects(0);
	list<int> fingerTipIndices(0);
	for (cv::Vec4i def : convexityDefectsSet)
	{
		cv::Point start(contour[def.val[0]]);
		cv::Point end(contour[def.val[1]]);
		cv::Point far(contour[def.val[2]]);

		double depth = def.val[3] / 256.0;
		if (depth > radius/2) 
			filteredDefects.push_back(def);
		/*
		double distance = dist(start, far) + dist(end, far);
		if (distance > limit && distance < roi.size.width * 2)
		{
			if (lessThan120(start, end, far))
				filteredDefects.push_back(def);
		}*/
	}

	for (int i = 0; i < filteredDefects.size(); i++)
	{
		int size = contour.size();
		cv::Vec4i current = filteredDefects[i];
		if (kcurvature(contour, current.val[0], size / 25))
			fingerTipIndices.push_back(current.val[0]);

		if (kcurvature(contour, current.val[1], size / 25)) 
			fingerTipIndices.push_back(current.val[1]);
	}

	fingerTipIndices.sort();
	vector<cv::Point> potential(0);

	potential = contourClustering(contour, fingerTipIndices);

	// further remove false negatives
	fingers = vector<cv::Point>(0);

	for (cv::Point tip : potential)
	{
		if (dist(tip, center) > 1.8 * radius)
			if (tip.y < center.y + radius)
				fingers.push_back(tip);
	}


}



Analysis::Analysis()
{
	contour = vector<cv::Point>(0);
	fingers = vector<cv::Point>(0);
	hull = vector<cv::Point>(0);

}


void Analysis::apply(cv::Mat &frame, cv::Mat &probImg, CenteredRect &bounds, cv::RotatedRect &roi)
{
	radius = 0.0;
	bounding_box = bounds;
	this->frame = frame;
	threshold(probImg);
	handStructure();

}






void Analysis::display()
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

		cv::circle(frame, center, radius, CV_RGB(255, 0, 255), 2);

	}


}








