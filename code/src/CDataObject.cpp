//
//  CDataObject.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/12.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CDataObject.hpp"
#include "common_define.h"
#include "commonfun.hpp"

CDataObject::CDataObject()
: m_strDataFilePath("")
, m_bInit(false)
, m_bUseIndex(true)
, m_bDDS(false)
, m_nBufferVertexOff(0)
, m_nBufferIndicesOff(0)
{
}

CDataObject::~CDataObject()
{
    printf("[CDataObject::~CDataObject()] \n");
}

void CDataObject::LoadData()
{
    if (m_strDataFilePath.empty())
    {
        printf("[CDataObject::InitData()] data path is empty, please set path first.\n");
        return;
    }
    
    if (m_bUseIndex)
    {
        std::vector<glm::vec3>  vecTmpVtxs;
        std::vector<glm::vec2>  vecTmpUvs;
        std::vector<glm::vec3>  vecTmpNormal;
        m_vec_uvs.clear();
        m_vec_vtxs.clear();
        m_vec_normal.clear();
        m_vec_indices.clear();
        m_bInit = LoadDataByFile(m_strDataFilePath.c_str(), vecTmpVtxs, vecTmpUvs, vecTmpNormal, m_bDDS);
        GenIndexVBO(vecTmpVtxs, vecTmpUvs, vecTmpNormal, m_vec_indices, m_vec_vtxs, m_vec_uvs, m_vec_normal);
        
        printf("[] Size: vecTmpVtxs[%d], vecTmpUvs[%d], vecTmpNormal[%d],, m_vec_indices[%d], m_vec_vtxs[%d], m_vec_uvs[%d], m_vec_normal[%d] \n"
               , vecTmpVtxs.size(), vecTmpUvs.size(), vecTmpNormal.size(), m_vec_indices.size()
               , m_vec_vtxs.size(), m_vec_uvs.size(), m_vec_normal.size());
    }
    else
    {
        m_bInit = LoadDataByFile(m_strDataFilePath.c_str(), m_vec_vtxs, m_vec_uvs, m_vec_normal, m_bDDS);
    }
}

bool CDataObject::GetVertexsData(std::vector<glm::vec3> &vecVertexs)
{
    if (!m_bInit)
    {
        return false;
    }
    
    CopyVertex(vecVertexs, m_vec_vtxs);
    
    return true;
}

bool CDataObject::GetUVsData(std::vector<glm::vec2> &vecUVs)
{
    if (!m_bInit)
    {
        return false;
    }
    
    CopyUV(vecUVs, m_vec_uvs);
    
    return true;
}

bool CDataObject::GetNormalData(std::vector<glm::vec3> &vecNormal)
{
    if (!m_bInit)
    {
        return false;
    }
    
    CopyVertex(vecNormal, m_vec_normal);
    
    return true;
}

bool CDataObject::GetIndices(std::vector<unsigned short> &vecIndices)
{
    if (!m_bInit || !m_bUseIndex)
    {
        return false;
    }
    
    vecIndices.clear();
    unsigned long nSize = m_vec_indices.size();
    for (unsigned long i = 0; i < nSize; ++i)
    {
        vecIndices.push_back(m_vec_indices[i]);
    }
    
    return true;
}

void CDataObject::SetUseIndex(bool bUseIndex)
{
    m_bUseIndex = bUseIndex;
}

bool CDataObject::IsUseIndex()
{
    return m_bUseIndex;
}

void CDataObject::SetDDSPicData(bool bDDS)
{
    m_bDDS = bDDS;
}

void CDataObject::SetBufferVertexOff(int nBufferVertexOff)
{
    m_nBufferVertexOff = nBufferVertexOff;
}

int CDataObject::GetBufferVertexOff()
{
    return m_nBufferVertexOff;
}

void CDataObject::SetBufferIndicesOff(int nBufferIndicesOff)
{
    m_nBufferIndicesOff = nBufferIndicesOff;
}

int CDataObject::GetBufferIndicesOff()
{
    return m_nBufferIndicesOff;
}

std::vector<glm::vec3> &CDataObject::GetVertexsData()
{
    return m_vec_vtxs;
}

std::vector<glm::vec2> &CDataObject::GetUVsData()
{
    return m_vec_uvs;
}

std::vector<glm::vec3> &CDataObject::GetNormalData()
{
    return m_vec_normal;
}

std::vector<unsigned short> &CDataObject::GetIndices()
{
    return m_vec_indices;
}

void CDataObject::SetDataFilePath(const std::string& strDataFilePath)
{
    m_strDataFilePath = strDataFilePath;
}

bool CDataObject::GetSimilarVertexIndexFast(PackedVertex & packed
                                            , std::map<PackedVertex,unsigned short> & VertexToOutIndex
                                            , unsigned short & result)
{
    std::map<PackedVertex,unsigned short>::iterator it = VertexToOutIndex.find(packed);
    if ( it == VertexToOutIndex.end() )
    {
        return false;
    }
    else
    {
        result = it->second;
        return true;
    }
}

void CDataObject::GenIndexVBO(std::vector<glm::vec3> &in_vertices
                           , std::vector<glm::vec2> &in_uvs
                           , std::vector<glm::vec3> &in_normals
                           , std::vector<unsigned short> &out_indices
                           , std::vector<glm::vec3> &out_vertices
                           , std::vector<glm::vec2> &out_uvs
                           , std::vector<glm::vec3> &out_normals)
{
    std::map<PackedVertex,unsigned short> VertexToOutIndex;
    
    // For each input vertex
    for ( unsigned int i=0; i<in_vertices.size(); i++ )
    {
        PackedVertex packed = {in_vertices[i], in_uvs[i]};
        
        // Try to find a similar vertex in out_XXXX
        unsigned short index;
        bool bFound = GetSimilarVertexIndexFast( packed, VertexToOutIndex, index);
        
        if (bFound)
        {
            // A similar vertex is already in the VBO, use it instead !
            out_indices.push_back( index );
        }
        else
        {
            // If not, it needs to be added in the output data.
            out_vertices.push_back( in_vertices[i]);
            out_uvs     .push_back( in_uvs[i]);
            out_normals .push_back( in_normals[i]);
            unsigned short newindex = (unsigned short)out_vertices.size() - 1;
            out_indices .push_back( newindex );
            VertexToOutIndex[ packed ] = newindex;
        }
    }
}

