#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/video/tracking.hpp>
#include<opencv2/video/background_segm.hpp>
#include<stdexcept>
#include<iostream>
#include<string>
#include<vector>
#include<chrono>
#include<fstream>
#include<thread>
#include"Hist.h"
#include"Window.h"
#include"Background.h"
#include"Tracker.h"
#include"Demo.h"
#include"Log.h"
#include"Settings.h"	
#include"Exception.h"

#define LEFT_KEY 2424832
#define RIGHT_KEY 2555904
#define UP_KEY 2490368
#define DOWN_KEY 2621440
#define ENTER_KEY 13
#define EQUAL_PLUS_KEY '='
#define MINUS_UNDERSCORE_KEY '-'
#define ESC_KEY 27

#define WRITER_CODEC CV_FOURCC('D','I','V','X')
#define WRITER_FPS 8 
#define MFRAME_SIZE cv::Size(640, 480)

#define OP_SETUP 0
#define OP_MAIN 1
#define OP_EXIT 2
#define OP_INIT 3

int process_argc;
char ** process_argv;
bool is_live = true;
extern OperationLog op_log;

std::string log_name;
//cv::Mat seg1, seg2, seg3;
cv::VideoWriter seg1, seg1_t, seg2, seg3, annotation, main_record;

bool live_run()
{
	log_name = get_name();
	op_log = OperationLog(log_name);
	cv::VideoCapture stream(1);			// initialte stream from vidoe camera.
	cv::VideoWriter output_stream;
	if (TRACKER_RECORD)
		output_stream = cv::VideoWriter(log_name + ".avi",
			WRITER_CODEC, WRITER_FPS, MFRAME_SIZE);


	int operation_mode = OP_INIT;
	cv::Mat imgBGR, imgHSV, backproj, foreground, hist, segmented;

	// initialize selection window to default size and position.
	Window select_win = Window(DEFAULT_POSITION, DEFAULT_SIZE);

	// initiate Mixture of Gaussian background subtractor
	BackgroundSubtractor bg = BackgroundSubtractor(50, 24);
	int i = 0; Tracker tracker;

	bool demo_on = false;
	op_log.start();
	// frame capture loop
	while (stream.isOpened() && operation_mode != OP_EXIT) {
		// Set up frame here.
		if (!stream.read(imgBGR) || imgBGR.empty()) {
			throw FailedToRead("FAILURE: cannot read image/frame.");
			return false;
		}
		cv::flip(imgBGR, imgBGR, 1);						// flips the frame to mirrror movement
		if (TRACKER_RECORD) output_stream.write(imgBGR);
		cv::cvtColor(imgBGR, imgHSV, cv::COLOR_BGR2HSV);	// convert the image to hue sturation and value image

		int keyPress = cv::waitKey(1);						// waits 1ms and assigns keypress (if any) to keypress
		op_log.key(keyPress);

		if (operation_mode == OP_INIT)
		{
			if (i >= INIT_BACK_SUB) operation_mode = OP_SETUP;
			bg.apply_frame(imgBGR, foreground, 0.5);

			std::string text = "Please wait ...";
			cv::putText(imgBGR, text, cv::Point(25, 50),
				cv::FONT_HERSHEY_SIMPLEX, 1, CV_RGB(0, 255, 255), 4);
			i++;
			if (keyPress == ESC_KEY) break;
		}

		// setup stage displays window to capture histogram of hand
		// window is allowed to move in position and change in szie
		else if (operation_mode == OP_SETUP) {
			cv::MatND hist;
			int delta = select_win.outer.size().width / 4.0;// unit size for the change in control
			switch (keyPress)								// controls for the selection phase
			{
			case LEFT_KEY:
				select_win.move(cv::Point(-delta, 0));
				break;
			case RIGHT_KEY:
				select_win.move(cv::Point(delta, 0));
				break;
			case UP_KEY:
				select_win.move(cv::Point(0, -delta));
				break;
			case DOWN_KEY:
				select_win.move(cv::Point(0, delta));
				break;
			case EQUAL_PLUS_KEY:
				select_win.scale(1.1);
				break;
			case MINUS_UNDERSCORE_KEY:
				select_win.scale(0.9);
				break;
			case ENTER_KEY:
				hist = select_win.histogram(imgBGR);
				tracker = Tracker(select_win.inner, hist, bg);
				operation_mode = OP_MAIN;		// advance to tracking mode
				if (DISPLAY_DEMO)
				{
					if (!demo_on)
					{
						demo_on = true;
						std::thread t(graphics::setup, process_argc, process_argv);
						t.detach();
					}
				}
				break;
			case ESC_KEY:
				operation_mode = OP_EXIT;
				break;
			}
			select_win.draw(imgBGR);						// draw the window on the coloured image
		}


		// track the hand's position in real time 
		else if (operation_mode == OP_MAIN) {
			try
			{
				tracker.process_frame(imgBGR, imgHSV, foreground, backproj, segmented);
			}
			catch (TrackingException& e)
			{
				std::cout << e.what() << endl;
				operation_mode = OP_SETUP;
			}
			switch (keyPress)
			{
			case 'r':
				operation_mode = OP_SETUP;
				break;
			case ESC_KEY:
				operation_mode = OP_EXIT;
				break;
			}
		}
		// display the window
		op_log.frame();
		cv::namedWindow("Tracker", CV_WINDOW_AUTOSIZE);
		cv::imshow("Tracker", imgBGR);
	}
	cout << "Exiting tracker!" << endl;
	cv::destroyAllWindows();
	stream.release();
	if (TRACKER_RECORD) output_stream.release();
	return true;
}



bool record_run(std::string name)
{
	extern ResultLog r_log;
	is_live = false;
	log_name = get_name(name);
	r_log = ResultLog(log_name);
	cv::VideoCapture stream(log_name + ".avi");
	if (!stream.isOpened())
	{
		std::cout << "Unable to read video file." << std::endl;
		return false;
	}
	std::ifstream inputs;
	inputs.open(log_name + "_op.txt", std::fstream::in);
	if (!inputs.is_open())
	{
		std::cout << "Unable to read log file." << std::endl;
		return false;
	}
	main_record = cv::VideoWriter(log_name + "_in.avi",
		WRITER_CODEC, WRITER_FPS, MFRAME_SIZE);
	seg1 = cv::VideoWriter(log_name + "_seg1.avi",
		WRITER_CODEC, WRITER_FPS, MFRAME_SIZE, false);
	seg1_t = cv::VideoWriter(log_name + "_seg1_t.avi",
		WRITER_CODEC, WRITER_FPS, MFRAME_SIZE, false);
	seg2 = cv::VideoWriter(log_name + "_seg2.avi",
		WRITER_CODEC, WRITER_FPS, MFRAME_SIZE, false);
	seg3 = cv::VideoWriter(log_name + "_seg3.avi",
		WRITER_CODEC, WRITER_FPS, MFRAME_SIZE, false);
	annotation = cv::VideoWriter(log_name + "_out.avi",
		WRITER_CODEC, WRITER_FPS, MFRAME_SIZE);

	int operation_mode = OP_INIT;
	cv::Mat imgBGR, imgHSV, backproj, foreground, hist, segmented;

	// initialize selection window to default size and position.
	Window select_win = Window(DEFAULT_POSITION, DEFAULT_SIZE);

	// initiate Mixture of Gaussian background subtractor
	BackgroundSubtractor bg = BackgroundSubtractor(50, 24);
	int i = 0; Tracker tracker;
	bool isPaused = false;
	bool next = false;
	bool demo_on = false;
	int frame_count = 0;
	// frame capture loop
	while (stream.isOpened() && operation_mode != OP_EXIT) {
		if (!isPaused || next)
		{
			next = false;
			if (!stream.read(imgBGR) || imgBGR.empty()) {
				throw FailedToRead("FAILURE: cannot read image/frame.");
				return false;
			}
			frame_count++;
			cv::cvtColor(imgBGR, imgHSV, cv::COLOR_BGR2HSV);	// convert the image to hue sturation and value image
			std::string line;
			if (!std::getline(inputs, line))
			{
				std::cout << "Incorrect log format!" << endl;
				return false;
			}

			std::istringstream iss(line); double time; int keyPress;
			if (!(iss >> time >> keyPress))
			{
				std::cout << "Incorrect log format!" << endl;
				return false;
			}


			if (operation_mode == OP_INIT)
			{
				if (i >= INIT_BACK_SUB) operation_mode = OP_SETUP;
				bg.apply_frame(imgBGR, foreground, 0.5);
				i++;
				if (keyPress == ESC_KEY) break;
			}

			// setup stage displays window to capture histogram of hand
			// window is allowed to move in position and change in szie
			else if (operation_mode == OP_SETUP) {
				cv::MatND hist;
				int delta = select_win.outer.size().width / 4.0;// unit size for the change in control
				switch (keyPress)								// controls for the selection phase
				{
				case LEFT_KEY:
					select_win.move(cv::Point(-delta, 0));
					break;
				case RIGHT_KEY:
					select_win.move(cv::Point(delta, 0));
					break;
				case UP_KEY:
					select_win.move(cv::Point(0, -delta));
					break;
				case DOWN_KEY:
					select_win.move(cv::Point(0, delta));
					break;
				case EQUAL_PLUS_KEY:
					select_win.scale(1.1);
					break;
				case MINUS_UNDERSCORE_KEY:
					select_win.scale(0.9);
					break;
				case ENTER_KEY:
					hist = select_win.histogram(imgBGR);
					tracker = Tracker(select_win.inner, hist, bg);
					operation_mode = OP_MAIN;		// advance to tracking mode
					if (DISPLAY_DEMO)
					{
						if (!demo_on)
						{
							demo_on = true;
							std::thread t(graphics::setup, process_argc, process_argv);
							t.detach();
						}
					}
					break;
				case ESC_KEY:
					operation_mode = OP_EXIT;
					break;
				}
				select_win.draw(imgBGR);						// draw the window on the coloured image
			}


			// track the hand's position in real time 
			else if (operation_mode == OP_MAIN) {
				try
				{
					main_record.write(imgBGR);
					r_log.time(time);
					tracker.process_frame(imgBGR, imgHSV, foreground, backproj, segmented);
					annotation.write(imgBGR);
					r_log.next();
				}
				catch (TrackingException& e)
				{
					r_log.message("LOST");
					std::cout << e.what() << endl;
					operation_mode = OP_SETUP;
				}
				switch (keyPress)
				{
				case 'r':
					operation_mode = OP_SETUP;
					break;
				case ESC_KEY:
					operation_mode = OP_EXIT;
					break;
				}
			}
			
		}


		// display the window
		int controls = cv::waitKey(1);
		switch (controls)
		{
		case 'p':
			isPaused = !isPaused;
			break;
		case 'n':
			if (isPaused) next = true;
			break;
		case ESC_KEY:
			return false;
			break;
		default:
			break;
		}
		cv::namedWindow("Tracker", CV_WINDOW_AUTOSIZE);
		cv::imshow("Tracker", imgBGR);
	}
	cout << "Exiting tracker!" << endl;
	cv::destroyAllWindows();
	stream.release();
	seg1.release();
	seg2.release();
	seg3.release();
	annotation.release();
	return true;
}




int main(int argc, char ** argv)
{
	process_argc = argc;
	process_argv = argv;
	if (argc == 1)
	{
		live_run();
	}
	else if (argc == 2)
	{
		record_run(argv[1]);
	}
	else
	{
		std::cout << "Unknown command line input format" << std::endl;
	}
	system("PAUSE");
	return 0;
}

