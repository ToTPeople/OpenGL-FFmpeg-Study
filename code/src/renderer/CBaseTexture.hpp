//
//  CBaseTexture.hpp
//  OpenGL_Product
//  纹理处理类
//  Created by meitu on 2018/1/13.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CBaseTexture_hpp
#define CBaseTexture_hpp

#include <stdio.h>
#include <iostream>
#include <cstring>
#include "common_define.h"


class CBaseTexture
{
public:
    CBaseTexture();
    virtual ~CBaseTexture();
    
public:
    void SetImagePath(const std::string& strImagePath);
    void SetTarget(int eTarget);
    
    
    virtual void LoadTexture(int nLoadType);
    
    unsigned int GetHandleID();
    unsigned int GetTarget();       // target目前只用GL_TEXTURE_2D，后续有使用其他再加接口让用户设置
    
    void SetHandleID(unsigned int uHandleID);
    
protected:
    // 加载纹理方式
    unsigned int LoadBMPCustom(const char * imagepath);
    unsigned int LoadDDS(const char * imagepath);
    
protected:
    std::string                 m_strImagePath;                 // 纹理图片路径
    unsigned int                m_uHandleID;
    unsigned int                m_uTarget;
};

#endif /* CBaseTexture_hpp */
