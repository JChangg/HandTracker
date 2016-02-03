#include"Contour.h"

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







bool kcurvature(vector<cv::Point>& contour, int index, int k_value)
{
	cv::Point q = contour[index];
	cv::Point p1 = contour[positive_modulo((index + k_value), contour.size())];
	cv::Point p2 = contour[positive_modulo((index + k_value), contour.size())];

	return lessThan60(p1, p2, q);

}




cv::Point Analysis::find_center(cv::Point proposed_center, int width, int delta, int delta2)
{
	cv::Point start = cv::Point(proposed_center.x - width / 2, proposed_center.y - width / 2);

	double max_min_distance = 0;
	cv::Point new_center(proposed_center);

	for (int h = 0; h < width; h+=delta2)
	{
		for (int w = 0; w < width; w+= delta2)
		{
			cv::Point pt = start + cv::Point(w, h);
			double min_distance = -1;
			for (int i = 0; i < contour.size(); i += delta)
			{
				double distance = dist(pt, contour[i]);

				if (min_distance < 0 || min_distance > distance)
					min_distance = distance;
			}

			if (min_distance > max_min_distance)
			{
				new_center = pt;
				max_min_distance = min_distance;
			}
		}
	}
	
	cv::circle(frame, new_center, max_min_distance, CV_RGB(255, 0, 0), 2);
	radius = max_min_distance;
	return new_center;
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




void Analysis::threshold(cv::Mat& probImg, CenteredRect& mask)
{
	cv::Mat tempMask = cv::Mat::zeros(probImg.size().height, probImg.size().width, CV_8U);
	cv::Rect rmask = (cv::Rect) mask & cv::Rect(cv::Point(0, 0), probImg.size());

	tempMask(rmask) = 255;

	cv::bitwise_and(probImg, tempMask, threshedimg);



	cv::GaussianBlur(threshedimg, threshedimg, cv::Size(3, 3), 0, 0);

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
			max_area = a;
			max_index = i;                //Store the index of largest contour
		}
	}

	contour = contours[max_index];


	vector<int> hull_indices = vector<int>(0);
	cv::convexHull(cv::Mat(contour), hull_indices, false);
	hull = vector<cv::Point>(hull_indices.size());
	for (int i = 0; i < hull_indices.size(); i++)
		hull[i] = contour[hull_indices[i]];


	roi = cv::fitEllipse(hull);
	
	center = find_center(roi.center, roi.size.width / 2, 20, 5);

	vector<cv::Vec4i>  defects(0);
	if (hull_indices.size() > 3) {
		cv::convexityDefects(contours[max_index], hull_indices, defects);
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
		double distance = dist(start, far) + dist(end, far);
		if (distance > limit && distance < roi.size.width * 2)
		{
			if (lessThan120(start, end, far))
				filteredDefects.push_back(def);
		}
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
		if (dist(tip, center) > 1.5 * radius)
		{
			if (tip.y < center.y + radius)
			{
				fingers.push_back(tip);
			}
		}
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
	this->frame = frame;
	threshold(probImg, bounds);
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
	}

	cv::circle(frame, center, 7, CV_RGB(255, 0, 255), 2);

}








