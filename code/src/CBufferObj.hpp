//
//  CBufferObj.hpp
//  OpenGL_Product
//  缓存处理类(单例)
//  Created by lifushan on 2018/1/26.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CBufferObj_hpp
#define CBufferObj_hpp

#include <stdio.h>

class CBufferObj
{
public:
    static CBufferObj* GetInstance();
    
public:
    // 获取数组缓存ID和目标类型
    unsigned int GetHandleID();
    unsigned int GetTarget();
    // 获取序号缓存ID和目标类型
    unsigned int GetIndexHandleID();
    unsigned int GetIndexTarget();
    // 生成缓存对象
    void GenerateBuffer();
    
private:
    CBufferObj();
    ~CBufferObj();
    
protected:
    unsigned int    m_uHandleID;                // 数组缓存ID
    unsigned int    m_uTarget;                  // 数组缓存 绑定目标类型
    unsigned int    m_uIndexTarget;             // 序号缓存 绑定目标类型
    unsigned int    m_uIndexHandleID;           // 序号缓存ID
    
    static CBufferObj*   m_pInstance;
};

#define g_pBufferObj (CBufferObj::GetInstance())

#endif /* CBufferObj_hpp */
