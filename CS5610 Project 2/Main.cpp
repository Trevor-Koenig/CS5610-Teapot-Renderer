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
void idleCallback();
void createEnvironmentQuad();
void createArgumentObject(int argc, char* argv[]);
void createScenePlane();
bool createDepthBuffer(int bufferWidth, int bufferHeight);
void setRotationAndDistance(float& xRot, float& yRot, float& zRot, float& distance);
float DEG2RAD(float degrees);

bool leftMouse, rightMouse;
bool altMouseDrag;
int mouseX, mouseY;
int windowWidth, windowHeight;
GLuint teapotVao;
GLuint envVao;
GLuint planeVao;
GLuint shadowFBO;
GLuint cityEnv;
GLuint depthTexture;
int numElem;
cy::GLSLProgram teapotShaders;
cy::GLSLProgram environmentShaders;
cy::GLSLProgram planeShaders;
cy::GLSLProgram shadowMapShaders;
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

    
    // define all objects to be rendered and their textures
    createEnvironmentQuad();
    if (argc > 1)
    {
        createArgumentObject(argc, argv);
    }
    else
    {
        std::cout << "Object file path not found. Please check main arguments.\n";
        exit(10);
    }
    createScenePlane();

    GLint origFB;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB);
    bool success = createDepthBuffer(2*windowWidth, 2*windowHeight);
    if (!success) { return 10; }
    glBindFramebuffer(GL_FRAMEBUFFER, origFB);

    
    /**
    * 
    * Compile shaders
    * 
    **/
    // initialize CyGL
    
    teapotShaders.BuildFiles("Shaders\\shader.vert", "Shaders\\shader.frag");
    environmentShaders.BuildFiles("Shaders\\envShader.vert", "Shaders\\envShader.frag");
    planeShaders.BuildFiles("Shaders\\Passthrough.vert", "Shaders\\SimpleTexture.frag");
    shadowMapShaders.BuildFiles("Shaders\\shadowMap.vert", "Shaders\\shadowMap.frag");

    std::cout << "teapotShaders id: " << teapotShaders.GetID() << "\n";
    std::cout << "environmentShaders id: " << environmentShaders.GetID() << "\n";
    std::cout << "planeShaders id: " << planeShaders.GetID() << "\n";
    std::cout << "shadowMapShaders id: " << shadowMapShaders.GetID() << "\n";
    char t = 0;
    //std::cin >> t;
    

    // clear scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // get original frame buffer id (should be the back buffer)
    origFB;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB);
    //std::cout << origFB << "\n";

    // draw plane with texture
    glBindVertexArray(envVao);
    environmentShaders.Bind();
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDepthMask(GL_TRUE);

    // render teapot
    // set shaders and draw teapot to texture
    glBindVertexArray(teapotVao);
    teapotShaders.Bind();
    glDrawArrays(GL_TRIANGLES, 0, numElem);

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

    // draw depth texture
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowFBO);
    glViewport(0, 0, 2*windowWidth, 2*windowHeight);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(teapotVao);
    shadowMapShaders.Bind();
    glDrawArrays(GL_TRIANGLES, 0, numElem);

    // return to back buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, origFB);
    glViewport(0, 0, windowWidth, windowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw plane with texture
    glBindVertexArray(envVao);
    environmentShaders.Bind();
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDepthMask(GL_TRUE);

    // render argument object (tb to depth map)
    // set shaders and draw teapot to texture
    glBindVertexArray(teapotVao);
    teapotShaders.Bind();
    glDrawArrays(GL_TRIANGLES, 0, numElem);

    // render plane under argument object (also used for testing as a plane to render depth map to)
    glBindVertexArray(planeVao);
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
    cy::Matrix3f rotMatrix = cy::Matrix3f::RotationXYZ(-xRot, -yRot, zRot);
    cy::Matrix4f scaleMatrix = cy::Matrix4f::Scale(cy::Vec3f(distance, distance, distance));
    // for teapot
    cy::Matrix4f trans = cy::Matrix4f::Translation(cy::Vec3f(0.0f, -5.5f, 0.0f));
    // for teapot
    // cy::Matrix4f trans = cy::Matrix4f::Translation(cy::Vec3f(0.0f, 0.0f, 0.0f));
    cy::Vec3f viewPos = (rotMatrix * cy::Vec3f(0.0f, 0.0f, 100.0f)) * distance;
    cy::Matrix4f model = trans;
    cy::Matrix4f view = cy::Matrix4f::View(viewPos, cy::Vec3f(0.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
    cy::Matrix4f projMatrix = cy::Matrix4f::Perspective(DEG2RAD(40), float(windowWidth) / float(windowHeight), 0.1f, 1000.0f);

    cy::Vec3f lightPos = cy::Vec3f(0.0f, 100.0f, 20.0f);

    // set constants for argument object
    teapotShaders["model"] = model;
    teapotShaders["view"] = view;
    teapotShaders["projection"] = projMatrix;
    teapotShaders["lightPos"] =  lightPos;
    teapotShaders["viewPos"] = viewPos;

    // define how the camera moves relative to the environment
    // camera should rotate faster than the object due to parallax
    cy::Matrix3f camRotMatrix = cy::Matrix3f::RotationXYZ(xRot, yRot, zRot);
    cy::Vec3f camPos = camRotMatrix * cy::Vec3f(0.0f, 0.0f, 100.0f);
    cy::Matrix4f camView = cy::Matrix4f::View(camPos, cy::Vec3f(0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
    environmentShaders["view"] = camView;
    environmentShaders["camView"] = (projMatrix * camView).GetInverse();
    environmentShaders["projection"] = projMatrix;
    environmentShaders["viewPos"] = camPos;
    
    // rotate the plane from vertical to horizontal
    cy::Matrix3f planeRotation = cy::Matrix3f::RotationXYZ(DEG2RAD(90.0f), 0.0f, 0.0f);
    // move the plane down to beneath the main scen object (usually a teapot)
    cy::Matrix4f planeTranslation = cy::Matrix4f::Translation(cy::Vec3f(0.0f, -10.0f, 0.0f));
    // define the scale of the teapot to fit the size of the current scene objects
    cy::Matrix4f planeScale = cy::Matrix4f::Scale(cy::Vec3f(0.4f, 0.4f, 0.4f));
    cy::Matrix4f planeModel = planeTranslation * planeScale * planeRotation;
    planeShaders["mvp"] = projMatrix * view * planeModel;

    //create shadow map matrices
    float near_plane = lightPos.Length() * 0.9, far_plane = lightPos.Length() * 1.1;
    cy::Matrix4f lightProjection = projMatrix;//cy::Matrix4f::Perspective(DEG2RAD(40), float(windowWidth) / float(windowHeight), near_plane, far_plane);
    cy::Matrix4f lightView = cy::Matrix4f::View(lightPos, cy::Vec3f(0.0f), cy::Vec3f(0.0f, 1.0f, 0.0f));
    cy::Matrix4f depthMVP = projMatrix * lightView * model;//lightProjection * lightView;
    shadowMapShaders["depthMVP"] = depthMVP;

    //cy::Matrix4f biasMatrix(
    //    0.5, 0.0, 0.0, 0.0,
    //    0.0, 0.5, 0.0, 0.0,
    //    0.0, 0.0, 0.5, 0.0,
    //    0.5, 0.5, 0.5, 1.0
    //);

    float bias = lightPos.Length() * 0.00000008;
    cy::Matrix4f biasMatrix = cy::Matrix4f::Translation(cy::Vec3f(0.5f, 0.5f, (0.5f-bias))) * cy::Matrix4f::Scale(cy::Vec3f(0.5f, 0.5f, 0.5f));
    planeShaders["shadowMVP"] = biasMatrix * projMatrix * lightView * planeModel;
    teapotShaders["shadowMVP"] = biasMatrix * projMatrix * lightView * model;

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
    glutCreateWindow("CS 5610 Project 6 - Environment Mapping\tTrevor Koenig");
    glEnable(GL_DEPTH_TEST);

    const char* versionGL = (const char*)glGetString(GL_VERSION);
    std::cout << "Current OpenGL version: " << versionGL << "\n";
}


/// <summary>
/// readies all the appropriate buffers so the evnironment quad can be rendered.
/// </summary>
void createEnvironmentQuad()
{
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
    glGenVertexArrays(1, &envVao);
    glBindVertexArray(envVao);
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

    // create city environment cubemap
    glGenTextures(1, &cityEnv);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cityEnv);
    // define side order
    std::string sides[] = { "posx.png", "negx.png", "posy.png", "negy.png", "posz.png", "negz.png" };
    unsigned int envWidth = 0;
    unsigned int envHeight = 0;
    // add every side
    for (int i = 0; i < 6; i++)
    {
        std::vector<unsigned char> envTexture;
        // load environment textures
        std::string filepath = "Environments\\city_cubemap\\cubemap_" + sides[i];
        unsigned int error = lodepng::decode(envTexture, envWidth, envHeight, filepath);
        if (error)
        {
            std::cout << "Environment texture" + sides[i] + " file not found. Please check file structure \n";
            exit(1);
        }

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,     // face, adding one specifies new face
            0,                                      // mipmap level
            GL_RGBA,                                // internal formatting
            envWidth,                               // image width
            envHeight,                              // image height
            0,                                      // border (must be 0 I guess)
            GL_RGBA,                                // image format
            GL_UNSIGNED_BYTE,                       // data type
            &envTexture[0]                          // image data
        );
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}


/// <summary>
/// readies all the appropriate buffers so a scene object can be rendered.
/// This method will override old objects if called more than once.
/// </summary>
void createArgumentObject(int argc, char* argv[])
{
    // load obj file
    cy::TriMesh mesh;
    loadObjFile(argv[1], &mesh);

    // Initalize buffers
    GLuint vbo;
    GLuint ebuffer;
    GLuint normbuffer;

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


    // create textures for teapot
    // attempt to load custom textures from command line, otherwise load default texturess
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
}


/// <summary>
/// Readies the linked Vao to be rendered as a plane in the scene
/// </summary>
void createScenePlane()
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
        100.0, -100.0, 0.0
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
}


bool createDepthBuffer(int bufferWidth, int bufferHeight)
{
    // method sourced from: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

    // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, bufferWidth, bufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    glDrawBuffer(GL_NONE); // No color buffer is drawn to.
    glReadBuffer(GL_NONE);


    // Always check that our framebuffer is ok
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;

    return true;
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