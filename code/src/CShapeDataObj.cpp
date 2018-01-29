//
//  CShapeDataObj.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/19.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CShapeDataObj.hpp"

#include "common_define.h"

CShapeDataObj::CShapeDataObj()
: m_strDataPath("")
, m_strTexturePath("")
, m_strVerterShaderPath("")
, m_strFragmentShaderPath("")
, m_eTextureLoadType(LOAD_TEXTURE_TYPE_BMP)
{
}

CShapeDataObj::~CShapeDataObj()
{
}

void CShapeDataObj::SetDataPath(const std::string& strPath)
{
    m_strDataPath = strPath;
}

std::string CShapeDataObj::GetDataPath()
{
    return m_strDataPath;
}

void CShapeDataObj::SetTexturePath(const std::string& strPath)
{
    m_strTexturePath = strPath;
}

std::string CShapeDataObj::GetTexturePath()
{
    return m_strTexturePath;
}

void CShapeDataObj::SetVerterShaderPath(const std::string& strPath)
{
    m_strVerterShaderPath = strPath;
}

std::string CShapeDataObj::GetVerterShaderPath()
{
    return m_strVerterShaderPath;
}

void CShapeDataObj::SetFragmentShaderPath(const std::string& strPath)
{
    m_strFragmentShaderPath = strPath;
}

std::string CShapeDataObj::GetFragmentShaderPath()
{
    return m_strFragmentShaderPath;
}

void CShapeDataObj::SetTextureLoadType(int eTextureLoadType)
{
    m_eTextureLoadType = eTextureLoadType;
}

int CShapeDataObj::GetTextureLoadType()
{
    return m_eTextureLoadType;
}
