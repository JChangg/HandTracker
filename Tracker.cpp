#include "Tracker.h"

////////////////////////////////////////////////////////
#define DISPLAY_CONTOUR_ANNOTATIONS true


////////////////////////////////////////////////////////

Tracker::Tracker() {}


Tracker::Tracker(CenteredRect& initial_window, cv::MatND& hist_model, BackgroundSubtractor& bg, double alpha)
	: bg(bg), hist_model(hist_model), alpha(alpha), beta(1-alpha), tracking_window(initial_window)
{
	hand_processer = HandAnalysis();
	classifier = StateClassifier(hand_processer);
	tracking_window_fitted = cv::RotatedRect(tracking_window.center(), tracking_window.size(), 0.0);
}


void Tracker::process_frame(cv::Mat& input_BGR, cv::Mat& input_HSV, cv::Mat& output_foreground,
	cv::Mat& output_backproj, cv::Mat& output)
{
	bg.apply_frame(input_BGR, output_foreground, 0);
	output_backproj = back_project(input_HSV, hist_model);
	cv::bitwise_and(output_backproj, output_foreground, output_backproj);
	cv::addWeighted(output_backproj, alpha, output_foreground, beta, 0.0, output);


	cv::meanShift(output_backproj, tracking_window, cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1));

	hand_processer.apply(input_BGR, output, tracking_window);
	cv::Mat mask = cv::Mat::zeros(input_HSV.size(), CV_8UC1);
	cv::rectangle(mask, tracking_window, CV_RGB(255, 255, 255), -1);


	hist_model = get_hist_hsv(input_HSV, mask);
	

	classifier.apply(hand_processer);
	//classifier.printState();
	bg.apply_frame(input_BGR, output, 0.15, hand_processer.thresh);


	if (DISPLAY_CONTOUR_ANNOTATIONS)
	{
		cv::rectangle(input_BGR, tracking_window, CV_RGB(255, 0, 255), 1);
		hand_processer.show();
	}
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



