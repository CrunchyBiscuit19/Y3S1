

// CS3241 Assignment 2: Let there be light
#include <cmath>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>

#include "GL/glut.h"
#define M_PI 3.141592654
#elif __APPLE__
#include <GLUT/GLUT.h>
#include <OpenGL/gl.h>
#endif

#include <array>
#include <vector>
#include <utility>

using namespace std;

// global variable

bool m_Smooth = FALSE;
bool m_Highlight = FALSE;
GLfloat angle = 0;  /* in degrees */
GLfloat angle2 = 0; /* in degrees */
GLfloat zoom = 1.0;
int mouseButton = 0;
int moving, startx, starty;

#define NO_OBJECT 4;
int current_object = 0;

std::vector<std::pair<std::array<float, 3>, std::vector<int>>> vertexAdjacency;  // positions, and associated face indices
std::vector<std::pair<std::vector<int>, std::vector<int>>> faceAdjacency;        // face vertex indices, and adjacent face indices
std::vector<std::array<float, 3>> faceNormals;
std::vector<std::array<float, 3>> vertexNormals;

using namespace std;

array<float, 3> add(const array<float, 3>& a, const array<float, 3>& b) {
    return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}

array<float, 3> subtract(const array<float, 3>& a, const array<float, 3>& b) {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

array<float, 3> cross(const array<float, 3>& a, const array<float, 3>& b) {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

array<float, 3> normalize(const array<float, 3>& a) {
    float length = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    if (length == 0.0f) return a;
    return {a[0] / length, a[1] / length, a[2] / length};
}

void setupLighting() {
    m_Smooth ? glShadeModel(GL_SMOOTH) : glShadeModel(GL_FLAT);
    glEnable(GL_NORMALIZE);

    // Lights, material properties
    GLfloat ambientProperties[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat diffuseProperties[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat specularProperties[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightPosition[] = {-100.0f, 100.0f, 100.0f, 1.0f};

    glClearDepth(1.0);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientProperties);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseProperties);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularProperties);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.0);

    // Default : lighting
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
}

void drawSphere(double r) {
    glScalef(r, r, r);
    int i, j;
    int n = 20;

    vertexAdjacency.clear();
    faceAdjacency.clear();

    auto vertexIndex = [n](int i, int j) {
        if (j == 0) return 0;
        if (j == n) return 1 + (n - 1) * 2 * n;
        return 1 + (j - 1) * 2 * n + i % (2 * n);
    };

    // Insert vertex positions
    vertexAdjacency.emplace_back(array<float, 3>{0.0f, 0.0f, 1.0f}, vector<int>());
    for (j = 1; j < n; j++) {
        for (i = 0; i < 2 * n; i++) {
            float x = sin(i * M_PI / n) * sin(j * M_PI / n);
            float y = cos(i * M_PI / n) * sin(j * M_PI / n);
            float z = cos(j * M_PI / n);
            vertexAdjacency.emplace_back(array<float, 3>{x, y, z}, vector<int>());
        }
    }
    vertexAdjacency.emplace_back(array<float, 3>{0.0f, 0.0f, -1.0f}, vector<int>());

    // Insert vertex associated face indices and face adjacency
    for (i = 0; i < 2 * n; i++) {
        for (j = 0; j < n; j++) {
            int corners[4] = {vertexIndex(i, j), vertexIndex(i + 1, j), vertexIndex(i + 1, j + 1), vertexIndex(i, j + 1)};
            vector<int> face;
            for (int k = 0; k < 4; k++) {
                if (face.empty() || (corners[k] != face.back() && corners[k] != face.front())) {
                    face.push_back(corners[k]);
                }
            }

            int f = faceAdjacency.size();
            for (int v : face) {
                vertexAdjacency[v].second.push_back(f);
            }

            vector<int> neighbours;
            neighbours.push_back(((i + 2 * n - 1) % (2 * n)) * n + j);
            neighbours.push_back(((i + 1) % (2 * n)) * n + j);
            if (j > 0) neighbours.push_back(i * n + j - 1);
            if (j < n - 1) neighbours.push_back(i * n + j + 1);

            faceAdjacency.emplace_back(face, neighbours);
        }
    }

    // Calculate face normals
    faceNormals.clear();
    for (auto& face : faceAdjacency) {
        array<float, 3>& a = vertexAdjacency[face.first[0]].first;
        array<float, 3>& b = vertexAdjacency[face.first[1]].first;
        array<float, 3>& c = vertexAdjacency[face.first[2]].first;
        faceNormals.push_back(normalize(cross(subtract(b, a), subtract(c, a))));
    }

    // Calculate vertex normals from face normals average
    vertexNormals.clear();
    for (auto& vertex : vertexAdjacency) {
        array<float, 3> sum = {0.0f, 0.0f, 0.0f};
        for (int f : vertex.second) {
            sum = add(sum, faceNormals[f]);
        }
        vertexNormals.push_back(normalize(sum));
    }

    GLfloat mediumBlue[] = {0.0f, 0.0f, 0.804f, 1.0f};

    for (auto& face : faceAdjacency) {
        glBegin(GL_POLYGON);
        for (int v : face.first) {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mediumBlue);
            glNormal3fv(vertexNormals[v].data());
            glVertex3fv(vertexAdjacency[v].first.data());
        }
        glEnd();
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    glTranslatef(0, 0, -6);

    glRotatef(angle2, 1.0, 0.0, 0.0);
    glRotatef(angle, 0.0, 1.0, 0.0);

    glScalef(zoom, zoom, zoom);

    switch (current_object) {
        case 0:
            drawSphere(1);
            break;
        case 1:
            // draw your second primitive object here
            break;
        case 2:
            // draw your first composite object here
            break;
        case 3:
            // draw your second composite object here
            break;
        default:
            break;
    };
    glPopMatrix();
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'p':
        case 'P':
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case 'w':
        case 'W':
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case 'v':
        case 'V':
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
        case 's':
        case 'S':
            m_Smooth = !m_Smooth;
            break;
        case 'h':
        case 'H':
            m_Highlight = !m_Highlight;
            break;

        case '1':
        case '2':
        case '3':
        case '4':
            current_object = key - '1';
            break;

        case 'Q':
        case 'q':
            exit(0);
            break;

        default:
            break;
    }

    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        mouseButton = button;
        moving = 1;
        startx = x;
        starty = y;
    }
    if (state == GLUT_UP) {
        mouseButton = button;
        moving = 0;
    }
}

void motion(int x, int y) {
    if (moving) {
        if (mouseButton == GLUT_LEFT_BUTTON) {
            angle = angle + (x - startx);
            angle2 = angle2 + (y - starty);
        } else {
            zoom += ((y - starty) * 0.001);
        }
        startx = x;
        starty = y;
        glutPostRedisplay();
    }
}

int main(int argc, char** argv) {
    cout << "CS3241 Lab 3" << endl << endl;

    cout << "1-4: Draw different objects" << endl;
    cout << "S: Toggle Smooth Shading" << endl;
    cout << "H: Toggle Highlight" << endl;
    cout << "W: Draw Wireframe" << endl;
    cout << "P: Draw Polygon" << endl;
    cout << "V: Draw Vertices" << endl;
    cout << "Q: Quit" << endl << endl;

    cout << "Left mouse click and drag: rotate the object" << endl;
    cout << "Right mouse click and drag: zooming" << endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("CS3241 Assignment 3");
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    setupLighting();
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(
        /* field of view in degree */ 40.0,
        /* aspect ratio */ 1.0,
        /* Z near */ 1.0,
        /* Z far */ 80.0
    );
    glMatrixMode(GL_MODELVIEW);
    glutMainLoop();

    return 0;
}
