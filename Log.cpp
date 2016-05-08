#include "Log.h"
OperationLog op_log;
ResultLog r_log;
double diffclock(clock_t clock1, clock_t clock2) {

	double diffticks = clock1 - clock2;
	double diffms = diffticks / (CLOCKS_PER_SEC / 1000);

	return diffms;
}

Logger::Logger()
{
	name = get_name();
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




double ms_elapsed(LOG_TIME start, LOG_TIME end)
{
	std::chrono::duration<double> elapsed_seconds = end - start;
	double ms = elapsed_seconds.count() * 1000;
	return ms;
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

std::string get_name(std::string name)
{
	return LOGGER_DIR + name;
}

BasicLog::BasicLog()
{
}

BasicLog::BasicLog(std::string name) 
{
	this->name = name;
	file.open(this->name, std::fstream::out);
}

OperationLog::OperationLog()
{
}

OperationLog::OperationLog(std::string name)
	:BasicLog(name + "_op.txt"), key_press(-1)
{
	events = std::vector<bool>(6);
}

void OperationLog::start()
{
	frame_arrival = LOG_GET_TIME();
}

void OperationLog::frame()
{
	LOG_TIME t = LOG_GET_TIME();
	double duration = ms_elapsed(frame_arrival, t);
	frame_arrival = t;
	file << duration;
	file <<" "<< key_press;
	file << std::endl;
	key_press = -1;
}

void OperationLog::key(int keypress)
{
	key_press = keypress;
}

ResultLog::ResultLog()
{
}

ResultLog::ResultLog(std::string name)
	:BasicLog(name+"_r.txt")
{
}

void ResultLog::time(double ms)
{
	file << ms << " ";
}

void ResultLog::message(std::string s)
{
	file << s << " ";
}

void ResultLog::position(cv::Point & p)
{
	file << p.x << " " << p.y;
}

void ResultLog::next()
{
	file << std::endl;
}
