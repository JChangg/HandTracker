#ifndef DEMO_H
#define DEMO_H
#include <GL/glut.h>
#include <math.h>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>

namespace graphics
{



	class Position
	{
	public:
		double x, y, z;
		Position();
		Position(double x, double y, double z);
		void translate(cv::Point& pt, double depth);
	};


	class Cube
	{
	public:
		cv::Point point;
		Position coord;
		double size;
		double angle = 0;
		double x_angle, y_angle, z_angle;
		Cube();
		void scale(double s);
		void rotate(cv::Point& dir);
		void position(cv::Point& pt);
		void draw();
	};

	class Curser
	{
		Cube attached;
	public:
		cv::Point point;
		Position coord;
		bool select, grabbed;
		Curser();
		void position(cv::Point& pt);
		void grab(Cube& cube, bool grabber);
		void draw();
	};



	Cube getcube(); 
	Curser getcurser();


	void setup(int argc, char ** argv);

};


extern graphics::Cube cube;
extern graphics::Curser curser;

#endif // ! DEMO_H
