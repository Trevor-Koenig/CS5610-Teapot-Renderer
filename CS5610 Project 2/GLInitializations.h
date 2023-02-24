#ifndef GLINIT
#define GLINIT


#include <iostream>
#include <math.h>
#define M_PI 3.141592653589793238462643383279502884L /* pi */
#include <vector>
#include <map>
#include <numbers>
#include "CyCodeBase/cyCore.h"
#include "CyCodeBase/cyVector.h"
#include "CyCodeBase/cyMatrix.h"
#include "CyCodeBase/cyTriMesh.h"



/**
 *
 * Load the obj file found at the provide path: filepath
 * pass filepath as String or char[]
 *
 * the passed in mesh class will be modified to contain the contents of the obj file
 *
 **/
inline void loadObjFile(char filepath[], cy::TriMesh* mesh)
{
    // check for obj  file in arguments - this is required for this program
    if (filepath != NULL)
    {
        std::cout << "Loading .obj file: " << filepath << "\n";
        mesh->LoadFromFileObj(filepath);
    }
    else
    {
        std::cout << "ERROR: no .obj file given to render.\nexiting...\n";
        exit(1);
    }
}

#endif