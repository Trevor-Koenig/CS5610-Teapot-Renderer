// CS6610Proj1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <math.h>
#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <numbers>
#include "GLInitializations.h"
#include "CyCodeBase/cyTriMesh.h"
#include "CyCodeBase/cyCore.h"
#include "CyCodeBase/cyVector.h"
#include "CyCodeBase/cyMatrix.h"
#include "CyCodeBase/cyGL.h"
#include "lodepng/lodepng.h"
#include "lodepng/lodepng.cpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"


void createOpenGLWindow(int width, int height);
void drawNewFrame();
void keyboardInterrupt(unsigned char key, int x, int y);
void mouseButtonTracker(int button, int state, int x, int y);
void mouseClickDrag(int x, int y);
void specialInput(int key, int x, int y);
void idleCallback();
void createScenePlane(GLuint& planeVao);
void setRotationAndDistance(float& xRot, float& yRot, float& zRot, float& distance);
float DEG2RAD(float degrees);

bool leftMouse, rightMouse, spacebarToggle;
bool altMouseDrag;
bool useTesselation;
int mouseX, mouseY;
unsigned short int tessLevel;
int windowWidth, windowHeight;
GLuint teapotPlaneVao;
GLuint triangulationVao;
cy::GLSLProgram planeShaders;
cy::GLSLProgram triangulationShaders;
cyGLTexture2D normMap;
cyGLTexture2D dispTex;


int main(int argc, char* argv[])
{

    /**
    *
    * Initializations
    *
    **/
    leftMouse = false; rightMouse = false;
    spacebarToggle = false;
    altMouseDrag = false;
    useTesselation = false;
    mouseX = 0; mouseY = 0;
    tessLevel = 1;

    // if there is not a png normal map then exit - simple check
    if (argc < 2) { exit(0); }

    // Initialize FreeGLUT
    glutInit(&argc, argv);
    glutInitContextVersion(4, 5);
    glutInitContextFlags(GLUT_DEBUG);

    // initalize a new window
    windowWidth = 1280;
    windowHeight = 720;
    createOpenGLWindow(windowWidth, windowHeight);

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
    glutSpecialFunc(specialInput);

    // OpenGL initializations
    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f; // sourced from: https://youtu.be/6dtqg0r28Yc
    glClearColor(Red, Green, Blue, Alpha);

    
    // define all objects to be rendered and their textures
    createScenePlane(teapotPlaneVao);
    createScenePlane(triangulationVao);

    // intialize textures
    unsigned int width = 0; unsigned int height = 0;
    std::vector<unsigned char> normTexture;
    std::cout << argv[1] << std::endl;
    std::string filename = argv[1];

    unsigned error = lodepng::decode(normTexture, width, height, filename);

    // create normal texture buffer now - load the normal map as a texture
    normMap.Initialize();
    normMap.Bind(0);
    normMap.SetImage(&normTexture[0], 4, width, height);
    normMap.BuildMipmaps();
    glUniform1i(glGetUniformLocation(planeShaders.GetID(), "normMap"), 0);

    if (argc > 2)
    {
        std::cout << "Using tessellation shaders." << std::endl;
        // intialize textures
        unsigned int width = 0; unsigned int height = 0;
        std::vector<unsigned char> dispTexture;
        std::cout << argv[2] << std::endl;
        std::string filename = argv[2];

        unsigned error = lodepng::decode(dispTexture, width, height, filename);
        // create displacement texture buffer now - load the depth map as a texture
        dispTex.Initialize();
        dispTex.Bind(1);
        dispTex.SetImage(&dispTexture[0], 4, width, height);
        dispTex.BuildMipmaps();
        glUniform1i(glGetUniformLocation(planeShaders.GetID(), "gDisplacementMap"), 1);

        // since a displacement texture was found we will also use tesselation shaders
        useTesselation = true;
    }

    // useTesselation = false;
    /**
    * 
    * Compile shaders
    * 
    **/
    // initialize CyGL
    
    if (useTesselation)
    {
        planeShaders.BuildFiles("Shaders\\passthrough.vert",
                                "Shaders\\shader.frag",
                                (const char*)nullptr,
                                "Shaders\\shader.tessc",
                                "Shaders\\shader.tesse"
                                );
        triangulationShaders.BuildFiles("Shaders\\passthrough.vert",
                                        "Shaders\\SimpleTexture.frag",
                                        "Shaders\\wire.geom",
                                        "Shaders\\shader.tessc",
                                        "Shaders\\shader.tesse"
                                        );
    }
    else
    {
        planeShaders.BuildFiles("Shaders\\shader.vert", "Shaders\\shader.frag");
        triangulationShaders.BuildFiles("Shaders\\Passthrough.vert", "Shaders\\SimpleTexture.frag", "Shaders\\wire.geom");
    }
    
    // specify patches for tesselations
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    // clear scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    std::cout << "Finished drawing first frame. Entering main loop\n";
    char t;
    //std::cin >> t;
    // Call main loop
    glutMainLoop();
    return 0;
}





/******************************************************************************
*
*
*                               GL CALLBACK FUNCTIONS
*
*
********************************************************************************/


/**
*
* GLUT Display Callback Function - Do not directly call.
*
**/
void drawNewFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // render plane under argument object (also used for testing as a plane to render depth map to)
    glBindVertexArray(teapotPlaneVao);
    planeShaders.Bind();
    if (useTesselation)
    {
        glDrawArrays(GL_PATCHES, 0, 6);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    if (spacebarToggle)
    {
        // draw triangulation plane
        glBindVertexArray(teapotPlaneVao);
        triangulationShaders.Bind();
        if (useTesselation)
        {
            glDrawArrays(GL_PATCHES, 0, 6);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

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
    case 32:
        std::cout << "User pressed spacebar.\n";
        spacebarToggle = !spacebarToggle;
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
    // check for alt button pressed while mouse is moving
    if (glutGetModifiers() == GLUT_ACTIVE_ALT)
    {
        altMouseDrag = !state;
    }

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

/// <summary>
/// method sourced from: https://community.khronos.org/t/what-are-the-codes-for-arrow-keys-to-use-in-glut-keyboard-callback-function/26457
/// </summary>
void specialInput(int key, int x, int y)
{
    int maxTesselation = 100;

    switch (key)
    {
    case GLUT_KEY_LEFT:
        // by casting to unsigned char first we clamp tessLevel between 0 and 255
        tessLevel = cy::Clamp((int)(--tessLevel), 1, maxTesselation);
        std::cout << "Decreased tesselation level to: " << tessLevel << std::endl;
        break;
    case GLUT_KEY_RIGHT:
        tessLevel = tessLevel = cy::Clamp((int)(++tessLevel), 1, maxTesselation);
        std::cout << "Increased tesselation level to: " << tessLevel << std::endl;
        break;
    }
}


/**
*
* GLUT Idle Callback Function - Do not directly call.
**/
void idleCallback()
{
    static float yRot = 0;
    static float xRot = 0;
    static float zRot = 0;
    static float distance = 1.0f;
    static float altYRot = 0;
    static float altXRot = 0;
    static float altZRot = 0;
    static float altDistance = 1.0f;

    // check if the alt button is pressed and call functions accordingly
    if (altMouseDrag)
    {
        setRotationAndDistance(altXRot, altYRot, altZRot, altDistance);
    }
    else
    {
        setRotationAndDistance(xRot, yRot, zRot, distance);
    }



    // std::cout << "yRot: " << yRot << "\n";
    cy::Matrix3f rotMatrix = cy::Matrix3f::RotationXYZ(-xRot, -yRot, zRot);
    cy::Matrix4f scaleMatrix = cy::Matrix4f::Scale(cy::Vec3f(distance, distance, distance));
    // for teapot
    cy::Matrix4f trans = cy::Matrix4f::Translation(cy::Vec3f(0.0f, -50.0f, 0.0f));
    // for teapot
    // cy::Matrix4f trans = cy::Matrix4f::Translation(cy::Vec3f(0.0f, 0.0f, 0.0f));
    cy::Vec3f viewPos = (rotMatrix * cy::Vec3f(0.0f, 0.0f, 100.0f)) * distance;
    cy::Matrix4f view = cy::Matrix4f::View(viewPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
    cy::Matrix4f projMatrix = cy::Matrix4f::Perspective(DEG2RAD(40), float(windowWidth) / float(windowHeight), 0.1f, 1000.0f);

    cy::Vec3f lightPos = cy::Vec3f(0.0f, 100.0f, 0.0f);
    
    // rotate the plane from vertical to horizontal
    cy::Matrix3f planeRotation = cy::Matrix3f::RotationXYZ(DEG2RAD(90.0f), 0.0f, 0.0f);
    // move the plane down to beneath the main scen object (usually a teapot)
    cy::Matrix4f planeTranslation = cy::Matrix4f::Translation(cy::Vec3f(0.0f, -10.0f, 0.0f));
    // define the scale of the teapot to fit the size of the current scene objects
    cy::Matrix4f planeScale = cy::Matrix4f::Scale(cy::Vec3f(0.3f, 0.3f, 0.3f));
    
    planeShaders["vModel"] = planeScale;
    planeShaders["model"] = planeScale;
    planeShaders["view"] = view;
    planeShaders["projection"] = projMatrix;
    planeShaders["lightPos"] = lightPos;
    planeShaders["camPos"] = viewPos;

    // define matrix to move the plane slightly forward
    cy::Matrix4f bringForward = cy::Matrix4f::Translation(cy::Vec3f(0.0f, 0.0f, 0.1f));
    triangulationShaders["vModel"] = bringForward * planeScale;
    triangulationShaders["model"] = bringForward * planeScale;
    triangulationShaders["view"] = view;
    triangulationShaders["projection"] = projMatrix;

    if (useTesselation)
    {
        planeShaders["tessLevel"] = (float)tessLevel;
        triangulationShaders["tessLevel"] = (float)tessLevel;
    }

    // Tell GLUT to redraw
    glutPostRedisplay();
}






/******************************************************************************
* 
* 
*                               HELPER FUNCTIONS
* 
* 
********************************************************************************/


/**
*
* Given an integer width and height of the window this method
* will create a new OpenGL window
* 
*/
void createOpenGLWindow(int width, int height)
{
    // Create a window
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 50);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("CS 5610 Project 8 - Tessellation\tTrevor Koenig");
    glEnable(GL_DEPTH_TEST);

    const char* versionGL = (const char*)glGetString(GL_VERSION);
    std::cout << "Current OpenGL version: " << versionGL << "\n";
}


/// <summary>
/// Readies the linked Vao to be rendered as a plane in the scene
/// </summary>
void createScenePlane(GLuint& planeVao)
{
    GLuint planeVbo;
    GLuint planeNBuffer;
    GLuint planeEBuffer;
    GLuint planeTxc;

    // define plane positioning
    float planeVert[] = {
        100.0,  100.0, 0.0,
       -100.0,  100.0, 0.0,
        100.0, -100.0, 0.0,
       -100.0,  100.0, 0.0,
       -100.0, -100.0, 0.0,
        100.0, -100.0, 0.0,
    };

    // define plane normals
    float planeNorms[] = {
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
    };

    // define plane faces
    int planeFaces[] = {
        0, 1, 2,
        3, 4, 5,
    };

    // define texture coordinates
    float planeTxcArray[] = {
        1.0, 0.0,
        0.0, 0.0,
        1.0, 1.0,
        0.0, 0.0,
        0.0, 1.0,
        1.0, 1.0,
    };

    // create plane plane VAO and vbo
    glGenVertexArrays(1, &planeVao);
    glBindVertexArray(planeVao);
    glGenBuffers(1, &planeVbo);
    glBindBuffer(GL_ARRAY_BUFFER, planeVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVert), &planeVert[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // create plane normal buffer
    glGenBuffers(1, &planeNBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, planeNBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeNorms), &planeNorms[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(1);

    // create plane element buffer
    glGenBuffers(1, &planeEBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeFaces), planeFaces, GL_STATIC_DRAW);

    // create texture coordinates buffer
    glGenBuffers(1, &planeTxc);
    glBindBuffer(GL_ARRAY_BUFFER, planeTxc);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeTxcArray), &planeTxcArray[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(2);
}


/**
* 
* Set the x, y, and z rotation values given based on mouse location.
* 
**/
void setRotationAndDistance(float& xRot, float& yRot, float& zRot, float& distance)
{
    static int prevMouseX = mouseX;
    static int prevMouseY = mouseY;

    // check to see if there has been a significant jump
    if (abs(mouseX - prevMouseX) > 30 || abs(mouseY - prevMouseY) > 30)
    {
        prevMouseX = mouseX;
        prevMouseY = mouseY;
    }

    // true means camera is right side up
    bool upDir = true;

    if (leftMouse)
    {
        // std::cout << "leftMouse drag.\nChange in Y: " << (prevMouseY - mouseY) << "\n";
        float yDelt = float(mouseY - prevMouseY) / (0.2 * windowHeight);
        float xDelt = float(mouseX - prevMouseX) / (0.2 * windowWidth);
        // dont ask me why these have to be flipped, but if you know let me know
        xRot += yDelt;
        yRot += xDelt;
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
    xRot = cy::Clamp(xRot, DEG2RAD(-89.9f), DEG2RAD(89.9f));
    yRot = fmod(yRot, DEG2RAD(360.0f));
    zRot = fmod(zRot, DEG2RAD(360.0f));
}


/**
* 
* Converts Degrees to Radians
* 
**/
float DEG2RAD(float degrees)
{
    return degrees * (M_PI / 180.0f);
}