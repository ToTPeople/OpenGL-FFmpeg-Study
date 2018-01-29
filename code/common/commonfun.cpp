//
//  commonfun.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/12.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "commonfun.hpp"
#include <string>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <GLFW/glfw3.h>
#include "common_define.h"


bool LoadDataByFile(const char* path, std::vector<glm::vec3> & out_vertices
                    , std::vector<glm::vec2> & out_uvs
                    , std::vector<glm::vec3> & out_normals, bool bDDS)
{
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    
    out_vertices.clear();
    out_uvs.clear();
    out_normals.clear();
    
    FILE *file = fopen(path, "r");
    if (NULL == file)
    {
        char * dir = getcwd(NULL, 0); // Platform-dependent, see reference link below
        printf("Current dir: %s\n\nImpossiable to open the file %s\n", dir, path);
        return false;
    }
    
    while (1)
    {
        char lineHeader[g_nDefaultMaxLineLength];
        // read the first word of the line
        if (fscanf(file, "%s", lineHeader) == EOF)
        {
            break;
        }
        
        // else : parse lineHeader
        if ( strcmp( lineHeader, "v" ) == 0 )
        {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }
        else if ( strcmp( lineHeader, "vt" ) == 0 )
        {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y );
            if (bDDS)
            {
                uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
            }
            temp_uvs.push_back(uv);
        }
        else if ( strcmp( lineHeader, "vn" ) == 0 )
        {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
        }
        else if ( strcmp( lineHeader, "f" ) == 0 )
        {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 9)
            {
                printf("File can't be read by our simple parser :-( Try exporting with other options\n");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices    .push_back(uvIndex[0]);
            uvIndices    .push_back(uvIndex[1]);
            uvIndices    .push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
        else
        {
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }
    
    // For each vertex of each triangle
    for (unsigned int i = 0; i < vertexIndices.size(); ++i)
    {
        // Get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];
        
        // Get the attributes thanks to the index
        glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
        glm::vec2 uv = temp_uvs[ uvIndex-1 ];
        glm::vec3 normal = temp_normals[ normalIndex-1 ];
        
        // Put the attributes in buffers
        out_vertices.push_back(vertex);
        out_uvs     .push_back(uv);
        out_normals .push_back(normal);
    }
    
    return true;
}

void CopyVertex(std::vector<glm::vec3> &to, std::vector<glm::vec3> &from)
{
    to.clear();
    unsigned long nSize = from.size();
    for (unsigned long i = 0; i < nSize; ++i)
    {
        to.push_back(from[i]);
    }
}

void CopyUV(std::vector<glm::vec2> &to, std::vector<glm::vec2> &from)
{
    to.clear();
    unsigned long nSize = from.size();
    for (unsigned long i = 0; i < nSize; ++i)
    {
        to.push_back(from[i]);
    }
}

int TransGLFWKey2Normal(int eGLFWKeyType)
{
    switch (eGLFWKeyType)
    {
        case GLFW_KEY_UP:
            return ARROW_KEY_TYPE_UP;
        case GLFW_KEY_DOWN:
            return ARROW_KEY_TYPE_DOWN;
        case GLFW_KEY_LEFT:
            return ARROW_KEY_TYPE_LEFT;
        case GLFW_KEY_RIGHT:
            return ARROW_KEY_TYPE_RIGHT;
            
        default:
            return 0;
    }
}

