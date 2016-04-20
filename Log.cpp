#include "Log.h"
Logger program_log;

double diffclock(clock_t clock1, clock_t clock2) {

	double diffticks = clock1 - clock2;
	double diffms = diffticks / (CLOCKS_PER_SEC / 1000);

	return diffms;
}

Logger::Logger()
{
	std::string name = get_name();
	std::string fname = name + ".txt";
	file.open(fname, std::fstream::out);
	posfile.open(name + "_pos.txt", std::fstream::out);
	start_time = std::chrono::system_clock::now();
}



double Logger::ms_elapsed()
{
	std::chrono::duration<double> elapsed_seconds = frame_time - start_time;
	double ms = elapsed_seconds.count() * 1000;
	return ms;
}

void Logger::frame()
{
	frame_time = std::chrono::system_clock::now();
	file << "["<< ms_elapsed() << "]" << std::endl;
}

void Logger::update_position(cv::Point & pt)
{
	posfile << pt.x << " " << pt.y << std::endl;
}

void Logger::event(std::string msg)
{
	file << msg << std::endl;
}

void Logger::except(HandException & e)
{
	file << e.what() << std::endl;
}




std::string get_name()
{
	struct tm timeinfo;
	time_t rawtime = time(0);
	localtime_s(&timeinfo, &rawtime);
	std::ostringstream sstream;
	sstream << LOGGER_DIR;
	sstream << timeinfo.tm_mon + 1 << "_" << timeinfo.tm_mday << "__";
	sstream << timeinfo.tm_hour << "_" << timeinfo.tm_min << "_";
	sstream << timeinfo.tm_sec ;
	std::string name = sstream.str();
	return name;
}
