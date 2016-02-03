#include "Graphics.h"

#define PI 3.1415926


double delta_x = 0.0, delta_y = 0.0;

double cube_x = 0.0, cube_y = 0.0, scale = 1;

double cursor_x = 0.0, cursor_y = 0.0;



bool cursor_select = false;
bool cube_grabbed = false;

double translate_x(cv::Point& pt)
{
	return (pt.x - 320) / 320.0 * 2.4;
}

double translate_y(cv::Point& pt)
{
	return -(pt.y - 240) / 240.0 * 1.8;
}

void graphics::grab()
{
	cursor_select = true;
	
	delta_x = cube_x - cursor_x;
	delta_y = cube_y - cursor_y;
	double sqdst = delta_x * delta_x + delta_y * delta_y;
	if (sqdst < 0.2 * 0.2 * scale * scale)
	{
		cube_grabbed = true;
	}
}

void graphics::ungrab()
{
	cursor_select = false;
	cube_grabbed = false;
	delta_x = 0;
	delta_y = 0;
	
}


void graphics::move_cube(cv::Point pt)
{
	graphics::move_cursor(pt);
	cube_x = cursor_x + delta_x;
	cube_y = cursor_y + delta_y;
}

volatile void graphics::move_cursor(cv::Point& pt)
{
	cursor_x = translate_x(pt);
	cursor_y = translate_y(pt);

}

void graphics::rescale(double s)
{

}



/* One face per line */
GLfloat		vertices[] =
{
	-0.2, -0.2, -0.2,   -0.2, -0.2,  0.2,   -0.2,  0.2,  0.2,   -0.2,  0.2, -0.2,
	0.2, -0.2, -0.2,    0.2, -0.2,  0.2,    0.2,  0.2,  0.2,    0.2,  0.2, -0.2,
	-0.2, -0.2, -0.2,   -0.2, -0.2,  0.2,    0.2, -0.2,  0.2,    0.2, -0.2, -0.2,
	-0.2,  0.2, -0.2,   -0.2,  0.2,  0.2,    0.2,  0.2,  0.2,    0.2,  0.2, -0.2,
	-0.2, -0.2, -0.2,   -0.2,  0.2, -0.2,    0.2,  0.2, -0.2,    0.2, -0.2, -0.2,
	-0.2, -0.2,  0.2,   -0.2,  0.2,  0.2,    0.2,  0.2,  0.2,    0.2, -0.2,  0.2
};

GLfloat		colors[] =
{
	0, 0, 0,   0, 0, 1,   0, 1, 1,   0, 1, 0,
	1, 0, 0,   1, 0, 1,   1, 1, 1,   1, 1, 0,
	0, 0, 0,   0, 0, 1,   1, 0, 1,   1, 0, 0,
	0, 1, 0,   0, 1, 1,   1, 1, 1,   1, 1, 0,
	0, 0, 0,   0, 1, 0,   1, 1, 0,   1, 0, 0,
	0, 0, 1,   0, 1, 1,   1, 1, 1,   1, 0, 1
};


void drawCursor(GLfloat x, GLfloat y, bool selected, bool grabbed) {
	int i;
	int triangleAmount = 30; //# of triangles used to draw circle
	GLfloat radius = 0.08;
							 //GLfloat radius = 0.8f; //radius
	GLfloat twicePi = 2.0f * PI;
	if (selected && !grabbed) glColor3f(1, 0, 0);
	else if (selected && grabbed) glColor3f(0, 1, 0);
	else glColor3f(1, 1, 1);

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y); // center of circle
	for (i = 0; i <= triangleAmount;i++) {
		glVertex2f(
			x + (radius * cos(i *  twicePi / triangleAmount)),
			y + (radius * sin(i * twicePi / triangleAmount))
			);
	}
	glEnd();
}


/*
** Function called to update rendering
*/
void DisplayFunc(void)
{
	static float alpha = 0;

	/* Clear the buffer, clear the matrix */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	/* A step backward, then spin the cube */
	glTranslatef(0, 0, -10 + 0.2 * scale + 0.1);
	drawCursor(cursor_x, cursor_y, cursor_select, cube_grabbed);
	glLoadIdentity();
	glTranslatef(0, 0, -10);
	glTranslatef(cube_x, cube_y, 0);
	glScalef(scale, scale, scale);
	glRotatef(30, 1, 0, 0);
	glRotatef(alpha, 0, 1, 0);


	/* We have a color array and a vertex array */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glColorPointer(3, GL_FLOAT, 0, colors);

	/* Send data : 24 vertices */
	glDrawArrays(GL_QUADS, 0, 24);


	/* Cleanup states */
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	/* Rotate a bit more */
	alpha = alpha + 0.05;



	/* End */
	glFlush();
	glutSwapBuffers();

	/* Update again and again */
	glutPostRedisplay();
}

/*
** Function called when the window is created or resized
*/
void ReshapeFunc(int width, int height)
{
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(20, width / (float)height, 5, 15);
	glViewport(0, 0, width, height);

	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();
}




void graphics::gl_setup(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutCreateWindow("Spinning cube");

	/* OpenGL settings */
	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);

	/* Declaration of the callbacks */
	glutDisplayFunc(&DisplayFunc);
	glutReshapeFunc(&ReshapeFunc);

	glutMainLoop();

}

/*


int main(int argc, char **argv)
{


	std::thread t(gl_setup, argc, argv);
	t.detach();

	camera();
	return 0;
}
*/

