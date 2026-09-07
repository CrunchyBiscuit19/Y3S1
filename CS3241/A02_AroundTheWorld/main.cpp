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
#define numStars 5
#define faceIndex 1
#define angelIndex 4


enum class Shape
{
	Circle,
	Ellipse,
	Square,
	Rectangle,
	Star,
	Revolver,
	Crewmate,
	KnockedOutFace,
	SmilingFace
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
	float orbitTilt;
	float orbitTiltSpeed;

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
		orbitTilt = 0;
		orbitTiltSpeed = 0;
	}
};

GLfloat PI = 3.14;
GLfloat backgroundColor[3] = { 0.0f, 0.0f, 0.3f };
float alpha = 0.0, k=1;
float tx = 0.0, ty=0.0;
planet planetList[numPlanets];
planet squareList[numSquares];
planet starList[numStars];


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
	glClearColor (backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.0);
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

void drawArc(float radius, float thickness, float startAngle, float endAngle, const GLfloat color[3], float alpha)
{
	float inner = radius - thickness / 2, outer = radius + thickness / 2;

	glColor4f(color[0], color[1], color[2], alpha);

	glBegin(GL_QUAD_STRIP);

		for (int i = 0; i <= circleSegments; i++)
		{
			float angle = startAngle + (endAngle - startAngle) * i / circleSegments;
			glVertex2f(inner * cos(angle), inner * sin(angle));
			glVertex2f(outer * cos(angle), outer * sin(angle));
		}

	glEnd();
}

void drawCross(float length, float thickness, const GLfloat color[3], float alpha)
{
	glPushMatrix();
	glRotatef(45, 0, 0, 1);
	drawRectangle(length, thickness, color, alpha);
	drawRectangle(thickness, length, color, alpha);
	glPopMatrix();
}

void drawKnockedOutFace(float radius, const GLfloat color[3], float alpha)
{
	GLfloat featureColor[3] = { 0.1f, 0.1f, 0.1f };

	drawCircle(radius, color, alpha);

	glPushMatrix();
	glTranslatef(-radius * 0.38f, radius * 0.32f, 0);
	drawCross(radius * 0.44f, radius * 0.12f, featureColor, alpha);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(radius * 0.38f, radius * 0.32f, 0);
	drawCross(radius * 0.44f, radius * 0.12f, featureColor, alpha);
	glPopMatrix();

	// frown: upper half of an arc centred below the mouth, so the corners point down
	glPushMatrix();
	glTranslatef(0, -radius * 0.60f, 0);
	drawArc(radius * 0.42f, radius * 0.12f, PI / 9, 8 * PI / 9, featureColor, alpha);
	glPopMatrix();
}

void drawSmilingFace(float radius, const GLfloat color[3], float alpha)
{
	GLfloat featureColor[3] = { 0.1f, 0.1f, 0.1f };

	drawCircle(radius, color, alpha);

	glPushMatrix();
	glTranslatef(-radius * 0.35f, radius * 0.30f, 0);
	drawCircle(radius * 0.11f, featureColor, alpha);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(radius * 0.35f, radius * 0.30f, 0);
	drawCircle(radius * 0.11f, featureColor, alpha);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -radius * 0.10f, 0);
	drawArc(radius * 0.45f, radius * 0.11f, PI + PI / 6, 2 * PI - PI / 6, featureColor, alpha);
	glPopMatrix();
}

void drawEllipseRing(float outerX, float outerY, float innerX, float innerY, const GLfloat color[3], float alpha)
{
	glBegin(GL_QUAD_STRIP);

		for (int i = 0; i <= circleSegments; i++)
		{
			float angle = 2 * PI * i / circleSegments;
			shadedVertex(innerX * cos(angle), innerY * sin(angle), color, alpha);
			shadedVertex(outerX * cos(angle), outerY * sin(angle), color, alpha);
		}

	glEnd();
}

void drawHalo(float radiusX, float radiusY, float thickness, const GLfloat color[3], float alpha)
{
	drawEllipseRing(radiusX, radiusY, radiusX - thickness, radiusY - thickness, color, alpha);
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
		case Shape::KnockedOutFace:
			drawKnockedOutFace(p.size, p.color, p.alpha);
			break;
		case Shape::SmilingFace:
			drawSmilingFace(p.size, p.color, p.alpha);
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

	// knocked-out face on elliptical orbit
	planetList[1].distFromRef = 6.0;
	planetList[1].angularSpeed = 2.5;
	planetList[1].color[0] = 0.95;
	planetList[1].color[1] = 0.85;
	planetList[1].color[2] = 0.25;
	planetList[1].size = 1.0;
	planetList[1].shape = Shape::KnockedOutFace;
	planetList[1].orbitAspect = 0.5;
	planetList[1].orbitTiltSpeed = 1.5;
    planetList[1].spinSpeed = 7;

	// smiling face with a bobbing halo
	planetList[4].distFromRef = 3.2;
	planetList[4].angularSpeed = -4.0;
	planetList[4].color[0] = 0.98;
	planetList[4].color[1] = 0.80;
	planetList[4].color[2] = 0.62;
	planetList[4].size = 0.75;
	planetList[4].shape = Shape::SmilingFace;
	planetList[4].orbitAspect = 1.6;
	planetList[4].orbitTiltSpeed = -2.0;
	planetList[4].spinSpeed = 9;		// drives the halo bob

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

void getOrbitPosition(const planet &p, float &orbitX, float &orbitY)
{
	float orbitAngle = p.angle * PI / 180;
	float localX = p.orbitCenterX + p.distFromRef * -sin(orbitAngle);
	float localY = p.orbitCenterY + p.distFromRef * p.orbitAspect * cos(orbitAngle);

	// spin the orbit path itself clockwise about the reference point
	float tilt = p.orbitTilt * PI / 180;
	orbitX = localX * cos(tilt) + localY * sin(tilt);
	orbitY = -localX * sin(tilt) + localY * cos(tilt);
}

void drawOrbitingBody(const planet &p)
{
	if (p.size <= 0)
		return;

	float orbitX, orbitY;
	getOrbitPosition(p, orbitX, orbitY);

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
		squareList[i].spinSpeed = (i % 2 == 0 ? -1 : 1) * (3.0f + i);
	}
}

void drawFaceSystem(const planet &face)
{
	if (face.size <= 0)
		return;

	float orbitX, orbitY;
	getOrbitPosition(face, orbitX, orbitY);

	glPushMatrix();
	glTranslatef(orbitX, orbitY, 0);

		glPushMatrix();
		glRotatef(12.0f * sin(face.spin * PI / 180), 0, 0, 1);	// dazed head-loll
		drawPlanet(face);
		glPopMatrix();

		for (int i = 0; i < numStars; i++)
		{
			const planet &star = starList[i];
			float starAngle = star.angle * PI / 180;

			glPushMatrix();
			glTranslatef(star.distFromRef * -sin(starAngle), star.distFromRef * cos(starAngle), 0);
			glRotatef(star.spin, 0, 0, 1);
			drawPlanet(star);
			glPopMatrix();
		}

	glPopMatrix();
}

void drawAngelSystem(const planet &angel)
{
	if (angel.size <= 0)
		return;

	GLfloat haloColor[3] = { 1.0f, 0.88f, 0.35f };

	float orbitX, orbitY;
	getOrbitPosition(angel, orbitX, orbitY);

	glPushMatrix();
	glTranslatef(orbitX, orbitY, 0);

		drawPlanet(angel);

		float bob = angel.size * 0.10f * sin(angel.spin * PI / 180);

		glPushMatrix();
		glTranslatef(0, angel.size * 1.30f + bob, 0);
		drawHalo(angel.size * 0.70f, angel.size * 0.26f, angel.size * 0.09f, haloColor, angel.alpha);
		glPopMatrix();

	glPopMatrix();
}

void generateStars()
{
	const int paletteSize = 3;
	GLfloat palette[paletteSize][3] =
	{
		{ 1.000f, 0.960f, 0.600f },
		{ 1.000f, 0.750f, 0.850f },
		{ 0.700f, 0.900f, 1.000f }
	};

	for (int i = 0; i < numStars; i++)
	{
		starList[i].distFromRef = 1.9f;
		starList[i].angularSpeed = 6.0f;
		starList[i].color[0] = palette[i % paletteSize][0];
		starList[i].color[1] = palette[i % paletteSize][1];
		starList[i].color[2] = palette[i % paletteSize][2];
		starList[i].size = 0.30f;
		starList[i].shape = Shape::Star;
		starList[i].angle = i * 360.0f / numStars;
		starList[i].spinSpeed = 8;
	}
}

void updateStars()
{
	const planet &face = planetList[faceIndex];

	float faceX, faceY;
	getOrbitPosition(face, faceX, faceY);

	float range = face.distFromRef;
	float normalizedY = (range > 0) ? (faceY + range) / (2 * range) : 0.5f;

	if (normalizedY < 0) normalizedY = 0;
	if (normalizedY > 1) normalizedY = 1;

	float speedFactor = 0.25f + 1.75f * normalizedY;

	for (int i = 0; i < numStars; i++)
	{
		starList[i].alpha = face.alpha;
		starList[i].angle -= starList[i].angularSpeed * speedFactor * timer;	// clockwise
		starList[i].spin += starList[i].spinSpeed * timer;
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
	{
		if (i == faceIndex)
			drawFaceSystem(planetList[i]);
		else if (i == angelIndex)
			drawAngelSystem(planetList[i]);
		else
			drawOrbitingBody(planetList[i]);
	}

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
			planetList[i].orbitTilt += planetList[i].orbitTiltSpeed*timer;
		}

		for(int i=0;i<numSquares;i++)
		{
			squareList[i].alpha = 1;
			squareList[i].angle += squareList[i].angularSpeed*timer;
			squareList[i].spin += squareList[i].spinSpeed*timer;
			squareList[i].orbitTilt += squareList[i].orbitTiltSpeed*timer;
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
			planetList[i].orbitTilt += planetList[i].orbitTiltSpeed*timer;
		}

		for (int i = 0; i < numSquares; i++)
		{
			squareList[i].alpha = 1;
			squareList[i].angle = ((float)timeinfo->tm_sec  )*6;
			squareList[i].spin += squareList[i].spinSpeed*timer;
			squareList[i].orbitTilt += squareList[i].orbitTiltSpeed*timer;
		}
	}

	updateStars();

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
	
		case 'w': ty += 0.5f; break;
		case 's': ty -= 0.5f; break;
		case 'a': tx -= 0.5f; break;
		case 'd': tx += 0.5f; break;

		case 'z': alpha += 5.0f; break;
		case 'x': alpha -= 5.0f; break;

		case '=':
		case '+': k *= 1.1f; break;
		case '-':
		case '_': k /= 1.1f; break;

		case 'r':
			k = 1; tx = ty = alpha = 0;
			break;

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
	cout<<"Pan: W/A/S/D\n";
	cout<<"Rotate: Z/X\n";
	cout<<"Zoom: +/-\n";
	cout<<"Reset View: R\n";
    cout<<"Exit: ESC or q/Q\n\n";
	cout << "Current Mode: Solar mode." << endl;


	generatePlanets();
	generateSquares();
	generateStars();

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
