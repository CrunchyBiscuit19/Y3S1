// CS3241Lab1.cpp : Defines the entry point for the console application.
#include <cmath>
#include <iostream>
#include <time.h>
#include <iostream>
#include <thread> // Required for std::this_thread::sleep_for
#include <chrono> // Required for std::chrono::milliseconds
#define GL_SILENCE_DEPRECATION


#ifdef _WIN32
#include <Windows.h>
#include "GL/glut.h"
#define M_PI 3.141592654

#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/GLUT.h>
#endif

using namespace std;

#define numSquares 3
#define numPlanets 9


enum class Shape
{
	Circle,
	Ellipse,
	Square,
	Rectangle,
	Star,
	Revolver,
	Crewmate
};


class planet
{
public:
	float distFromRef;
	float angularSpeed;
	GLfloat color[3];
	float size;
	float angle;
	float alpha;
	Shape shape;
	float aspect;		
	float orbitAspect;
	float orbitCenterX;
	float orbitCenterY;
	float spin;
	float spinSpeed;

	planet()
	{
		distFromRef = 0;
		angularSpeed = 0;
		color[0] = color[1] = color[2] = 0;
		size = 0;
		angle = 0;
		alpha = 1.0;
		shape = Shape::Circle;
		aspect = 1.0;
		orbitAspect = 1.0;
		orbitCenterX = 0;
		orbitCenterY = 0;
		spin = 0;
		spinSpeed = 0;
	}
};

GLfloat PI = 3.14;
float alpha = 0.0, k=1;
float tx = 0.0, ty=0.0;
planet planetList[numPlanets];
planet squareList[numSquares];


bool clockMode = false;


time_t seconds = 0;
struct tm * timeinfo;
float timer = 1; // this is a dumb way to control the speed of rotations


void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-10, 10, -10, 10, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void init(void)
{
	glClearColor (0.0, 0.0, 0.3, 1.0);
	glShadeModel (GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

#define circleSegments 50

void shadedVertex(float x, float y, const GLfloat color[3], float alpha)
{
	if (y < 0)
		glColor4f(color[0], color[1], color[2], alpha);
	else
		glColor4f(color[0] - 0.2f, color[1] - 0.2f, color[2] - 0.2f, alpha);

	glVertex2f(x, y);
}

void drawRectangle(float width, float height, const GLfloat color[3], float alpha)
{
	float w = width / 2, h = height / 2;

	glBegin(GL_POLYGON);
		shadedVertex(-w, -h, color, alpha);
		shadedVertex( w, -h, color, alpha);
		shadedVertex( w,  h, color, alpha);
		shadedVertex(-w,  h, color, alpha);
	glEnd();
}

void drawSquare(float side, const GLfloat color[3], float alpha)
{
	drawRectangle(side, side, color, alpha);
}

void drawEllipse(float radiusX, float radiusY, const GLfloat color[3], float alpha)
{
	glBegin(GL_POLYGON);

		for (int i = 0; i < circleSegments; i++)
		{
			float angle = 2 * PI * i / circleSegments;
			shadedVertex(radiusX * cos(angle), radiusY * sin(angle), color, alpha);
		}

	glEnd();
}

void drawCircle(float radius, const GLfloat color[3], float alpha)
{
	drawEllipse(radius, radius, color, alpha);
}

void drawStar(float outerRadius, const GLfloat color[3], float alpha)
{
	float innerRadius = outerRadius * 0.4f;

	glBegin(GL_TRIANGLE_FAN);
		shadedVertex(0, 0, color, alpha);
		for (int i = 0; i <= 10; i++)
		{
			float angle = PI / 2 + 2 * PI * i / 10;	
			float radius = (i % 2 == 0) ? outerRadius : innerRadius;
			shadedVertex(radius * cos(angle), radius * sin(angle), color, alpha);
		}
	glEnd();
}

void drawRevolver(float radius, const GLfloat color[3], float alpha)
{
	GLfloat chamberColor[3] = { color[0] * 0.3f, color[1] * 0.3f, color[2] * 0.3f };

	drawCircle(radius, color, alpha);

	for (int i = 0; i < 6; i++)
	{
		float angle = 2 * PI * i / 6;

		glPushMatrix();
		glTranslatef(radius * 0.55f * cos(angle), radius * 0.55f * sin(angle), 0);
		drawCircle(radius * 0.26f, chamberColor, alpha);
		glPopMatrix();
	}
}

void drawCrewmate(float radius, const GLfloat color[3], float alpha)
{
	float body = radius * 2;
	float legW = body * 0.30f;
	float legH = body * 0.35f;
	float packW = body * 0.30f;
	float packH = body * 0.60f;

	glPushMatrix();
	glTranslatef(0, radius * 0.85f, 0);

	glPushMatrix();
	glTranslatef(-(body / 2 + packW / 2), -radius, 0);
	drawRectangle(packW, packH, color, alpha);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-body * 0.25f, -body - legH / 2, 0);
	drawRectangle(legW, legH, color, alpha);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(body * 0.25f, -body - legH / 2, 0);
	drawRectangle(legW, legH, color, alpha);
	glPopMatrix();

	drawCircle(radius, color, alpha);

	glPushMatrix();
	glTranslatef(0, -radius, 0);
	drawSquare(body, color, alpha);
	glPopMatrix();

	GLfloat visorColor[3] = { 0.584f, 0.792f, 0.863f };

	glPushMatrix();
	glTranslatef(body * 0.25f, -body * 0.2f, 0);
	drawRectangle(body * 0.75f, body * 0.3f, visorColor, alpha);
	glPopMatrix();

	glPopMatrix();
}

void drawPlanet(const planet &p)
{
	switch (p.shape)
	{
		case Shape::Circle:
			drawCircle(p.size, p.color, p.alpha);
			break;
		case Shape::Ellipse:
			drawEllipse(p.size, p.size * p.aspect, p.color, p.alpha);
			break;
		case Shape::Square:
			drawSquare(p.size, p.color, p.alpha);
			break;
		case Shape::Rectangle:
			drawRectangle(p.size, p.size * p.aspect, p.color, p.alpha);
			break;
		case Shape::Star:
			drawStar(p.size, p.color, p.alpha);
			break;
		case Shape::Revolver:
			drawRevolver(p.size, p.color, p.alpha);
			break;
		case Shape::Crewmate:
			drawCrewmate(p.size, p.color, p.alpha);
			break;
	}
}

void generatePlanets()
{
	//The sun
	planetList[0].distFromRef = 0;
	planetList[0].angularSpeed = 0;
	planetList[0].color[0] = 1.0;
	planetList[0].color[1] = 0.7;
	planetList[0].color[2] = 0.0;
	planetList[0].size = 1.5;
	planetList[0].shape = Shape::Circle;

	// asteroid vertical orbit
	planetList[2].distFromRef = 4.5;
	planetList[2].angularSpeed = 3;
	planetList[2].color[0] = 0.60;
	planetList[2].color[1] = 0.35;
	planetList[2].color[2] = 0.18;
	planetList[2].size = 0.8;
	planetList[2].shape = Shape::Ellipse;
	planetList[2].aspect = 0.55;
	planetList[2].orbitAspect = 1.7;
	planetList[2].orbitCenterX = 1.4;
	planetList[2].orbitCenterY = 0.7;
	planetList[2].spinSpeed = 12;

	// revolver wide, flat orbit
	planetList[3].distFromRef = 8;
	planetList[3].angularSpeed = 2;
	planetList[3].color[0] = 1.0;
	planetList[3].color[1] = 0.9;
	planetList[3].color[2] = 0.4;
	planetList[3].size = 0.9;
	planetList[3].shape = Shape::Revolver;
	planetList[3].orbitAspect = 0.45;
	planetList[3].spinSpeed = 9;

}

void drawOrbitingBody(const planet &p)
{
	if (p.size <= 0)
		return;

	float orbitAngle = p.angle * PI / 180;
	float orbitX = p.orbitCenterX + p.distFromRef * -sin(orbitAngle);
	float orbitY = p.orbitCenterY + p.distFromRef * p.orbitAspect * cos(orbitAngle);

	glPushMatrix();
	glTranslatef(orbitX, orbitY, 0);
	glRotatef(p.spin, 0, 0, 1);
	drawPlanet(p);
	glPopMatrix();
}

void generateSquares()
{
	const int paletteSize = 3;
	GLfloat palette[paletteSize][3] =
	{
		{ 1.000f, 0.992f, 0.745f },
		{ 0.925f, 0.459f, 0.471f },
		{ 0.220f, 0.886f, 0.867f }
	};

	for (int i = 0; i < numSquares; i++)
	{
		squareList[i].distFromRef = 4.0f + i * 2.f;
		squareList[i].angularSpeed = (i % 2 == 0 ? 1 : -1) * (4.0f - i * 0.5f);
		squareList[i].color[0] = palette[i % paletteSize][0];
		squareList[i].color[1] = palette[i % paletteSize][1];
		squareList[i].color[2] = palette[i % paletteSize][2];
		squareList[i].size = 0.35f;
		squareList[i].shape = Shape::Crewmate;
		squareList[i].angle = i * 360.0f / numSquares;
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glPushMatrix();

	glScalef(k, k, k);	
	glTranslatef(tx, ty, 0);	
	glRotatef(alpha, 0, 0, 1);

	for (int i = 0; i < numPlanets; i++)
		drawOrbitingBody(planetList[i]);

	for (int i = 0; i < numSquares; i++)
		drawOrbitingBody(squareList[i]);


	glPopMatrix();
	glFlush ();
}

void idle()
{
	if(!clockMode)
	{
		
		//animate planets
		for(int i=0;i<numPlanets;i++)
		{
			planetList[i].alpha = 1;
			planetList[i].angle += planetList[i].angularSpeed*timer;
			planetList[i].spin += planetList[i].spinSpeed*timer;
		}

		for(int i=0;i<numSquares;i++)
		{
			squareList[i].alpha = 1;
			squareList[i].angle += squareList[i].angularSpeed*timer;
			squareList[i].spin += squareList[i].spinSpeed*timer;
		}

	}
	else
	{
		// draw your clock here

		seconds = time(NULL);
		timeinfo = localtime(&seconds);

		for (int i = 0; i < numPlanets; i++)
		{
			planetList[i].alpha = 1;
			planetList[i].angle = ((float)timeinfo->tm_sec  )*6;
			planetList[i].spin += planetList[i].spinSpeed*timer;
		}

		for (int i = 0; i < numSquares; i++)
		{
			squareList[i].alpha = 1;
			squareList[i].angle = ((float)timeinfo->tm_sec  )*6;
			squareList[i].spin += squareList[i].spinSpeed*timer;
		}
	}
	
	glutPostRedisplay();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void keyboard (unsigned char key, int x, int y)
{
	//keys to control scaling - k
	//keys to control rotation - alpha
	//keys to control translation - tx, ty
	switch (key) {

        case 27: // press ESC to exit
		case 'q':
		case 'Q':
            exit(0);
	
		case 't':
			clockMode = !clockMode;
			if (clockMode)
				cout << "Current Mode: Clock mode." << endl;
			else
				cout << "Current Mode: Solar mode." << endl;
			break;

		default:
			break;
	}
}

int main(int argc, char **argv)
{
	cout<<"CS3241 Lab 2\n\n";
	cout<<"+++++CONTROL BUTTONS+++++++\n\n";
	cout<<"Toggle Time Mode: T\n";
    cout<<"Exit: ESC or q/Q\n";
	cout << "Current Mode: Clock mode." << endl;


	generatePlanets();
	generateSquares();

	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize (600, 600);
	glutInitWindowPosition (50, 50);
	glutCreateWindow (argv[0]);
	init ();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);	
	//glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}
