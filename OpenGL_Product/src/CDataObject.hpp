//
//  CDataObject.hpp
//  OpenGL_Product
//  模型加载类
//  Created by lifushan on 2018/1/12.
//  Copyright © 2018年 lifs. All rights reserved.
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
    
    // 顶点、UV、法线、序号 数据获取
    bool GetVertexsData(std::vector<glm::vec3> &vecVertexs);
    bool GetUVsData(std::vector<glm::vec2> &vecUVs);
    bool GetNormalData(std::vector<glm::vec3> &vecNormal);
    bool GetIndices(std::vector<unsigned short> &vecIndices);
    
    // set and get attribute that if depend on 'Index' to store data
    void SetUseIndex(bool bUseIndex);
    bool IsUseIndex();
    
    // pic is DDS type or not
    void SetDDSPicData(bool bDDS);
    
    // set and get attribute that 'Index' store data offset
    void SetBufferVertexOff(int nBufferVertexOff);
    int GetBufferVertexOff();
    void SetBufferIndicesOff(int nBufferIndicesOff);
    int GetBufferIndicesOff();
    
    // 顶点、UV、法线、序号 数据获取
    std::vector<glm::vec3> &GetVertexsData();
    std::vector<glm::vec2> &GetUVsData();
    std::vector<glm::vec3> &GetNormalData();
    std::vector<unsigned short> &GetIndices();
    
    // set load data path
    void SetDataFilePath(const std::string& strDataFilePath);
    
protected:
    // judge if 'Index' data has exist
    bool GetSimilarVertexIndexFast(PackedVertex & packed
                                   , std::map<PackedVertex,unsigned short> & VertexToOutIndex
                                   , unsigned short & result);
    
    // generate 'Index' data
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
