// CS6610Proj1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <math.h>
#define M_PI 3.141592653589793238462643383279502884L /* pi */
#include <vector>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "CyCodeBase/cyCore.h"
#include "CyCodeBase/cyVector.h"
#include "CyCodeBase/cyTriMesh.h"
#include "CyCodeBase/cyMatrix.h"
#include "CyCodeBase/cyGL.h"
#include <numbers>


static void drawNewFrame();
static void keyboardInterrupt(unsigned char key, int x, int y);
static void mouseButtonTracker(int button, int state, int x, int y);
static void mouseClickDrag(int x, int y);
static void idleCallback();
static void CreateVBO(UINT& buffer, const std::vector<cy::Vec3f> vertices);
static void CreateVAO(UINT& vao, UINT& vbo, const std::vector<cy::Vec3f> vertices);
static float DEG2RAD(float degrees);

bool leftMouse, rightMouse;
int mouseX, mouseY;
int width, height;
cy::GLSLProgram prog;
int numVertices;
int numElem;


int main(int argc, char* argv[])
{

    /**
    *
    * Initializations
    *
    **/
    leftMouse = false; rightMouse = false;
    mouseX = 0; mouseY = 0;
    numVertices = 0, numElem = 0;

    // check for obj  file in arguments - this is required for this program
    cy::TriMesh mesh;
    if (argc >= 2)
    {
        std::cout << "Printing filepath: " << argv[1] << "\n";
        mesh.LoadFromFileObj(argv[1], false);
    }
    else
    {
        std::cout << "ERROR: no .obj file given to render.\nexiting...\n";
        exit(1);
    }

    // Initialize FreeGLUT
    glutInit(&argc, argv);
    glutInitContextVersion(4, 5);
    glutInitContextFlags(GLUT_DEBUG);

    // Create a window
    width = 1280;
    height = 720;

    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("CS 5610 Project 2 - Transformations");

    const char* versionGL = (const char*)glGetString(GL_VERSION);
    std::cout << "Current OpenGL version: " << versionGL << "\n";

    //initialize glew
    GLenum res = glewInit();
    // Error code sourced from: https://youtu.be/6dtqg0r28Yc
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    CY_GL_REGISTER_DEBUG_CALLBACK; 

    /**
    *
    * Register functiuons for GLUT
    *
    **/
    glutDisplayFunc(drawNewFrame);
    glutKeyboardFunc(keyboardInterrupt);
    glutIdleFunc(idleCallback);
    glutMouseFunc(mouseButtonTracker);
    glutMotionFunc(mouseClickDrag);

    // OpenGL initializations
    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f; // sourced from: https://youtu.be/6dtqg0r28Yc
    glClearColor(Red, Green, Blue, Alpha);

    // Initalize buffers
    GLuint vbo;
    GLuint vao;
    GLuint ebuffer;
    GLuint nbuffer;

    // initalize vertices
    numVertices = mesh.NV();
    std::cout << "numVecs: " << numVertices << "\n";
    // A vector of vertices of the mesh
    std::vector<cy::Vec3f> vertices = {};
    for (int i = 0; i < numVertices; i++)
    {
        vertices.push_back(mesh.V(i));
        // std::cout << "index: " << i << "\n";
    }

    // initalize elements
    // A vector of triangular elements of the mesh
    numElem = mesh.NF();
    std::cout << "numElem: " << numElem << "\n";
    std::vector<cy::TriMesh::TriFace> elem = {};
    for (int i = 0; i < numElem; i++)
    {
        elem.push_back(mesh.F(i));
    }
    // multiply numElem to account for the mesh returning the number of faces, not values
    numElem *= 3;

    // initalize normals


    // create VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    std::cout << "waiting for buffer...\n";
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(cy::Vec3f), &vertices[0], GL_STATIC_DRAW);
    std::cout << "buffer complete.\n";
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

    // creat element buffer
    glGenBuffers(1, &ebuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elem.size() * sizeof(cy::TriMesh::TriFace), &elem[0], GL_STATIC_DRAW);

    // create normal buffer (for vertex shader)

    

    // Create textures
    
    // Compile shaders
    // initialize CyGL
    cy::GLSLProgram prog;
    prog.BuildFiles("Shaders\\shader.vert", "Shaders\\shader.frag");

    // Other initializations
    prog["mvp"] = cy::Matrix4f(0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    // call draw function
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prog.Bind();
    std::cout << "waiting to draw...\n";
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebuffer);
    glDrawElements(GL_TRIANGLES, numElem, GL_UNSIGNED_INT, 0);
    glutSwapBuffers();

    // Call main loop
    glutMainLoop();
    return 0;
}

/**
* 
* GLUT Display Callback Function - Do not directly call.
* 
**/
void drawNewFrame()
{
    // call draw function
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // std::cout << "drawing new...\n";
    glDrawElements(GL_TRIANGLES, numElem, GL_UNSIGNED_INT, 0);
    glutSwapBuffers();
    return;
}

/**
*
* GLUT Keyboard Interrupt Callback Function - Do not directly call.
* 
* param:
* key - ascii value of key pressed
* x - x coordinate of mouse pointer at time of press
* y - y coordinate of mouse pointer at time of press
*
**/
void keyboardInterrupt(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
        std::cout << "User pressed escape key.\n";
        glutLeaveMainLoop();
        break;
    }
    return;
}

/**
*
* GLUT Keyboard Interrupt Callback Function - Do not directly call.
*
* param:
* key - ascii value of key pressed
* x - x coordinate of mouse pointer at time of press
* y - y coordinate of mouse pointer at time of press
*
**/
void mouseButtonTracker(int button, int state, int x, int y)
{
    switch (button) {
    case 0:
        leftMouse = !state;
        break;
    case 2:
        rightMouse = !state;
        break;
    }
    return;
}

/**
*
* GLUT Keyboard Interrupt Callback Function - Do not directly call.
*
* param:
* key - ascii value of key pressed
* x - x coordinate of mouse pointer at time of press
* y - y coordinate of mouse pointer at time of press
*
**/
void mouseClickDrag(int x, int y)
{
    mouseX = x;
    mouseY = y;
    // std::cout << "X: " << x << " Y: " << y << "\n";
    return;
}

/**
*
* GLUT Idle Callback Function - Do not directly call.
**/
void idleCallback()
{

    static int prevMouseX = mouseX;
    static int prevMouseY = mouseY;

    // check to see if there has been a significant jump
    if (abs(mouseX - prevMouseX) > 30 || abs(mouseY - prevMouseY) > 30)
    {
        prevMouseX = mouseX;
        prevMouseY = mouseY;
    }

    static float yRot = 0;
    static float xRot = 0;
    static float zRot = 0;
    static float distance = 1.0f;

    // true means camera is right side up
    bool upDir = true;

    if (leftMouse)
    {
        // std::cout << "leftMouse drag.\nChange in Y: " << (prevMouseY - mouseY) << "\n";
        float yDelt = float(mouseY - prevMouseY) / (0.2*height);
        float xDelt = float(mouseX - prevMouseX) / (0.2*width);
        if (xRot < 180.0f)
        {
            yRot += yDelt;
        }
        else
        {
            yRot -= yDelt;
        }
        if (yRot < 180.0f)
        {
            xRot += xDelt;
        }
        else
        {
            xRot -= xDelt;
        }
        // zRot -= float((mouseX - prevMouseX) + (mouseY - prevMouseY)) / 4.0f;
    }
    else if (rightMouse)
    {
        // calculate distance of mouse moved (scale it so that it is easy to use)
        distance += float((mouseX - prevMouseX) + (mouseY - prevMouseY)) / 500.0f;
        // std::cout << "rightMouse drag.\nChange in distance: " << distance << "\n";
    }

    prevMouseY = mouseY;
    prevMouseX = mouseX;

    // clean up xRot and yRot
    if (xRot >= 360.0f)
    {
        xRot = 0.0f;
    }
    if (yRot >= 360.0f)
    {
        yRot = 0.0f;
    }


    // std::cout << "yRot: " << yRot << "\n";
    cy::Matrix3f rotMatrix = cy::Matrix3f::RotationXYZ(yRot, xRot, 0);
    cy::Matrix4f projMatrix = cy::Matrix4f::Perspective(DEG2RAD(40), float(width) / float(height), 0.1f, 1000.0f);
    cy::Matrix4f scaleMatrix = cy::Matrix4f::Scale(cy::Vec3f(distance, distance, distance));
    cy::Matrix4f trans = cy::Matrix4f::Translation(cy::Vec3f(0.0f, 0.0f, -5.5f));
    cy::Vec3f pos = cy::Vec3f(0, 0, 50);
    cy::Matrix4f v = cy::Matrix4f::View(pos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
    cy::Matrix4f mvp = projMatrix * v * rotMatrix * scaleMatrix * trans;

    // recompile shaders, set constants, and bind them
    prog.BuildFiles("Shaders\\shader.vert", "Shaders\\shader.frag");
    prog["mvp"] = mvp;
    prog.Bind();

    // Tell GLUT to redraw
    glutPostRedisplay();
}


/**
* 
* Converts Degrees to Radians
* 
**/
static float DEG2RAD(float degrees)
{
    return degrees * (M_PI / 180.0f);
}