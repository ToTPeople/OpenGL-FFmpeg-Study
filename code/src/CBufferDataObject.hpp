//
//  CBufferDataObject.hpp
//  OpenGL_Product
//  
//  Created by meitu on 2018/1/26.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CBufferDataObject_hpp
#define CBufferDataObject_hpp

#include <stdio.h>
#include <vector>
#include "common_define.h"

class CDataObject;

class CBufferDataObject
{
public:
    static CBufferDataObject* GetInstance();
    
public:
    // 添加数据
    void AddData(CDataObject* pData);
    // 获取数据总娄
    int GetVertexTotalCount();
    int GetIndicesTotalCount();
    // 获取buffer data
    std::vector<PackedVertex> &GetBufferData();
    void GetBufferData(std::vector<PackedVertex> &vecBufferData);
    // 获取index data
    std::vector<unsigned short> &GetIndicesData();
    void GetIndicesData(std::vector<unsigned short> &vecIndicesData);
    
private:
    CBufferDataObject();
    ~CBufferDataObject();
    
private:
    std::vector<PackedVertex>   m_vecBufferData;        // 数组缓存整合数据
    std::vector<unsigned short> m_vec_indices;          // 序号缓存整合数据
    int                         m_nVtxTotalSize;        // 数组缓存整合数据个数
    int                         m_nIndicesTotalSize;    // 序号缓存整合数据个数
    static CBufferDataObject*   m_pInstance;
};

#define g_pBufferDataObject (CBufferDataObject::GetInstance())

#endif /* CBufferDataObject_hpp */
