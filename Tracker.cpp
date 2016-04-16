#include "Tracker.h"

extern Logger program_log;


Tracker::Tracker() {}


Tracker::Tracker(CenteredRect& initial_window, cv::MatND& hist_model, BackgroundSubtractor& bg)
	: bg(bg), hist_model(hist_model), tracking_window(initial_window)
{
	hand_processer = HandAnalysis();
	classifier = StateClassifier(hand_processer);
	//classifier = HMMClassifier(hand_processer);
	tracking_window_fitted = cv::RotatedRect(tracking_window.center(), tracking_window.size(), 0.0);
}


void Tracker::process_frame(cv::Mat& input_BGR, cv::Mat& input_HSV, cv::Mat& output_foreground,
	cv::Mat& output_backproj, cv::Mat& output)
{
	bg.apply_frame(input_BGR, output_foreground, 0);

	output_backproj = back_project(input_HSV, hist_model);
	cv::bitwise_and(output_backproj, output_foreground, output_backproj);
	cv::addWeighted(output_backproj, BACKPROJ_PROPORTION, 
		output_foreground, 1- BACKPROJ_PROPORTION, 0.0, output);

	cv::meanShift(output_backproj, tracking_window, 
		cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1));

	hand_processer.apply(input_BGR, output, tracking_window);
	cv::Mat mask = cv::Mat::zeros(input_HSV.size(), CV_8UC1);
	cv::rectangle(mask, tracking_window, CV_RGB(255, 255, 255), -1);

	cv::MatND new_hist_model = get_hist_hsv(input_HSV, mask);
	double hist_dist = cv::compareHist(hist_model, new_hist_model, CV_COMP_BHATTACHARYYA);
	if (hist_dist < HIST_MAX_DISTANCE)
		cv::addWeighted(new_hist_model, HIST_LEARNING_RATE,
			hist_model, 1-HIST_LEARNING_RATE, 1, hist_model);
	

	classifier.apply(hand_processer);
	//cout << "State = " << classifier.str() << endl;
	program_log.event("State = " + classifier.str());
	bg.apply_frame(input_BGR, output, FRAME_LEARNING_RATE, hand_processer.thresh);


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




