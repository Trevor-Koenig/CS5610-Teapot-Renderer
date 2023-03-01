// CS6610Proj1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <math.h>
#include <vector>
#include <map>
#include <algorithm>
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


void createOpenGLWindow(int width, int height);
void drawNewFrame();
void keyboardInterrupt(unsigned char key, int x, int y);
void mouseButtonTracker(int button, int state, int x, int y);
void mouseClickDrag(int x, int y);
void idleCallback();
void setRotationAndDistance(float& xRot, float& yRot, float& zRot, float& distance);
float DEG2RAD(float degrees);

bool leftMouse, rightMouse;
bool altMouseDrag;
int mouseX, mouseY;
int windowWidth, windowHeight;
GLuint teapotVao;
GLuint planeVao;
GLuint frameBuffer;
GLuint renderTexture;
int numElem;
cy::GLSLProgram teapotShaders;
cy::GLSLProgram planeShaders;
cyGLTexture2D tex;


int main(int argc, char* argv[])
{

    /**
    *
    * Initializations
    *
    **/
    leftMouse = false; rightMouse = false;
    altMouseDrag = false;
    mouseX = 0; mouseY = 0;
    numElem = 0;


    cy::TriMesh mesh;
    loadObjFile(argv[1], &mesh);


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

    // OpenGL initializations
    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f; // sourced from: https://youtu.be/6dtqg0r28Yc
    glClearColor(Red, Green, Blue, Alpha);

    // Initalize buffers
    GLuint vbo;
    GLuint ebuffer;
    GLuint normbuffer;

    GLuint planeVbo;
    GLuint planeNBuffer;
    GLuint planeEBuffer;
    GLuint planeTxc;

    // define plane positioning
    float planeVert[] = {
        1.0,  1.0, 0.0,
       -1.0,  1.0, 0.0,
        1.0, -1.0, 0.0,
       -1.0,  1.0, 0.0,
       -1.0, -1.0, 0.0,
        1.0, -1.0, 0.0
    };

    // define plane normals
    float planeNorms[] = {
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 1.0
    };

    // define plane faces
    int planeFaces[] = {
        0, 1, 2,
        3, 4, 5
    };

    // define texture coordinates
    float planeTxcArray[] = {
        1.0, 1.0,
        0.0, 1.0,
        1.0, 0.0,
        0.0, 1.0,
        0.0, 0.0,
        1.0, 0.0
    };

    // create plane plane VAO and vbo
    glGenVertexArrays(1, &planeVao);
    glBindVertexArray(planeVao);
    glGenBuffers(1, &planeVbo);
    glBindBuffer(GL_ARRAY_BUFFER, planeVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVert), planeVert, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // create plane normal buffer
    glGenBuffers(1, &planeNBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, planeNBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeNorms), planeNorms, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(1);

    // create plane element buffer
    glGenBuffers(1, &planeEBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeFaces), planeFaces, GL_STATIC_DRAW);

    // create texture coordinates buffer
    glGenBuffers(1, &planeTxc);
    glBindBuffer(GL_ARRAY_BUFFER, planeTxc);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeTxcArray), planeTxcArray, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(2);

    // create texture for plane the manual way (this is empty for now and is to be rendered to)
    // now define the texture buffer
    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    // define texture as null data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // define depth buffer for texture
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
    // finally configure that framebuffer we created earlier
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTexture, 0);
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Frame buffer was not initalized in time. Exiting...";
        exit(70);
    }

    // rebind the back buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // now onto the teapot
    std::vector<cy::Vec3f> vertices = {};
    std::vector<cy::Vec3f> normals = {};
    std::vector<cy::Vec2f> texCoords = {};
    std::vector<cy::TriMesh::TriFace> elem = {};
    numElem = mesh.NF();
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
    glGenVertexArrays(1, &teapotVao);
    glBindVertexArray(teapotVao);

    // create VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(cy::Vec3f), &vertices[0], GL_STATIC_DRAW);


    // create normal buffer
    glGenBuffers(1, &normbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(cy::Vec3f), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

    // create element buffer
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


    // create textures
    //attempt to load custom textures from command line, otherwise load default texturess
    std::vector<unsigned char> texture;
    unsigned int width, height;

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
    
    teapotShaders.BuildFiles("Shaders\\shader.vert", "Shaders\\shader.frag");

    planeShaders.BuildFiles("Shaders\\Passthrough.vert", "Shaders\\SimpleTexture.frag");
    

    // call draw function
    // get original frame buffer id (should be the back buffer)
    GLint origFB;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB);

    // set frame target and render
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, windowWidth, windowHeight);
    Red = 0.5f, Green = 0.5f, Blue = 0.5f, Alpha = 0.0f; // sourced from: https://youtu.be/6dtqg0r28Yc
    glClearColor(Red, Green, Blue, Alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // set shaders and draw teapot to texture
    glBindVertexArray(teapotVao);
    tex.Bind(0);
    teapotShaders.Bind();
    glDrawArrays(GL_TRIANGLES, 0, numElem);

    // draw plane with rendered texture
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, origFB);
    glViewport(0, 0, windowWidth, windowHeight);
    Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f; // sourced from: https://youtu.be/6dtqg0r28Yc
    glClearColor(Red, Green, Blue, Alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(planeVao);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    planeShaders.Bind();

    std::cout << "Finished drawing first frame. Entering main loop\n";
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
    // get original frame buffer id (should be the back buffer)
    GLint origFB;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB);

    // set frame target and render
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, windowWidth, windowHeight);
    GLclampf Red = 0.5f, Green = 0.5f, Blue = 0.5f, Alpha = 0.0f; // sourced from: https://youtu.be/6dtqg0r28Yc
    glClearColor(Red, Green, Blue, Alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // set shaders and draw teapot to texture
    glBindVertexArray(teapotVao);
    tex.Bind(0);
    teapotShaders.Bind();
    glDrawArrays(GL_TRIANGLES, 0, numElem);

    // draw plane with rendered texture
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, origFB);
    glViewport(0, 0, windowWidth, windowHeight);
    Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f; // sourced from: https://youtu.be/6dtqg0r28Yc
    glClearColor(Red, Green, Blue, Alpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(planeVao);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    planeShaders.Bind();

    glDrawArrays(GL_TRIANGLES, 0, 6);

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
    cy::Matrix3f rotMatrix = cy::Matrix3f::RotationXYZ(xRot, yRot, zRot);
    cy::Matrix4f scaleMatrix = cy::Matrix4f::Scale(cy::Vec3f(distance, distance, distance));
    cy::Matrix4f trans = cy::Matrix4f::Translation(cy::Vec3f(0.0f, 0.0f, -5.5f));
    cy::Vec3f viewPos = rotMatrix * cy::Vec3f(0, 50, 0);
    cy::Matrix4f model = scaleMatrix * trans;
    cy::Matrix4f view = cy::Matrix4f::View(viewPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, 1.0f));
    cy::Matrix4f projMatrix = cy::Matrix4f::Perspective(DEG2RAD(40), float(windowWidth) / float(windowHeight), 0.1f, 1000.0f);
    cy::Matrix4f mvp = projMatrix * view * model;

    // recompile shaders, set constants, and bind them
    teapotShaders["model"] = model;
    teapotShaders["view"] = view;
    teapotShaders["projection"] = projMatrix;
    // why does this work? - is it because it moves the light opposite of camera?
    teapotShaders["lightPos"] =  cy::Vec3f(20.0f, 0.0f, 100.0f);
    teapotShaders["viewPos"] = viewPos;
    // teapotShaders["tex"] = 0;


    cy::Matrix3f planeRotMatrix = cy::Matrix3f::RotationXYZ(altXRot, altYRot, altZRot);
    cy::Matrix4f planeScaleMatrix = cy::Matrix4f::Scale(cy::Vec3f(altDistance, altDistance, altDistance));
    //cy::Matrix4f planeTrans = cy::Matrix4f::Translation(cy::Vec3f(0.0f, 0.0f, -5.5f));
    cy::Matrix4f planeModel = planeScaleMatrix * planeRotMatrix;
    cy::Vec3f planeViewPos = cy::Vec3f(0, 0, 0.01);
    cy::Matrix4f planeView = cy::Matrix4f::View(planeViewPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f)) * planeScaleMatrix * planeRotMatrix;
    planeShaders["model"] = planeModel;
    planeShaders["view"] = planeView;
    planeShaders["projection"] = projMatrix;
    


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
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("CS 5610 Project 4 - Textures\tTrevor Koenig");
    glEnable(GL_DEPTH_TEST);

    const char* versionGL = (const char*)glGetString(GL_VERSION);
    std::cout << "Current OpenGL version: " << versionGL << "\n";
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
        if (zRot < DEG2RAD(180.0f))
        {
            xRot += yDelt;
        }
        else
        {
            xRot -= yDelt;
        }
        if (xRot < DEG2RAD(180.0f))
        {
            zRot -= xDelt;
        }
        else
        {
            zRot += xDelt;
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