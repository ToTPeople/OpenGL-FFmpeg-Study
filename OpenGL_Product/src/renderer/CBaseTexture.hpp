//
//  CBaseTexture.hpp
//  OpenGL_Product
//  纹理处理类
//  Created by lifushan on 2018/1/13.
//  Copyright © 2018年 lifs. All rights reserved.
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
    
    // 加载纹理
    virtual void LoadTexture(int nLoadType);
    // 更新纹理
    void UpdateTexture(const std::string &strPath);
    void UpdateTexture(const void * pData, int width, int height);
    
    unsigned int GetHandleID();
    unsigned int GetTarget();       // target目前只用GL_TEXTURE_2D，后续有使用其他再加接口让用户设置
    
    void SetHandleID(unsigned int uHandleID);
    
protected:
    // 加载纹理方式
    unsigned int LoadBMPCustom(const char * imagepath);
    unsigned int LoadDDS(const char * imagepath);
    
protected:
    std::string                 m_strImagePath;         // texture image path
    unsigned int                m_uHandleID;            // texture object handle id
    unsigned int                m_uTarget;              // texture target
    bool                        m_is_first_update;
};

#endif /* CBaseTexture_hpp */
