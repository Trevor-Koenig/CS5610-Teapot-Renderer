// CS6610Proj1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <math.h>
#define M_PI 3.141592653589793238462643383279502884L /* pi */
#include <vector>
#include <map>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "CyCodeBase/cyCore.h"
#include "CyCodeBase/cyVector.h"
#include "CyCodeBase/cyTriMesh.h"
#include "CyCodeBase/cyMatrix.h"
#include "CyCodeBase/cyGL.h"
#include "lodepng/lodepng.h"
#include "lodepng/lodepng.cpp"
#include <numbers>


static void drawNewFrame();
static void keyboardInterrupt(unsigned char key, int x, int y);
static void mouseButtonTracker(int button, int state, int x, int y);
static void mouseClickDrag(int x, int y);
static void idleCallback();
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
        std::cout << "Loading .obj file: " << argv[1] << "\n";
        mesh.LoadFromFileObj(argv[1]);
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
    glutCreateWindow("CS 5610 Project 4 - Textures\tTrevor Koenig");
    glEnable(GL_DEPTH_TEST);

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
    GLuint normbuffer;

    
    std::vector<cy::Vec3f> vertices = {};
    std::vector<cy::Vec3f> normals = {};
    std::vector<cy::Vec2f> texCoords = {};
    std::vector<cy::TriMesh::TriFace> elem = {};
    numElem = mesh.NF();
    numVertices = mesh.NV();
    for (int i = 0; i < numElem; i++)
    {
        elem.push_back(mesh.F(i));
        for (int j = 0; j < 3; j++)
        {
            // this is the index of the Vertex corresponding to the current vertex of the face we are on
            int vertexIndex = mesh.F(i).v[j];
            // this is the index of the texture index corresponding to the current vertex of the face we are on
            int textureIndex = mesh.FT(i).v[j];

            // get vertex at corresponding vertex index
            vertices.push_back(mesh.V(vertexIndex));
            // get normal at corresponding vertex index
            normals.push_back(mesh.VN(vertexIndex));
            // get texture position for index at corresponding texture index
            texCoords.push_back(cy::Vec2f(mesh.VT(textureIndex)));
        }
    }
    numElem *= 3;

    // create VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create normal buffer (for vertex shader)
    glGenBuffers(1, &normbuffer);

    // create VBO
    glGenBuffers(1, &vbo);

    // initalize VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(cy::Vec3f), &vertices[0], GL_STATIC_DRAW);
    
    
    // initialize normal buffer
    glBindBuffer(GL_ARRAY_BUFFER, normbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(cy::Vec3f), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

    // creat element buffer
    glGenBuffers(1, &ebuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elem.size() * sizeof(cy::TriMesh::TriFace), &elem[0], GL_STATIC_DRAW);

    // set up vao for the buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, normbuffer);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    

    /**
    * Create textures
    **/
    //attempt to load custom textures from command line, otherwise load default texturess
    std::vector<unsigned char> texture;
    unsigned int width, height;
    cyGLTexture2D tex;

    if (argc > 2)
    {
        // load custom textures
        unsigned int error = lodepng::decode(texture, width, height, argv[2]);
        if (error)
        {
            std::cout << "Texture file not found. Were you messing with this program?\n";
            exit(1);
        }
    }
    else
    {
        unsigned int error = lodepng::decode(texture, width, height, "Textures\\brick\\brick.png");
        if (error)
        {
            std::cout << "Default texture file not found. Were you messing with this program?\n";
            exit(1);
        }
    }

    // initalize VBO
    GLuint txc;
    glGenBuffers(1, &txc);
    glBindBuffer(GL_ARRAY_BUFFER, txc);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(cy::Vec2f), &texCoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(2);
    // create texture buffer now;
    tex.Initialize();
    tex.SetImage(&texture[0], 4, width, height);
    tex.BuildMipmaps();
    tex.Bind(0);
    
    /**
    * 
    * Compile shaders
    * 
    **/
    // initialize CyGL
    cy::GLSLProgram prog;
    prog.BuildFiles("Shaders\\shader.vert", "Shaders\\shader.frag");

    // call draw function
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prog.Bind();
    std::cout << "Waiting to draw...\n";
    glDrawElements(GL_TRIANGLES, numElem, GL_UNSIGNED_INT, 0);
    glutSwapBuffers();

    std::cout << "Finished drawing first frame. Entering main loop\n";
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
    glDrawArrays(GL_TRIANGLES, 0, numElem);
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
        // clamp distance to be above zero or else it appears to start coming closer when it should be moving away
        distance = std::max(0.0f, distance + float((mouseX - prevMouseX) + (mouseY - prevMouseY)) / 500.0f);
        // std::cout << "rightMouse drag.\nChange in distance: " << distance << "\n";
    }

    prevMouseY = mouseY;
    prevMouseX = mouseX;

    // clean up rotation values
    if (xRot >= 360.0f)
    {
        xRot = xRot - 360.0f;
    }
    if (yRot >= 360.0f)
    {
        yRot = yRot - 360.0f;
    }
    if (zRot >= 360.0f)
    {
        zRot = zRot - 360.0f;
    }


    // std::cout << "yRot: " << yRot << "\n";
    cy::Matrix3f rotMatrix = cy::Matrix3f::RotationXYZ(yRot, xRot, zRot);
    cy::Matrix4f scaleMatrix = cy::Matrix4f::Scale(cy::Vec3f(distance, distance, distance));
    cy::Matrix4f trans = cy::Matrix4f::Translation(cy::Vec3f(0.0f, 0.0f, -5.5f));
    cy::Vec3f viewPos = cy::Vec3f(0, 0, 50);
    cy::Matrix4f model = scaleMatrix * rotMatrix * trans;
    cy::Matrix4f view = cy::Matrix4f::View(viewPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
    cy::Matrix4f projMatrix = cy::Matrix4f::Perspective(DEG2RAD(40), float(width) / float(height), 0.1f, 1000.0f);
    cy::Matrix4f mvp = projMatrix * view * model;

    // recompile shaders, set constants, and bind them
    prog.BuildFiles("Shaders\\shader.vert", "Shaders\\shader.frag");
    prog["model"] = model;
    prog["view"] = view;
    prog["projection"] = projMatrix;
    // why does this work? - is it because it moves the light opposite of camera?
    prog["lightPos"] = (rotMatrix.GetInverse()) * cy::Vec3f(0, 100, 0);
    prog["viewPos"] = (rotMatrix.GetInverse()) * viewPos;
    prog["tex"] = 0;
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