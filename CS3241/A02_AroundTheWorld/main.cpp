// CS3241Lab1.cpp : Defines the entry point for the console application.
#include <time.h>
#include <chrono>  // Required for std::chrono::milliseconds
#include <cmath>
#include <iostream>
#include <thread>  // Required for std::this_thread::sleep_for
#include <vector>
#define GL_SILENCE_DEPRECATION

#ifdef _WIN32
#include <Windows.h>
#include "GL/glut.h"
#define M_PI 3.141592654

#endif
#ifdef __APPLE__
#include <GLUT/GLUT.h>
#include <OpenGL/gl.h>
#endif

#define NUM_CREWMATES 3
#define NUM_STARS 5

#define CIRCLE_SEGMENTS 50

class Planet;

using DrawBody = void (*)(const Planet&);

enum class ClockHand { Second, Minute, Hour, None };

class Planet {
   public:
    float dist;
    float angularSpeed;
    float angle;
    float orbitAspect;
    float orbitCenterX;
    float orbitCenterY;
    float orbitTilt;
    float orbitTiltSpeed;
    ClockHand clockHand;

    bool speedFromParentHeight;

    float size;
    float aspect;
    float spin;
    float spinSpeed;
    float alpha;
    GLfloat color[3];

    DrawBody draw;

    std::vector<Planet> subplanets;

    Planet() {
        dist = 0;
        angularSpeed = 0;
        angle = 0;
        orbitAspect = 1.0;
        orbitCenterX = 0;
        orbitCenterY = 0;
        orbitTilt = 0;
        orbitTiltSpeed = 0;
        speedFromParentHeight = false;
        size = 0;
        aspect = 1.0;
        spin = 0;
        spinSpeed = 0;
        alpha = 1.0;
        color[0] = color[1] = color[2] = 0;
        draw = nullptr;
        clockHand = ClockHand::None;
    }
};

GLfloat PI = 3.14;
GLfloat backgroundColor[3] = {0.1f, 0.3f, 0.3f};
float alpha = 0.0, k = 1;
float tx = 0.0, ty = 0.0;

std::vector<Planet> planetList;
std::vector<Planet> savedPlanetList;

bool clockMode = false;

time_t seconds = 0;
struct tm* timeinfo;
float timer = 1;

// Helper

void getOrbitPosition(const Planet& p, float& orbitX, float& orbitY) {
    float orbitAngle = p.angle * PI / 180;
    float localX = p.orbitCenterX + p.dist * -sin(orbitAngle);
    float localY =
        p.orbitCenterY + p.dist * p.orbitAspect * cos(orbitAngle);

    // spin the orbit path itself
    float tilt = p.orbitTilt * PI / 180;
    orbitX = localX * cos(tilt) + localY * sin(tilt);
    orbitY = -localX * sin(tilt) + localY * cos(tilt);
}

float parentHeightFactor(const Planet& parent) {
    float parentX, parentY;
    getOrbitPosition(parent, parentX, parentY);

    float range = parent.dist;
    float normalizedY = (range > 0) ? (parentY + range) / (2 * range) : 0.5f;

    if (normalizedY < 0)
        normalizedY = 0;
    if (normalizedY > 1)
        normalizedY = 1;

    return 0.25f + 1.75f * normalizedY;
}

void shadedVertex(float x, float y, const GLfloat color[3], float alpha) {
    if (y < 0)
        glColor4f(color[0], color[1], color[2], alpha);
    else
        glColor4f(color[0] - 0.2f, color[1] - 0.2f, color[2] - 0.2f, alpha);

    glVertex2f(x, y);
}

// Shapes

void drawRectangle(float width, float height, const GLfloat color[3],
                   float alpha) {
    float w = width / 2, h = height / 2;

    glBegin(GL_POLYGON);
    shadedVertex(-w, -h, color, alpha);
    shadedVertex(w, -h, color, alpha);
    shadedVertex(w, h, color, alpha);
    shadedVertex(-w, h, color, alpha);
    glEnd();
}

void drawSquare(float side, const GLfloat color[3], float alpha) {
    drawRectangle(side, side, color, alpha);
}

void drawEllipse(float radiusX, float radiusY, const GLfloat color[3],
                 float alpha) {
    glBegin(GL_POLYGON);

    for (int i = 0; i < CIRCLE_SEGMENTS; i++) {
        float angle = 2 * PI * i / CIRCLE_SEGMENTS;
        shadedVertex(radiusX * cos(angle), radiusY * sin(angle), color, alpha);
    }

    glEnd();
}

void drawCircle(float radius, const GLfloat color[3], float alpha) {
    drawEllipse(radius, radius, color, alpha);
}

void drawStar(float outerRadius, const GLfloat color[3], float alpha) {
    float innerRadius = outerRadius * 0.4f;

    glBegin(GL_TRIANGLE_FAN);
    shadedVertex(0, 0, color, alpha);
    for (int i = 0; i <= 10; i++) {
        float angle = PI / 2 + 2 * PI * i / 10;
        float radius = (i % 2 == 0) ? outerRadius : innerRadius;
        shadedVertex(radius * cos(angle), radius * sin(angle), color, alpha);
    }
    glEnd();
}

void drawArc(float radius, float thickness, float startAngle, float endAngle,
             const GLfloat color[3], float alpha) {
    float inner = radius - thickness / 2, outer = radius + thickness / 2;

    glColor4f(color[0], color[1], color[2], alpha);

    glBegin(GL_QUAD_STRIP);

    for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
        float angle = startAngle + (endAngle - startAngle) * i / CIRCLE_SEGMENTS;
        glVertex2f(inner * cos(angle), inner * sin(angle));
        glVertex2f(outer * cos(angle), outer * sin(angle));
    }

    glEnd();
}

void drawCross(float length, float thickness, const GLfloat color[3],
               float alpha) {
    glPushMatrix();
    glRotatef(45, 0, 0, 1);
    drawRectangle(length, thickness, color, alpha);
    drawRectangle(thickness, length, color, alpha);
    glPopMatrix();
}

void drawEllipseRing(float outerX, float outerY, float innerX, float innerY,
                     const GLfloat color[3], float alpha) {
    glBegin(GL_QUAD_STRIP);

    for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
        float angle = 2 * PI * i / CIRCLE_SEGMENTS;
        shadedVertex(innerX * cos(angle), innerY * sin(angle), color, alpha);
        shadedVertex(outerX * cos(angle), outerY * sin(angle), color, alpha);
    }

    glEnd();
}

void drawHalo(float radiusX, float radiusY, float thickness,
              const GLfloat color[3], float alpha) {
    drawEllipseRing(radiusX, radiusY, radiusX - thickness, radiusY - thickness,
                    color, alpha);
}

// Planets

void drawPlanet(const Planet& p) {
    if (p.size <= 0 || p.draw == nullptr)
        return;

    float orbitX, orbitY;
    getOrbitPosition(p, orbitX, orbitY);

    glPushMatrix();
    glTranslatef(orbitX, orbitY, 0);
    p.draw(p);
    glPopMatrix();
}

void generatePlanets() {
    planetList.clear();

    //The sun
    Planet sun;
    sun.draw = [](const Planet& p) {
        glRotatef(p.spin, 0, 0, 1);
        drawCircle(p.size, p.color, p.alpha);
    };
    sun.dist = 0;
    sun.angularSpeed = 0;
    sun.color[0] = 1.0;
    sun.color[1] = 0.7;
    sun.color[2] = 0.0;
    sun.size = 1.5;
    planetList.push_back(sun);

    // knocked-out face on elliptical orbit
    Planet frown;
    frown.draw = [](const Planet& p) {
        GLfloat featureColor[3] = {0.1f, 0.1f, 0.1f};

        glRotatef(12.0f * sin(p.spin * PI / 180), 0, 0, 1); // head roll

        drawCircle(p.size, p.color, p.alpha);

        glPushMatrix();
        glTranslatef(-p.size * 0.40f, p.size * 0.30f, 0);
        drawCross(p.size * 0.45f, p.size * 0.10f, featureColor, p.alpha);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(p.size * 0.40f, p.size * 0.30f, 0);
        drawCross(p.size * 0.45f, p.size * 0.10f, featureColor, p.alpha);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0, -p.size * 0.60f, 0);
        drawArc(p.size * 0.40f, p.size * 0.10f, PI / 9, 8 * PI / 9, featureColor,
                p.alpha);
        glPopMatrix();

        for (int i = 0; i < p.subplanets.size(); i++) {
            const Planet& s = p.subplanets[i];

            if (s.size <= 0 || s.draw == nullptr)
                continue;

            float orbitX, orbitY;
            getOrbitPosition(s, orbitX, orbitY);

            glPushMatrix();
            glTranslatef(orbitX, orbitY, 0);
            s.draw(s);
            glPopMatrix();
        }
    };
    frown.dist = 6.0;
    frown.angularSpeed = 2.5;
    frown.color[0] = 0.95;
    frown.color[1] = 0.85;
    frown.color[2] = 0.25;
    frown.size = 1.0;
    frown.orbitAspect = 0.5;
    frown.orbitTiltSpeed = 1.5;
    frown.spinSpeed = 7;
    frown.clockHand = ClockHand::Hour;
    int frownIndex = planetList.size();
    planetList.push_back(frown);

    // rugby vertical orbit
    Planet rugby;
    rugby.draw = [](const Planet& p) {
        glRotatef(p.spin, 0, 0, 1);
        drawEllipse(p.size, p.size * p.aspect, p.color, p.alpha);
    };
    rugby.dist = 4.5;
    rugby.angularSpeed = 3;
    rugby.color[0] = 0.60;
    rugby.color[1] = 0.35;
    rugby.color[2] = 0.20;
    rugby.size = 0.8;
    rugby.aspect = 0.55;
    rugby.orbitAspect = 1.7;
    rugby.orbitCenterX = 1.4;
    rugby.orbitCenterY = 0.7;
    rugby.spinSpeed = 12;
    rugby.clockHand = ClockHand::Minute;
    planetList.push_back(rugby);

    // revolver wide, flat orbit
    Planet revolver;
    revolver.draw = [](const Planet& p) {
        GLfloat chamberColor[3] = {p.color[0] * 0.3f, p.color[1] * 0.3f,
                                   p.color[2] * 0.3f};

        glRotatef(p.spin, 0, 0, 1);
        drawCircle(p.size, p.color, p.alpha);

        for (int i = 0; i < 6; i++) {
            float angle = 2 * PI * i / 6;

            glPushMatrix();
            glTranslatef(p.size * 0.55f * cos(angle), p.size * 0.55f * sin(angle),
                         0);
            drawCircle(p.size * 0.25f, chamberColor, p.alpha);
            glPopMatrix();
        }
    };
    revolver.dist = 8;
    revolver.angularSpeed = 2;
    revolver.color[0] = 1.0;
    revolver.color[1] = 0.9;
    revolver.color[2] = 0.4;
    revolver.size = 0.9;
    revolver.orbitAspect = 0.45;
    revolver.spinSpeed = 9;
    revolver.clockHand = ClockHand::Minute;
    planetList.push_back(revolver);

    // smiling face with a bobbing halo
    Planet smile;
    smile.draw = [](const Planet& p) {
        GLfloat featureColor[3] = {0.1f, 0.1f, 0.1f};

        drawCircle(p.size, p.color, p.alpha);

        glPushMatrix();
        glTranslatef(-p.size * 0.35f, p.size * 0.30f, 0);
        drawCircle(p.size * 0.1f, featureColor, p.alpha);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(p.size * 0.35f, p.size * 0.30f, 0);
        drawCircle(p.size * 0.1f, featureColor, p.alpha);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0, -p.size * 0.10f, 0);
        drawArc(p.size * 0.45f, p.size * 0.1f, PI + PI / 6, 2 * PI - PI / 6,
                featureColor, p.alpha);
        glPopMatrix();

        for (int i = 0; i < p.subplanets.size(); i++) {
            const Planet& s = p.subplanets[i];

            if (s.size <= 0 || s.draw == nullptr)
                continue;

            float orbitX, orbitY;
            getOrbitPosition(s, orbitX, orbitY);

            glPushMatrix();
            glTranslatef(orbitX, orbitY, 0);
            s.draw(s);
            glPopMatrix();
        }
    };
    smile.dist = 3.2;
    smile.angularSpeed = -4.0;
    smile.color[0] = 1.00;
    smile.color[1] = 0.80;
    smile.color[2] = 0.60;
    smile.size = 0.75;
    smile.orbitAspect = 1.6;
    smile.orbitTiltSpeed = -2.0;
    smile.spinSpeed = 9;
    smile.clockHand = ClockHand::Hour;

    Planet halo;
    halo.draw = [](const Planet& p) {
        float bob = p.size * 0.15f * sin(p.spin * PI / 180);

        glTranslatef(0, bob, 0);
        drawHalo(p.size, p.size * p.aspect, p.size * 0.15f, p.color, p.alpha);
    };
    halo.dist = 0;
    halo.orbitCenterY = smile.size * 1.30f;
    halo.color[0] = 1.0f;
    halo.color[1] = 0.90f;
    halo.color[2] = 0.35f;
    halo.size = smile.size * 0.70f;
    halo.aspect = 0.35f;
    halo.spinSpeed = 9;
    smile.subplanets.push_back(halo);

    planetList.push_back(smile);

    const int crewmatePaletteSize = 3;
    GLfloat crewmatePalette[crewmatePaletteSize][3] = {{1.000f, 1.000f, 0.750f},
                                                       {0.950f, 0.450f, 0.450f},
                                                       {0.200f, 0.900f, 0.850f}};

    for (int i = 0; i < NUM_CREWMATES; i++) {
        Planet crewmate;

        crewmate.draw = [](const Planet& p) {
            GLfloat visorColor[3] = {0.600f, 0.800f, 0.850f};

            float radius = p.size;
            float body = radius * 2;
            float legW = body * 0.30f;
            float legH = body * 0.35f;
            float packW = body * 0.30f;
            float packH = body * 0.60f;

            glRotatef(p.spin, 0, 0, 1);
            glTranslatef(0, radius * 0.85f, 0);

            glPushMatrix();
            glTranslatef(-(body / 2 + packW / 2), -radius, 0);
            drawRectangle(packW, packH, p.color, p.alpha);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(-body * 0.25f, -body - legH / 2, 0);
            drawRectangle(legW, legH, p.color, p.alpha);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(body * 0.25f, -body - legH / 2, 0);
            drawRectangle(legW, legH, p.color, p.alpha);
            glPopMatrix();

            drawCircle(radius, p.color, p.alpha);

            glPushMatrix();
            glTranslatef(0, -radius, 0);
            drawSquare(body, p.color, p.alpha);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(body * 0.25f, -body * 0.2f, 0);
            drawRectangle(body * 0.75f, body * 0.3f, visorColor, p.alpha);
            glPopMatrix();
        };
        crewmate.dist = 4.0f + i * 2.f;
        crewmate.angularSpeed = (i % 2 == 0 ? 1 : -1) * (4.0f - i * 0.5f);
        crewmate.color[0] = crewmatePalette[i % crewmatePaletteSize][0];
        crewmate.color[1] = crewmatePalette[i % crewmatePaletteSize][1];
        crewmate.color[2] = crewmatePalette[i % crewmatePaletteSize][2];
        crewmate.size = 0.35f;
        crewmate.angle = i * 360.0f / NUM_CREWMATES;
        crewmate.spinSpeed = (i % 2 == 0 ? -1 : 1) * (3.0f + i);
        crewmate.clockHand = ClockHand::Second;
        planetList.push_back(crewmate);
    }

    const int starPaletteSize = 3;
    GLfloat starPalette[starPaletteSize][3] = {{1.000f, 0.950f, 0.600f},
                                               {1.000f, 0.750f, 0.850f},
                                               {0.700f, 0.900f, 1.000f}};
    auto& frownPlanet = planetList[frownIndex];
    for (int i = 0; i < NUM_STARS; i++) {
        Planet star;
        star.draw = [](const Planet& p) {
            glRotatef(p.spin, 0, 0, 1);
            drawStar(p.size, p.color, p.alpha);
        };
        star.speedFromParentHeight = true;
        star.dist = 1.9f;
        star.angularSpeed = -6.0f;  // clockwise
        star.color[0] = starPalette[i % starPaletteSize][0];
        star.color[1] = starPalette[i % starPaletteSize][1];
        star.color[2] = starPalette[i % starPaletteSize][2];
        star.size = 0.30f;
        star.angle = i * 360.0f / NUM_STARS;
        star.spinSpeed = 8;
        frownPlanet.subplanets.push_back(star);
    }
}

// Clock

const float clockBandRadius[3] = {3.0f, 5.5f, 8.0f};
const float clockBandSpacing = 0.6f;

void applyClockLayout() {
    int bandTotal[3] = {0, 0, 0};
    int bandPlaced[3] = {0, 0, 0};

    for (int i = 0; i < planetList.size(); i++) {
        if (planetList[i].clockHand != ClockHand::None)
            bandTotal[(int)planetList[i].clockHand]++;
    }

    for (int i = 0; i < planetList.size(); i++) {
        Planet& p = planetList[i];

        if (p.clockHand == ClockHand::None)
            continue;

        int band = (int)p.clockHand;
        int k = bandPlaced[band]++;
        float offset = (k - (bandTotal[band] - 1) / 2.0f) * clockBandSpacing;

        p.dist = clockBandRadius[band] + offset;

        p.orbitAspect = 1.0f;
        p.orbitCenterX = 0;
        p.orbitCenterY = 0;
        p.orbitTilt = 0;
        p.orbitTiltSpeed = 0;
    }
}

float clockHandAngle(ClockHand hand, const struct tm* t) {
    switch (hand) {
        case ClockHand::Second:
            return -(t->tm_sec) * 6.0f;
        case ClockHand::Minute:
            return -(t->tm_min + t->tm_sec / 60.0f) * 6.0f;
        case ClockHand::Hour:
            return -(t->tm_hour % 12 + t->tm_min / 60.0f) * 30.0f;
        default:
            return 0.0f;
    }
}

// Others

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-10, 10, -10, 10, -10, 10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void init(void) {
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2],
                 1.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();

    glScalef(k, k, k);
    glTranslatef(tx, ty, 0);
    glRotatef(alpha, 0, 0, 1);

    for (int i = 0; i < (int)planetList.size(); i++)
        drawPlanet(planetList[i]);

    glPopMatrix();
    glFlush();
}

void idle() {
    if (clockMode) {
        // draw your clock here
        seconds = time(NULL);
        timeinfo = localtime(&seconds);
    }

    for (int i = 0; i < planetList.size(); i++) {
        Planet& p = planetList[i];

        p.alpha = 1;

        if (clockMode && p.clockHand != ClockHand::None) {
            p.angle = clockHandAngle(p.clockHand, timeinfo);
        } else {
            p.angle += p.angularSpeed * timer;
        }

        p.spin += p.spinSpeed * timer;
        p.orbitTilt += p.orbitTiltSpeed * timer;

        for (int j = 0; j < p.subplanets.size(); j++) {
            Planet& s = p.subplanets[j];

            float speed = s.angularSpeed;

            if (s.speedFromParentHeight)
                speed *= parentHeightFactor(p);

            s.alpha = 1;
            s.angle += speed * timer;
            s.spin += s.spinSpeed * timer;
            s.orbitTilt += s.orbitTiltSpeed * timer;
        }
    }

    glutPostRedisplay();
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
}

void keyboard(unsigned char key, int x, int y) {
    //keys to control scaling - k
    //keys to control rotation - alpha
    //keys to control translation - tx, ty
    switch (key) {

        case 27:  // ESC to exit
        case 'q':
        case 'Q':
            exit(0);

        case 'w':
            ty += 0.5f;
            break;
        case 's':
            ty -= 0.5f;
            break;
        case 'a':
            tx -= 0.5f;
            break;
        case 'd':
            tx += 0.5f;
            break;

        case 'z':
            alpha += 5.0f;
            break;
        case 'x':
            alpha -= 5.0f;
            break;

        case '=':
        case '+':
            k *= 1.1f;
            break;
        case '-':
        case '_':
            k /= 1.1f;
            break;

        case 'r':
            k = 1;
            tx = ty = alpha = 0;
            break;

        case 't':
            clockMode = !clockMode;
            if (clockMode) {
                savedPlanetList = planetList;
                applyClockLayout();
                std::cout << "Current Mode: Clock mode." << std::endl;
            } else {
                planetList = savedPlanetList;
                std::cout << "Current Mode: Solar mode." << std::endl;
            }
            break;

        default:
            break;
    }
}

int main(int argc, char** argv) {
    std::cout << "CS3241 Lab 2\n\n";
    std::cout << "+++++CONTROL BUTTONS+++++++\n\n";
    std::cout << "Toggle Time Mode: T\n";
    std::cout << "Pan: W/A/S/D\n";
    std::cout << "Rotate: Z/X\n";
    std::cout << "Zoom: +/-\n";
    std::cout << "Reset View: R\n";
    std::cout << "Exit: ESC or q/Q\n\n";
    std::cout << "Current Mode: Solar mode." << std::endl;

    generatePlanets();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(50, 50);
    glutCreateWindow(argv[0]);
    init();
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    //glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}
