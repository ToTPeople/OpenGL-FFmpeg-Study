//
//  CDataObject.hpp
//  OpenGL_Product
//  模型加载类
//  Created by meitu on 2018/1/12.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CDataObject_hpp
#define CDataObject_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>

struct PackedVertex;

class CDataObject
{
public:
    CDataObject();
    ~CDataObject();
    
    virtual void LoadData();
    
    bool GetVertexsData(std::vector<glm::vec3> &vecVertexs);
    bool GetUVsData(std::vector<glm::vec2> &vecUVs);
    bool GetNormalData(std::vector<glm::vec3> &vecNormal);
    bool GetIndices(std::vector<unsigned short> &vecIndices);
    
    void SetUseIndex(bool bUseIndex);
    bool IsUseIndex();
    void SetDDSPicData(bool bDDS);
    
    void SetBufferVertexOff(int nBufferVertexOff);
    int GetBufferVertexOff();
    void SetBufferIndicesOff(int nBufferIndicesOff);
    int GetBufferIndicesOff();
    
    std::vector<glm::vec3> &GetVertexsData();
    std::vector<glm::vec2> &GetUVsData();
    std::vector<glm::vec3> &GetNormalData();
    std::vector<unsigned short> &GetIndices();
    
    void SetDataFilePath(const std::string& strDataFilePath);
    
protected:
    bool GetSimilarVertexIndexFast(PackedVertex & packed
                                   , std::map<PackedVertex,unsigned short> & VertexToOutIndex
                                   , unsigned short & result);
    void GenIndexVBO(std::vector<glm::vec3> & in_vertices,
                  std::vector<glm::vec2> & in_uvs,
                  std::vector<glm::vec3> & in_normals,
                  
                  std::vector<unsigned short> & out_indices,
                  std::vector<glm::vec3> & out_vertices,
                  std::vector<glm::vec2> & out_uvs,
                  std::vector<glm::vec3> & out_normals
                  );
    
private:
    std::vector<glm::vec3>  m_vec_vtxs;          // 顶点数据
    std::vector<glm::vec2>  m_vec_uvs;           // 纹理UV数据
    std::vector<glm::vec3>  m_vec_normal;        // 法线数据
    std::vector<unsigned short> m_vec_indices;   // 索引数据
    
    bool                    m_bInit;
    bool                    m_bUseIndex;        // 是否使用索引数据
    bool                    m_bDDS;             //
    std::string             m_strDataFilePath;  // 数据文件路径
    
    int                     m_nBufferVertexOff;
    int                     m_nBufferIndicesOff;
};

#endif /* CDataObject_hpp */
