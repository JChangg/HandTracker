#include "Demo.h"
#define PI 3.14159265
#define TRI_COUNT 30

#define DEFAULT_HEIGHT 480
#define DEFAULT_WIDTH 640

bool demo_on = false;

class Position
{
public:
	double x, y, z;
	Position();
	Position(double x, double y, double z);
	void translate(cv::Point& pt, double depth);
	Position operator+(Position other);
	Position operator-(Position other);
};


class Cube
{
public:
	cv::Point point;
	Position coord;
	Position rot_dir;
	double alpha = 1;
	double size;
	double size_scale = 1;
	double x_angle, y_angle, z_angle;
	double r = 1.0, g = 0.0, b = 0.0;
	Cube();
	Cube(double alpha);
	void scale(double s);
	void set_scale();
	void rotate(cv::Point& pt);
	void position(cv::Point& pt);
	void draw();
	void reset();
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

inline double distance_sqred(Position p1, Position p2);
void drawText();
void drawDisk(GLfloat radius);
void drawRing(GLfloat inner, GLfloat outer);
void drawCubeOfCubes(GLfloat size);
void display_each_frame(void);
void reshape_on_resize(int w, int h);
void enable_shader();
bool compare_cubes(Cube& a, Cube& b);

//////////////////////////////////////////////////////////////////




// Set up module level variables.
int width = 640, height = 480;
double fovy = 20; 
StaticState state = StaticState::OPEN;
graphics::cvParams parameters;
Curser curser = Curser();
Cube cube = Cube();
Cube mask = Cube();

bool compare_cubes(Cube& a, Cube& b)
{
	double scale = a.size*a.size_scale/b.size*b.size_scale;
	bool same_size = scale > 0.9 && scale < 1.1;
	Position p = a.coord - b.coord;
	double dist2d = distance_sqred(a.coord, b.coord);
	bool same_pos = dist2d < 0.015 * b.size * b.size_scale;
	return same_size && same_pos;
}

inline double distance_sqred(Position p1, Position p2)
{
	double deltax = p1.x - p2.x;
	double deltay = p1.y - p2.y;
	return deltax * deltax + deltay * deltay;
}


void drawText()
{
	glColor3f(0, 1, 0);
	std::string state_string = staticStateString(state);
	const char *c = state_string.c_str();
	glRasterPos3f(-2.3, 1.5, -10);
	for (; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
	}
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


Position::Position() : x(0), y(0), z(0) {}
Position::Position(double x, double y, double z) : x(x), y(y), z(z) {}

void Position::translate(cv::Point& pt, double depth)
{
	double tanx = tan(fovy / 2.0 * width / double(height) * PI / 180.0);
	double tany = tan(fovy / 2.0 * PI / 180.0);
	x = (pt.x - 320.0) / (width/2.0) * tanx * depth;
	y = -(pt.y - 240.0) / (height/2.0) * tany * depth;
}

Position Position::operator+(Position other)
{
	Position p = Position(x + other.x, y + other.y, z + other.z);
	return p;
}

Position Position::operator-(Position other)
{
	Position p = Position(x - other.x, y - other.y, z - other.z);
	return p;
}




Curser::Curser() 
	: coord(Position(0, 0.45, -5)), select(false), grabbed(false){}

void Curser::position(cv::Point& pt)
{
	point = pt;
	coord.translate(pt, -coord.z);
	if (select) cube.position(pt);
}

void Curser::grab(Cube& cube, bool grabber)
{
	grabbed = grabber;
	if (grabbed)
	{
		Position pos = this->coord;
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

void Curser::draw()
{
	glPushMatrix();
	glTranslatef(coord.x, coord.y, coord.z);

	if (grabbed && !select) glColor3f(1, 1, 0);
	else glColor3f(0, 1, 0);
	
	if (grabbed) drawDisk(0.06);
	else drawRing(0.04, 0.06);
	glPopMatrix();
}



Cube::Cube() 
	: coord(Position(0, 0, -10)), size(0.5){}

Cube::Cube(double alpha) 
	: coord(Position(0, 0, -10)), size(0.5), alpha(alpha)
{}

void Cube::scale(double s)
{
	size_scale = s;
}


void Cube::set_scale()
{
	size = size_scale * size;
	size_scale = 1.0;
}


void Cube::rotate(cv::Point& pt)
{	
	double sensitivity = 80;

	Position pos = Position();
	pos.translate(pt, -10);


	x_angle = (pos.x * 20);
	y_angle = (pos.y * 20);

}

void Cube::position(cv::Point& pt)
{
	coord.translate(pt, 10);
	point = pt;
}

void Cube::draw()
{
	
	glColor4f(r, g, b, alpha);
	glPushMatrix();
	glTranslatef(coord.x, coord.y, -10);
	glRotatef(x_angle, 0, -1, 0);
	glRotatef(y_angle, 1, 0, 0);
	glRotatef(z_angle, 0, 0, 1);
	drawCubeOfCubes(size*size_scale);	
	glPopMatrix();
}

void Cube::reset()
{
	size = 0.5;
	size_scale = 1;
	x_angle = 0; y_angle = 0; z_angle = 0;
	coord = Position(0, 0, -10);
}






/*
** Function called to update rendering
*/
void display_each_frame(void)
{

	if (DEMO_TARGET && compare_cubes(cube, mask))
	{
		cube.r = 0; cube.g = 1; cube.b = 0;
		mask.r = 0; mask.g = 1; mask.b = 0;
	}
	else
	{
		cube.r = 1; cube.g = 0; cube.b = 0;
		mask.r = 1; mask.g = 0; mask.b = 0;
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Clear the buffer, clear the matrix 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// A step backward, then spin the cube
	glLoadIdentity();
	drawText();
	curser.draw();
	cube.draw();
	if (DEMO_TARGET) mask.draw();
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




void graphics::setup(int argc, char ** argv)
{
	if (DEMO_TARGET)
	{
		mask.alpha = 0.4;
		mask.r = 1; mask.g = 1; mask.b = 1;
		mask.size = 1.2;
		mask.coord = Position(0.5, 0.5, 0);
	}
	
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





graphics::cvParams::cvParams()
{
}

graphics::cvParams::cvParams(cv::Point center, cv::Point prev_max, cv::Point prev_min, cv::Point max, cv::Point min)
	: center(center), prev_max(prev_max), prev_min(prev_min), max(max), min(min)
{}





void graphics::updateParams(StaticState newState, cvParams param)
{
	static double distance;
	parameters = param;
	curser.position(param.center);
	double d = param.radius;
	if (state != newState)
	{
		switch (state)
		{
		case STATE::StaticState::OPEN:
			if (newState == STATE::StaticState::CLOSED)
				curser.grab(cube, true);

			if (newState == STATE::StaticState::PINCH)
			{
				if (d > 0) distance = d;
				else newState = STATE::StaticState::OPEN;
			}

			break;
		case STATE::StaticState::CLOSED:
			if (newState == STATE::StaticState::OPEN)
				curser.grab(cube, false);
			break;
		case STATE::StaticState::SCROLL:
			break;
		case STATE::StaticState::PINCH:
			if (newState == STATE::StaticState::OPEN)
				cube.set_scale();
			break;
		case STATE::StaticState::POINTER:
			
			break;
		default:
			break;
		}
		state = newState;
	}
	
	switch (state)
	{
	case STATE::StaticState::OPEN:
		if (DEMO_TARGET && compare_cubes(cube, mask))
		{
			std::cout << "SUCCESS" << std::endl;
		}
		break;
	case STATE::StaticState::CLOSED:
		curser.grab(cube, true);
		break;
	case STATE::StaticState::SCROLL:
		cube.rotate(param.center);
		break;
	case STATE::StaticState::PINCH:
		if (d > 0) 	
			cube.scale((d / distance)*(d / distance));
		break;
	case STATE::StaticState::POINTER:
		cube.reset();
		break;
	default:
		break;
	}
}
