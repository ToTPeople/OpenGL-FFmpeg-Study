//
//  CBufferObj.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/26.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CBufferObj.hpp"
#include <GL/glew.h>
#include "commonfun.hpp"
#include "CBufferDataObject.hpp"


CBufferObj* CBufferObj::m_pInstance = NULL;

CBufferObj* CBufferObj::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CBufferObj();
    }
    return m_pInstance;
}

CBufferObj::CBufferObj()
: m_uTarget(GL_ARRAY_BUFFER)
, m_uHandleID(0)
, m_uIndexTarget(GL_ELEMENT_ARRAY_BUFFER)
, m_uIndexHandleID(0)
{
}

CBufferObj::~CBufferObj()
{
    printf("[CBufferObj::~CBufferObj()] m_uHandleID[%d] \n", m_uHandleID);
    if (0 < m_uHandleID)
    {
        glDeleteBuffers(1, &m_uHandleID);
    }
    if (0 < m_uIndexHandleID)
    {
        glDeleteBuffers(1, &m_uIndexHandleID);
    }
}

unsigned int CBufferObj::GetHandleID()
{
    return m_uHandleID;
}

unsigned int CBufferObj::GetTarget()
{
    return m_uTarget;
}

unsigned int CBufferObj::GetIndexHandleID()
{
    return m_uIndexHandleID;
}

unsigned int CBufferObj::GetIndexTarget()
{
    return m_uIndexTarget;
}

void CBufferObj::GenerateBuffer()
{
    if (0 < m_uHandleID)
    {
        printf("[CBufferObj::GenerateBuffer] warning: Buffer object had create, not need to generate again.\n");
        return;
    }
    // array buffer
    m_uTarget = GL_ARRAY_BUFFER;
    glGenBuffers(1, &m_uHandleID);
    glBindBuffer(m_uTarget, m_uHandleID);
    unsigned int nSize = sizeof(PackedVertex) * (g_pBufferDataObject->GetVertexTotalCount());
    std::vector<PackedVertex> vecData = g_pBufferDataObject->GetBufferData();
    glBufferData(m_uTarget, nSize, &vecData[0], GL_STATIC_DRAW);
    // index buffer
    m_uIndexTarget = GL_ELEMENT_ARRAY_BUFFER;
    glGenBuffers(1, &m_uIndexHandleID);
    glBindBuffer(m_uIndexTarget, m_uIndexHandleID);
    nSize = sizeof(unsigned short) * (g_pBufferDataObject->GetIndicesTotalCount());
    std::vector<unsigned short> vecIndices = g_pBufferDataObject->GetIndicesData();
    glBufferData(m_uIndexTarget, nSize, &vecIndices[0], GL_STATIC_DRAW);
}

