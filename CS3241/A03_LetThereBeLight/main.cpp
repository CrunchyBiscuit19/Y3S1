

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
GLfloat mediumBlue[] = {0.0f, 0.0f, 0.8f, 1.0f};
GLfloat maroon[] = {0.5f, 0.0f, 0.0f, 1.0f};
GLfloat silver[] = {0.75f, 0.75f, 0.75f, 1.0f};
GLfloat gold[] = {1.0f, 0.84f, 0.0f, 1.0f};

GLdouble cameraEye[3];
GLdouble cameraCentre[3];
GLdouble cameraUp[3];
GLdouble nearPlane;
GLdouble farPlane;
GLdouble fovy;

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

float dot(const array<float, 3>& a, const array<float, 3>& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

array<float, 3> negate(const array<float, 3>& a) {
    return {-a[0], -a[1], -a[2]};
}

array<float, 3> normalize(const array<float, 3>& a) {
    float length = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    if (length == 0.0f) return a;
    return {a[0] / length, a[1] / length, a[2] / length};
}

void drawMesh(const GLfloat* colour);

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

void drawSphere(double r, const GLfloat* colour = mediumBlue) {
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

    drawMesh(colour);
}

void drawMobius(double r, const GLfloat* colour = gold) {
    glScalef(r, r, r);
    int i, j;
    int n = 60;
    int m = 8;
    float halfWidth = 0.4f;

    vertexAdjacency.clear();
    faceAdjacency.clear();

    auto vertexIndex = [n, m](int i, int j) {
        if (i == n) return m - j;
        return i * (m + 1) + j;
    };
    auto faceIndex = [n, m](int i, int j) {
        if (i < 0) return (n - 1) * m + (m - 1 - j);
        if (i == n) return m - 1 - j;
        return i * m + j;
    };

    // Insert vertex positions, u goes around the loop and v across the strip with a half twist
    for (i = 0; i < n; i++) {
        for (j = 0; j <= m; j++) {
            float u = 2 * M_PI * i / n;
            float v = -halfWidth + 2 * halfWidth * j / m;
            float x = (1 + v * cos(u / 2)) * cos(u);
            float y = (1 + v * cos(u / 2)) * sin(u);
            float z = v * sin(u / 2);
            vertexAdjacency.emplace_back(array<float, 3>{x, y, z}, vector<int>());
        }
    }

    // Insert vertex associated face indices and face adjacency, the seam joins row j to row m - j
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            vector<int> face = {vertexIndex(i, j), vertexIndex(i + 1, j), vertexIndex(i + 1, j + 1), vertexIndex(i, j + 1)};

            int f = faceAdjacency.size();
            for (int v : face) {
                vertexAdjacency[v].second.push_back(f);
            }

            vector<int> neighbours;
            neighbours.push_back(faceIndex(i - 1, j));
            neighbours.push_back(faceIndex(i + 1, j));
            if (j > 0) neighbours.push_back(faceIndex(i, j - 1));
            if (j < m - 1) neighbours.push_back(faceIndex(i, j + 1));

            faceAdjacency.emplace_back(face, neighbours);
        }
    }

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    drawMesh(colour);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
}

void drawChainLink(double r, const GLfloat* colour = silver) {
    glScalef(r, r, r);
    int i, j, k;
    int straightSteps = 8;
    int curveSteps = 16;
    int m = 16;
    float halfStraight = 0.48f;
    float bendRadius = 0.52f;
    float tubeRadius = 0.2f;

    vertexAdjacency.clear();
    faceAdjacency.clear();

    // Centre line of the tube, a stadium of two straights and two semicircles, with its outward direction
    vector<pair<array<float, 3>, array<float, 3>>> path;
    for (k = 0; k < straightSteps; k++) {
        float y = -halfStraight + 2 * halfStraight * k / straightSteps;
        path.push_back({{bendRadius, y, 0.0f}, {1.0f, 0.0f, 0.0f}});
    }
    for (k = 0; k < curveSteps; k++) {
        float theta = M_PI * k / curveSteps;
        path.push_back({{bendRadius * cos(theta), halfStraight + bendRadius * sin(theta), 0.0f}, {cos(theta), sin(theta), 0.0f}});
    }
    for (k = 0; k < straightSteps; k++) {
        float y = halfStraight - 2 * halfStraight * k / straightSteps;
        path.push_back({{-bendRadius, y, 0.0f}, {-1.0f, 0.0f, 0.0f}});
    }
    for (k = 0; k < curveSteps; k++) {
        float theta = M_PI + M_PI * k / curveSteps;
        path.push_back({{bendRadius * cos(theta), -halfStraight + bendRadius * sin(theta), 0.0f}, {cos(theta), sin(theta), 0.0f}});
    }
    int n = path.size();

    auto vertexIndex = [n, m](int i, int j) { return (i % n) * m + j % m; };
    auto faceIndex = [n, m](int i, int j) { return ((i + n) % n) * m + (j + m) % m; };

    // Insert vertex positions, a circle of radius tubeRadius around each centre line point
    for (i = 0; i < n; i++) {
        array<float, 3>& centre = path[i].first;
        array<float, 3>& outward = path[i].second;
        for (j = 0; j < m; j++) {
            float phi = 2 * M_PI * j / m;
            float x = centre[0] + tubeRadius * cos(phi) * outward[0];
            float y = centre[1] + tubeRadius * cos(phi) * outward[1];
            float z = tubeRadius * sin(phi);
            vertexAdjacency.emplace_back(array<float, 3>{x, y, z}, vector<int>());
        }
    }

    // Insert vertex associated face indices and face adjacency, wrapping both along and around the tube
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            vector<int> face = {vertexIndex(i, j), vertexIndex(i + 1, j), vertexIndex(i + 1, j + 1), vertexIndex(i, j + 1)};

            int f = faceAdjacency.size();
            for (int v : face) {
                vertexAdjacency[v].second.push_back(f);
            }

            vector<int> neighbours = {faceIndex(i - 1, j), faceIndex(i + 1, j), faceIndex(i, j - 1), faceIndex(i, j + 1)};

            faceAdjacency.emplace_back(face, neighbours);
        }
    }

    drawMesh(colour);
}

void drawCylinder(double r, const GLfloat* colour = maroon) {
    glScalef(r, r, r);
    int i;
    int n = 32;
    float halfHeight = 1.25f;

    vertexAdjacency.clear();
    faceAdjacency.clear();

    int sideBottom = 0;
    int sideTop = n;
    int capBottom = 2 * n;
    int capBottomCentre = 3 * n;
    int capTop = 3 * n + 1;
    int capTopCentre = 4 * n + 1;

    // Insert vertex positions
    for (float y : {-halfHeight, halfHeight}) {
        for (i = 0; i < n; i++) {
            float theta = 2 * M_PI * i / n;
            vertexAdjacency.emplace_back(array<float, 3>{cos(theta), y, -sin(theta)}, vector<int>());
        }
    }
    for (float y : {-halfHeight, halfHeight}) {
        for (i = 0; i < n; i++) {
            float theta = 2 * M_PI * i / n;
            vertexAdjacency.emplace_back(array<float, 3>{cos(theta), y, -sin(theta)}, vector<int>());
        }
        vertexAdjacency.emplace_back(array<float, 3>{0.0f, y, 0.0f}, vector<int>());
    }

    // Insert vertex associated face indices and face adjacency, faces are side quads, then top and bottom cap triangles
    vector<vector<int>> faces;
    for (i = 0; i < n; i++) {
        faces.push_back({sideBottom + i, sideBottom + (i + 1) % n, sideTop + (i + 1) % n, sideTop + i});
    }
    for (i = 0; i < n; i++) {
        faces.push_back({capTopCentre, capTop + i, capTop + (i + 1) % n});
    }
    for (i = 0; i < n; i++) {
        faces.push_back({capBottomCentre, capBottom + (i + 1) % n, capBottom + i});
    }

    for (int f = 0; f < 3 * n; f++) {
        for (int v : faces[f]) {
            vertexAdjacency[v].second.push_back(f);
        }

        int ring = f / n;
        i = f % n;
        vector<int> neighbours = {ring * n + (i + n - 1) % n, ring * n + (i + 1) % n};
        if (ring == 0) {
            neighbours.push_back(n + i);
            neighbours.push_back(2 * n + i);
        } else {
            neighbours.push_back(i);
        }

        faceAdjacency.emplace_back(faces[f], neighbours);
    }

    drawMesh(colour);
}

void drawNunchuck() {
    int k;
    float chainHeight = 0.8f;
    float linkScale = 0.25f;
    float linkPitch = 1.6f * linkScale;
    int linkCount = 7;
    float chainHalfLength = (linkCount - 1) / 2.0f * linkPitch;
    float handleX = chainHalfLength + 0.8f * linkScale;

    for (k = 0; k < linkCount; k++) {
        glPushMatrix();
        glTranslatef(-chainHalfLength + k * linkPitch, chainHeight, 0.0f);
        if (k % 2 == 1) glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
        drawChainLink(linkScale);
        glPopMatrix();
    }

    for (float x : {-handleX, handleX}) {
        glPushMatrix();
        glTranslatef(x, -0.5f, 0.0f);
        glScalef(0.15f, 1.f, 0.15f);
        drawCylinder(1);
        glPopMatrix();
    }
}

void drawAnchor() {
    int k;
    float shankHalfLength = 1.2f;
    float shankRadius = 0.1f;
    float armRadius = 0.1f;
    float arcRadius = 0.9f;
    float arcCentreY = -shankHalfLength + arcRadius;
    float arcStart = -160.0f;
    float arcEnd = -20.0f;
    int armSegments = 10;
    float stockY = 1.f;
    float stockHalfLength = 0.8f;
    float linkScale = 0.3f;
    float linkPitch = 1.5f * linkScale;
    float ringY = shankHalfLength + 0.8f * linkScale;

    glPushMatrix();
    glTranslatef(0.0f, -0.6f, 0.0f);
    glScalef(0.8f, 0.8f, 0.8f);

    glPushMatrix();
    glScalef(shankRadius, shankHalfLength / 1.25f, shankRadius);
    drawCylinder(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, stockY, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.1f, stockHalfLength / 1.25f, 0.1f);
    drawCylinder(1);
    glPopMatrix();

    for (float z : {-stockHalfLength, stockHalfLength}) {
        glPushMatrix();
        glTranslatef(0.0f, stockY, z);
        drawSphere(0.1, gold);
        glPopMatrix();
    }

    float step = (arcEnd - arcStart) / armSegments;
    float segmentHalfLength = arcRadius * sin(step / 2 * M_PI / 180);
    for (k = 0; k < armSegments; k++) {
        float theta = arcStart + (k + 0.5f) * step;
        glPushMatrix();
        glTranslatef(arcRadius * cos(theta * M_PI / 180), arcCentreY + arcRadius * sin(theta * M_PI / 180), 0.0f);
        glRotatef(theta, 0.0f, 0.0f, 1.0f);
        glScalef(armRadius, 1.1f * segmentHalfLength / 1.25f, armRadius);
        drawCylinder(1);
        glPopMatrix();
    }

    glPushMatrix();
    glTranslatef(0.0f, -shankHalfLength, 0.0f);
    drawSphere(0.14, maroon);
    glPopMatrix();

    for (float theta : {arcStart, arcEnd}) {
        glPushMatrix();
        glTranslatef(arcRadius * cos(theta * M_PI / 180), arcCentreY + arcRadius * sin(theta * M_PI / 180), 0.0f);
        glRotatef(theta, 0.0f, 0.0f, 1.0f);
        glScalef(0.15f, 0.3f, 0.1f);
        drawSphere(1, gold);
        glPopMatrix();
    }

    for (k = 0; k < 3; k++) {
        glPushMatrix();
        glTranslatef(0.0f, ringY + k * linkPitch, 0.0f);
        if (k % 2 == 1) glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        drawChainLink(linkScale, k == 0 ? gold : silver);
        glPopMatrix();
    }

    glPopMatrix();
}

void drawMesh(const GLfloat* colour) {
    // Calculate face normals. For any 3 vertices abc, normal is (b - a) x (c - a)
    faceNormals.clear();
    for (auto& face : faceAdjacency) {
        array<float, 3>& a = vertexAdjacency[face.first[0]].first;
        array<float, 3>& b = vertexAdjacency[face.first[1]].first;
        array<float, 3>& c = vertexAdjacency[face.first[2]].first;
        faceNormals.push_back(normalize(cross(subtract(b, a), subtract(c, a))));
    }

    // Calculate vertex normals from surrounding face normals' average (a + b + c + ...)/n
    // Face normals are flipped to one side first, since one-sided surfaces like a Mobius strip have no consistent outward
    vertexNormals.clear();
    for (auto& vertex : vertexAdjacency) {
        array<float, 3> sum = {0.0f, 0.0f, 0.0f};
        for (auto& f : vertex.second) {
            array<float, 3> normal = faceNormals[f];
            if (dot(normal, faceNormals[vertex.second[0]]) < 0) normal = negate(normal);
            sum = add(sum, normal);
        }
        for (auto& s : sum) {
            s /= vertex.second.size();
        }
        vertexNormals.push_back(normalize(sum));
    }

    GLfloat highlightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat noSpecular[] = {0.0f, 0.0f, 0.0f, 1.0f};

    if (m_Highlight) {
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, highlightSpecular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64.0f);
    } else {
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, noSpecular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
    }

    for (size_t f = 0; f < faceAdjacency.size(); f++) {
        glBegin(GL_POLYGON);
        if (!m_Smooth) {
            glNormal3fv(faceNormals[f].data());
        }
        for (int v : faceAdjacency[f].first) {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colour);
            if (m_Smooth) {
                array<float, 3> normal = vertexNormals[v];
                if (dot(normal, faceNormals[f]) < 0) normal = negate(normal);
                glNormal3fv(normal.data());
            }
            glVertex3fv(vertexAdjacency[v].first.data());
        }
        glEnd();
    }
}

void updateProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovy, 1.0, nearPlane, farPlane);
    glMatrixMode(GL_MODELVIEW);
}

void setCamera(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ) {
    cameraEye[0] = eyeX;
    cameraEye[1] = eyeY;
    cameraEye[2] = eyeZ;
    cameraCentre[0] = cameraCentre[1] = cameraCentre[2] = 0.0;
    cameraUp[0] = 0.0;
    cameraUp[1] = 1.0;
    cameraUp[2] = 0.0;
    angle = 0;
    angle2 = 0;
    zoom = 1.0;
}

void resetCamera() {
    setCamera(0.0, 0.0, 6.0);
    nearPlane = 1.0;
    farPlane = 80.0;
    fovy = 40.0;
    updateProjection();
}

void bestCamera() {
    switch (current_object) {
        case 0:
            setCamera(-2.5, 2.5, 5.0);
            break;
        case 1:
            setCamera(0.0, 2.5, 5.5);
            break;
        case 2:
            setCamera(2.5, 2.0, 5.2);
            break;
        case 3:
            setCamera(4.5, 1.0, 4.2);
            break;
        default:
            break;
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(cameraEye[0], cameraEye[1], cameraEye[2], cameraCentre[0], cameraCentre[1], cameraCentre[2], cameraUp[0], cameraUp[1], cameraUp[2]);

    glRotatef(angle2, 1.0, 0.0, 0.0);
    glRotatef(angle, 0.0, 1.0, 0.0);

    glScalef(zoom, zoom, zoom);

    glShadeModel(m_Smooth ? GL_SMOOTH : GL_FLAT);

    switch (current_object) {
        case 0:
            drawSphere(1);
            break;
        case 1:
            glPushMatrix();
            glTranslatef(-1.35f, 0.0f, 0.0f);
            drawMobius(0.5);
            glPopMatrix();

            glPushMatrix();
            drawCylinder(0.4);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(1.4f, 0.0f, 0.0f);
            drawChainLink(0.6);
            glPopMatrix();
            break;
        case 2:
            drawNunchuck();
            break;
        case 3:
            drawAnchor();
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

        case 'n':
            nearPlane = max(0.1, nearPlane - 0.25);
            updateProjection();
            break;
        case 'N':
            nearPlane = min(farPlane - 0.25, nearPlane + 0.25);
            updateProjection();
            break;
        case 'f':
            farPlane = max(nearPlane + 0.25, farPlane - 5.0);
            updateProjection();
            break;
        case 'F':
            farPlane += 5.0;
            updateProjection();
            break;
        case 'o':
            fovy = max(5.0, fovy - 5.0);
            updateProjection();
            break;
        case 'O':
            fovy = min(175.0, fovy + 5.0);
            updateProjection();
            break;
        case 'r':
            resetCamera();
            break;
        case 'R':
            bestCamera();
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
    cout << "n/N: Move near plane closer/further" << endl;
    cout << "f/F: Move far plane closer/further" << endl;
    cout << "o/O: Decrease/increase field of view" << endl;
    cout << "r: Reset camera" << endl;
    cout << "R: Best camera view for the current object" << endl;
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

    resetCamera();
    glutMainLoop();

    return 0;
}
