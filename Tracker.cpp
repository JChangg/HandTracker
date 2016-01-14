#include "Tracker.h"

#define LEFT_KEY 2424832
#define RIGHT_KEY 2555904
#define UP_KEY 2490368
#define DOWN_KEY 2621440

#define DISPLAY_BGR 0
#define DISPLAY_HSV 1
#define DISPLAY_BACK_PROJ 2

#define OP_SETUP 0
#define OP_MAIN 1
#define OP_EXIT 2

using namespace std;




const string help_setup = "-HELP SETUP-\n\
navigate the window around to capture your hand. \n\
1) Use ARROW keys to move the window to suit the position of your hand. \n\
3) Use '-' to shrink and '=' to expand the window. \n\
2) Ensure that your entire hand lies within then blue window and it covers the \n\
green window entirely and press ENTER. \n\
3) Press ESC to exit. \n\n";

void setup(cv::VideoCapture& stream) {
	cv::Mat imgOriginal, hist, imgHSV, backproj, foreground;
	CenteredRect tw;
	
	cv::Ptr<cv::BackgroundSubtractor> bg =
		cv::createBackgroundSubtractorMOG2(1500, 16, false);

	int operation_mode = OP_SETUP;
	int display_mode = DISPLAY_BGR;

	// default size and position of window
	cv::Point pos = cv::Point(100.0, 200.0);
	cv::Size size = cv::Size(100, 200);
	Window w = Window(pos, size);
	cout << help_setup << endl;
	// frame capture loop
	while (stream.isOpened() && operation_mode != OP_EXIT) {
		if (!stream.read(imgOriginal) || imgOriginal.empty()) {
			throw FailedToRead("FAILURE: cannot read image/frame.");
			return;
		}
		cv::flip(imgOriginal, imgOriginal, 1); // flips the frame to mirrror movement
		cv::cvtColor(imgOriginal, imgHSV, cv::COLOR_BGR2HSV);

		int delta = size.width / 4.0; // unit change in position.
		int keyPress = cv::waitKey(1);		// delay (in ms) and get key press, if anychar

		if (operation_mode == OP_SETUP)
		{
			switch (keyPress)
			{
			case LEFT_KEY:
				w.move(cv::Point(-delta, 0)); // move left
				break;
			case RIGHT_KEY:
				w.move(cv::Point(delta, 0)); // move left
				break;
			case UP_KEY:
				w.move(cv::Point(0, -delta)); // move left
				break;
			case DOWN_KEY:
				w.move(cv::Point(0, delta)); // move left
				break;
			case '=':
				w.scale(1.1);
				break;
			case '-':
				w.scale(0.9);
				break;
			case 's':
				if (!hist.empty()) operation_mode = OP_MAIN;
				tw = w.outer;
				break;
			case 13:
				hist = w.histogram(imgOriginal);
				break;
			case 27:
				operation_mode = OP_EXIT;
				break;
			}
			w.draw(imgOriginal);
		}

		else if (operation_mode == OP_MAIN) {
			if (hist.empty()) {
				operation_mode = OP_SETUP;
				cout << "Error: histogram not found, please redo the setup." << endl;
				continue;
			}

			bg->apply(imgOriginal, foreground, 0.002);
			backproj = back_project(imgHSV, hist);
			double alpha = 0.5;
			cv::addWeighted(backproj, alpha, foreground, 1 - alpha, 0.0, backproj);

			cv::RotatedRect rtw = cv::CamShift(backproj, tw, 
				cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1));

			if (tw.area() <= 1)
			{
				// recreate the box around the tracked point.
				int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5) / 6;
				tw = CenteredRect(cv::Point(tw.x - r, tw.y - r), cv::Point(tw.x + r, tw.y + r));
			}

			tw.enlarge(tw.center(), 1.2);
			cv::rectangle(imgOriginal, tw, cv::Scalar(255, 0, 0), 1);
	
			

			

			switch (keyPress)
			{
			case 'r':
				operation_mode = OP_SETUP;
				break;
			case 'c':
				cv::imwrite("screencap_rgb.jpg", imgOriginal);
				cv::imwrite("screencap_hsv.jpg", imgHSV);
				break;
			case 27:
				operation_mode = OP_EXIT;
				break;
			}

			cv::namedWindow("Back Project", CV_WINDOW_AUTOSIZE);
			cv::imshow("Back Project", backproj);		// show windows

		}

		// display the window
		cv::namedWindow("Tracker", CV_WINDOW_AUTOSIZE);
		cv::imshow("Tracker", imgOriginal);		// show windows
	}
	cout << "Exiting tracker!" << endl;
	cv::destroyAllWindows();
}


