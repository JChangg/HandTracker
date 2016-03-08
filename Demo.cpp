#include "Demo.h"
#define PI 3.14159265
#define TRI_COUNT 30

#define DEFAULT_HEIGHT 480
#define DEFAULT_WIDTH 640

int width = 640, height = 480;
double fovy = 20;


graphics::Curser curser = graphics::Curser();
graphics::Cube cube = graphics::Cube();


inline double distance_sqred(graphics::Position p1, graphics::Position p2)
{
	double deltax = p1.x - p2.x;
	double deltay = p1.y - p2.y;
	return deltax * deltax + deltay * deltay;
}


void drawDisk(GLfloat radius)
{
	GLfloat twicePi = 2.0f * PI;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0, 0);
	for (int i = 0; i <= TRI_COUNT;i++) {
		glVertex2f(
			(radius * cos(i *  twicePi / TRI_COUNT)),
			(radius * sin(i * twicePi / TRI_COUNT))
			);
	}
	glEnd();
}


void drawRing(GLfloat inner, GLfloat outer)
{
	glBegin(GL_QUAD_STRIP);
	for (unsigned int i = 0; i <= TRI_COUNT; i++)
	{
		float angle = (i / (float)TRI_COUNT) * 3.14159f * 2.0f;
		glVertex2f(inner * cos(angle), inner * sin(angle));
		glVertex2f(outer * cos(angle), outer * sin(angle));
	}
	glEnd();
}



void drawCubeOfCubes(GLfloat size)
{
	glPushMatrix();
	glScalef(size / 3.0, size / 3.0, size / 3.0);
	for (int i = -1; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
			for (int k = -1; k <= 1; k++)
			{
				glPushMatrix();
				glTranslatef(i, j, k);
				glutSolidCube(0.8);
				glPopMatrix();
			}
	glPopMatrix();
}


graphics::Position::Position() : x(0), y(0), z(0) {}
graphics::Position::Position(double x, double y, double z) : x(x), y(y), z(z) {}

void graphics::Position::translate(cv::Point& pt, double depth)
{
	double tanx = tan(fovy / 2.0 * width / double(height) * PI / 180.0);
	double tany = tan(fovy / 2.0 * PI / 180.0);
	x = (pt.x - 320.0) / (width/2.0) * tanx * depth;
	y = -(pt.y - 240.0) / (height/2.0) * tany * depth;
}




graphics::Curser::Curser() 
	: coord(Position(0, 0.45, -5)), select(false), grabbed(false){}

void graphics::Curser::position(cv::Point& pt)
{
	point = pt;
	coord.translate(pt, -coord.z);
	if (select) cube.position(pt);
}

void graphics::Curser::grab(Cube& cube, bool grabber)
{
	grabbed = grabber;
	if (grabbed)
	{
		graphics::Position pos = this->coord;
		pos.translate(this->point, -cube.coord.z);
		if (distance_sqred(pos, cube.coord) < cube.size / 2.0)
		{
			select = true;
			attached = cube;
		}
		else select = false;
	}
	else select = false;
}

void graphics::Curser::draw()
{
	glPushMatrix();
	glTranslatef(coord.x, coord.y, coord.z);

	if (grabbed && !select) glColor3f(1, 1, 0);
	else glColor3f(0, 1, 0);
	
	if (grabbed) drawDisk(0.06);
	else drawRing(0.04, 0.06);
	glPopMatrix();
}



graphics::Cube::Cube() 
	: coord(Position(0, 0, -10)), size(0.5){}

void graphics::Cube::scale(double s)
{
	size = s;
}


void graphics::Cube::rotate(cv::Point& dir)
{
	
}

void graphics::Cube::position(cv::Point& pt)
{
	coord.translate(pt, 10);
	point = pt;
}

void graphics::Cube::draw()
{
	if (angle > 360) angle -= 360;
	glColor3f(1, 0, 0);
	glPushMatrix();
	glTranslatef(coord.x, coord.y, -10);
	glRotatef(45, 1, 0, 0);
	glRotatef(angle, 0, 1, 0);
	drawCubeOfCubes(size);	
	glPopMatrix();
	angle += 0.01;
}






/*
** Function called to update rendering
*/
void display_each_frame(void)
{
	// Clear the buffer, clear the matrix 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// A step backward, then spin the cube
	glLoadIdentity();
	curser.draw();
	cube.draw();

	// clear buffers
	glFlush();
	glutSwapBuffers();

	// reUpdate 
	glutPostRedisplay();
}

/*
** Function called when the window is created or resized
*/
void reshape_on_resize(int w, int h)
{
	width = w;
	height = h;

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(fovy, width / (float)height, 5, 15);
	glViewport(0, 0, width, height);

	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();
}



void enable_shader()
{
	const GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	const GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

	const GLfloat mat_ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat high_shininess[] = { 100.0f };

	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
}



graphics::Cube graphics::getcube()
{
	return cube;
}

graphics::Curser graphics::getcurser()
{
	return curser;
}

void graphics::setup(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
	glutCreateWindow("Demo");

	//OpenGL settings 
	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);

	//Declaration of the callbacks
	glutDisplayFunc(&display_each_frame);
	glutReshapeFunc(&reshape_on_resize);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);
	enable_shader();
	glutMainLoop();

}
