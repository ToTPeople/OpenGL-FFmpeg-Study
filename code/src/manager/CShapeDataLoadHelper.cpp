//
//  CShapeDataLoadHelper.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/19.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CShapeDataLoadHelper.hpp"
#include <unistd.h>

#include "common_define.h"
#include "CShapeDataObj.hpp"


namespace
{
    /*
     # dp  = data path
     # vsp = vertex shader path
     # fsp = fragment shader path
     # tp  = texture path
     # tlt = texture load type目前支持2种：dds和bmp
     # shape 一个形状数据开始标志
     # endshape 一个形状数据结束标志
     */
    const char* const kszShapeDataStart = "shape";
    const char* const kszShapeDataEnd = "endshape";
    const char* const kszDataPath = "dp";
    const char* const kszVertexShaderPath = "vsp";
    const char* const kszFragShaderPath = "fsp";
    const char* const kszTexturePath = "tp";
    const char* const kszTextureLoadType = "tlt";
    const char* const kszDDSLoadType = "dds";
    const char* const kszBMPLoadType = "bmp";
}

CShapeDataLoadHelper* CShapeDataLoadHelper::m_pInstance = NULL;

CShapeDataLoadHelper* CShapeDataLoadHelper::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CShapeDataLoadHelper();
    }
    
    return m_pInstance;
}

bool CShapeDataLoadHelper::LoadDataByFile(const char *path)
{
    if (NULL == path)
    {
        printf("[CShapeDataLoadHelper::LoadDataByFile] path is null.\n");
        return false;
    }
    FILE *file = fopen(path, "r");
    if (NULL == file)
    {
        char * dir = getcwd(NULL, 0); // Platform-dependent, see reference link below
        printf("Current dir: %s\n\nImpossiable to open the file %s\n", dir, path);
        return false;
    }
    
    ClearData();
    CShapeDataObj* pObj = NULL;
    
    char strTmpPath[g_nDefaultMaxLineLength];
    
    while (1)
    {
        char lineHeader[g_nDefaultMaxLineLength];
        // read the first word of the line
        if (fscanf(file, "%s", lineHeader) == EOF)
        {
            break;
        }
        
        // else : parse lineHeader
        if ( strcmp(lineHeader, kszShapeDataStart) == 0 )
        {
            if (NULL != pObj)
            {
                m_vecShapeDataObj.push_back(pObj);
            }
            
            pObj = new CShapeDataObj();
        }
        else if ( strcmp(lineHeader, kszShapeDataEnd) == 0 )
        {
            if (NULL != pObj)
            {
                m_vecShapeDataObj.push_back(pObj);
            }
            
            pObj = NULL;
        }
        else if ( strcmp( lineHeader, kszDataPath ) == 0 )
        {
            fscanf(file, "%s\n", strTmpPath);
            if (NULL != pObj)
            {
                pObj->SetDataPath(std::string(strTmpPath));
            }
        }
        else if ( strcmp( lineHeader, kszVertexShaderPath ) == 0 )
        {
            fscanf(file, "%s\n", strTmpPath);
            if (NULL != pObj)
            {
                pObj->SetVerterShaderPath(std::string(strTmpPath));
            }
        }
        else if ( strcmp( lineHeader, kszFragShaderPath ) == 0 )
        {
            fscanf(file, "%s\n", strTmpPath);
            if (NULL != pObj)
            {
                pObj->SetFragmentShaderPath(std::string(strTmpPath));
            }
        }
        else if ( strcmp( lineHeader, kszTexturePath ) == 0 )
        {
            fscanf(file, "%s\n", strTmpPath);
            if (NULL != pObj)
            {
                pObj->SetTexturePath(std::string(strTmpPath));
            }
        }
        else if ( strcmp(lineHeader, kszTextureLoadType) == 0 )
        {
            fscanf(file, "%s\n", strTmpPath);
            int eType = LOAD_TEXTURE_TYPE_BMP;
            if (strcmp(strTmpPath, kszDDSLoadType) == 0)
            {
                eType = LOAD_TEXTURE_TYPE_DDS;
            }
            if (NULL != pObj)
            {
                pObj->SetTextureLoadType(eType);
            }
        }
        else
        {
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }
    
    return true;
}

void CShapeDataLoadHelper::ClearData()
{
    int nSize = m_vecShapeDataObj.size();
    for (int i=0; i<nSize; ++i)
    {
        CShapeDataObj* pObj = m_vecShapeDataObj[i];
        if (NULL != pObj)
        {
            delete pObj;
            pObj = NULL;
        }
    }
    m_vecShapeDataObj.clear();
}

CShapeDataLoadHelper::CShapeDataLoadHelper()
{
    m_vecShapeDataObj.clear();
}
