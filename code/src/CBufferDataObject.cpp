//
//  CBufferDataObject.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/26.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CBufferDataObject.hpp"
#include "common_define.h"
#include "commonfun.hpp"
#include "CDataObject.hpp"


CBufferDataObject* CBufferDataObject::m_pInstance = NULL;

CBufferDataObject* CBufferDataObject::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CBufferDataObject();
    }
    
    return m_pInstance;
}

CBufferDataObject::CBufferDataObject()
: m_nVtxTotalSize(0)
, m_nIndicesTotalSize(0)
{
}

CBufferDataObject::~CBufferDataObject()
{
    printf("[CBufferDataObject::~CBufferDataObject()]\n");
}

void CBufferDataObject::AddData(CDataObject *pData)
{
    if (NULL == pData)
    {
        printf("[CBufferDataObject::AddData] warning: pDate == NULL, please pass right parameter.\n");
        return;
    }
    
    // buffer data
    std::vector<glm::vec3> vecVtx = pData->GetVertexsData();
    std::vector<glm::vec2> vecUV = pData->GetUVsData();
    int nSize = vecVtx.size();
    pData->SetBufferVertexOff(m_nVtxTotalSize);
    for (int i = 0; i < nSize; ++i)
    {
        PackedVertex packed = {vecVtx[i], vecUV[i]};
        m_vecBufferData.push_back(packed);
    }
    m_nVtxTotalSize += nSize;
    
    // index data
    if (pData->IsUseIndex())
    {
        std::vector<unsigned short> vecIndices = pData->GetIndices();
        nSize = vecIndices.size();
        pData->SetBufferIndicesOff(m_nIndicesTotalSize);
        for (int i = 0; i < nSize; ++i)
        {
            m_vec_indices.push_back(vecIndices[i]);
        }
        
        m_nIndicesTotalSize += nSize;
    }
}

int CBufferDataObject::GetVertexTotalCount()
{
    return m_nVtxTotalSize;
}

int CBufferDataObject::GetIndicesTotalCount()
{
    return m_nIndicesTotalSize;
}

std::vector<PackedVertex> &CBufferDataObject::GetBufferData()
{
    return m_vecBufferData;
}

void CBufferDataObject::GetBufferData(std::vector<PackedVertex> &vecBufferData)
{
    vecBufferData.clear();
    for (int i = 0; i < m_nVtxTotalSize; ++i)
    {
        vecBufferData.push_back(m_vecBufferData[i]);
    }
}

std::vector<unsigned short> &CBufferDataObject::GetIndicesData()
{
    return m_vec_indices;
}

void CBufferDataObject::GetIndicesData(std::vector<unsigned short> &vecIndicesData)
{
    vecIndicesData.clear();
    for (int i = 0; i < m_nIndicesTotalSize; ++i)
    {
        vecIndicesData.push_back(m_vec_indices[i]);
    }
}

