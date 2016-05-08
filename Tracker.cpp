#include "Tracker.h"

extern cv::VideoWriter seg1, seg1_t, seg2;
extern bool is_live;
extern ResultLog r_log;

Tracker::Tracker() {}


Tracker::Tracker(CenteredRect& initial_window, cv::MatND& hist_model, BackgroundSubtractor& bg)
	: bg(bg), hist_model(hist_model), tracking_window(initial_window)
{
	hand_processer = HandAnalysis();
	//classifier = StateClassifier(hand_processer);
	classifier = HMMClassifier(hand_processer);
	tracking_window_fitted = cv::RotatedRect(tracking_window.center(), tracking_window.size(), 0.0);
	hist_model.copyTo(hist_model_original);
}


void Tracker::process_frame(cv::Mat& input_BGR, cv::Mat& input_HSV, cv::Mat& output_foreground,
	cv::Mat& output_backproj, cv::Mat& output)
{
	bg.apply_frame(input_BGR, output_foreground, 0);
	output_backproj = back_project(input_HSV, hist_model);
	if (!is_live) seg1.write(output_backproj);
	if (!is_live) seg1_t.write(output_foreground);
	cv::bitwise_and(output_backproj, output_foreground, output_backproj);
	cv::addWeighted(output_backproj, BACKPROJ_PROPORTION, 
		output_foreground, 1- BACKPROJ_PROPORTION, 0.0, output);
	if (!is_live) seg2.write(output);
	cv::meanShift(output_backproj, tracking_window, 
		cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1));
	CenteredRect tracking_window_copy = tracking_window & tracking_window;
	hand_processer.apply(input_BGR, output, tracking_window);
	cv::Mat mask = cv::Mat::zeros(input_HSV.size(), CV_8UC1);
	cv::rectangle(mask, tracking_window, CV_RGB(255, 255, 255), -1);

	cv::MatND new_hist_model = get_hist_hsv(input_HSV, mask);
	double hist_dist = cv::compareHist(hist_model, new_hist_model, CV_COMP_BHATTACHARYYA);
	if (hist_dist < HIST_MAX_DISTANCE)
		cv::addWeighted(new_hist_model, HIST_LEARNING_RATE,
			hist_model, 1-HIST_LEARNING_RATE, 1, hist_model);
	
	classifier.apply(hand_processer);
	if (!is_live)
	{
		r_log.message(classifier.str2());
		r_log.position(hand_processer.center);
	}
	//cout << "State = " << classifier.str() << endl;
	bg.apply_frame(input_BGR, output, FRAME_LEARNING_RATE, hand_processer.thresh);


	if (DISPLAY_CONTOUR_ANNOTATIONS)
	{
		cv::rectangle(input_BGR, tracking_window_copy, CV_RGB(255, 255, 0), 1);
		cv::rectangle(input_BGR, tracking_window, CV_RGB(255, 0, 255), 1);
		hand_processer.show();
	}
}



void Tracker::set_window(CenteredRect& window)
{
	tracking_window = window;
}




