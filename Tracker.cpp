#include "Tracker.h"


Tracker::Tracker() {}


Tracker::Tracker(CenteredRect& initial_window, cv::MatND& hist_model, BackgroundSubtractor& bg, double alpha)
	: bg(bg), hist_model(hist_model), alpha(alpha), beta(1-alpha), tracking_window(initial_window)
{
	tracking_window_fitted = cv::RotatedRect(tracking_window.center(), tracking_window.size(), 0.0);
}


void Tracker::process_frame(cv::Mat& input_BGR, cv::Mat& input_HSV, cv::Mat& output_foreground,
	cv::Mat& output_backproj, cv::Mat& output)
{
	bg.apply_frame(input_BGR, output_foreground, 0);
	output_backproj = back_project(input_HSV, hist_model);
	cv::addWeighted(output_backproj, alpha, output_foreground, beta, 0.0, output);



	tracking_window_fitted = cv::CamShift(output_backproj, tracking_window,
		cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 5, 1));

	if (tracking_window.area() <= 1)
	{
		// recreate the box around the tracked point.
		int cols = output_backproj.cols, rows = output_backproj.rows, r = (MIN(cols, rows) + 5) / 6;
		tracking_window = CenteredRect(cv::Point(tracking_window.x - r, tracking_window.y - r),
			cv::Point(tracking_window.x + r, tracking_window.y + r));
	}
	CenteredRect roi = CenteredRect(tracking_window);
	
	// region of interest tracks the palm center enlarge inorder to include the fingers
	roi.enlarge(roi.center() + cv::Point(0, roi.size().height / 2), 2);

	cv::rectangle(input_BGR, roi, CV_RGB(255, 255, 0), 1);

	hand_processer.apply(input_BGR, output, roi, tracking_window_fitted);
	classifier.apply(hand_processer.fingers, hand_processer.center, hand_processer.radius);
	

	bg.apply_frame(input_BGR, output, 0.05, hand_processer.contour);
	

	if (tracking_window_fitted.size.area() > hand_processer.roi.size.area())
	{
		tracking_window = tracking_window & hand_processer.roi.boundingRect();
	}


	hand_processer.display();
	

}

void Tracker::set_window(CenteredRect& window)
{
	tracking_window = window;
}


void Tracker::set_alpha(double alpha)
{
	this->alpha = alpha;
	this->beta = 1 - alpha;
}


