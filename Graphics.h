#pragma once
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include	<GL/glut.h>
#include	<math.h>
#include	<thread>
#include	<iostream>
#include	<opencv2\core.hpp>
#include	<opencv2\highgui.hpp>







namespace graphics
{

	void grab();

	void ungrab();

	void move_cube(cv::Point pt);

	volatile void move_cursor(cv::Point& pt);

	void rescale(double s);



	void gl_setup(int argc, char ** argv);

}

#endif // !GRAPHICS_H



