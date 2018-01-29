//
//  CShapeDataObj.hpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/19.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CShapeDataObj_hpp
#define CShapeDataObj_hpp

#include <stdio.h>
#include <cstring>
#include <string>

class CShapeDataObj
{
public:
    CShapeDataObj();
    ~CShapeDataObj();
    
    void SetDataPath(const std::string& strPath);
    std::string GetDataPath();
    
    void SetTexturePath(const std::string& strPath);
    std::string GetTexturePath();
    
    void SetVerterShaderPath(const std::string& strPath);
    std::string GetVerterShaderPath();
    
    void SetFragmentShaderPath(const std::string& strPath);
    std::string GetFragmentShaderPath();
    
    void SetTextureLoadType(int eTextureLoadType);
    int GetTextureLoadType();
    
private:
    std::string         m_strDataPath;
    std::string         m_strTexturePath;
    std::string         m_strVerterShaderPath;
    std::string         m_strFragmentShaderPath;
    int                 m_eTextureLoadType;
};

#endif /* CShapeDataObj_hpp */
