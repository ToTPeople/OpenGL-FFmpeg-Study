//
//  CBaseFrameBufferObj.hpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/23.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CBaseFrameBufferObj_hpp
#define CBaseFrameBufferObj_hpp

#include <stdio.h>

class CBaseFrameBufferObj
{
public:
    explicit CBaseFrameBufferObj(int nWidth, int nHeight, bool bAttachRenderBuffer = false, bool bAttachTexture = true);
    ~CBaseFrameBufferObj();
    
public:
    // 获取FBO ID
    unsigned int GetHandleID();
    // 获取FBO texture ID
    unsigned int GetTexureID();
    // 绑定、解绑FBO，并指定viewport大小
    void BindFBO(int nViewportWidth, int nViewportHeight);
    void UnBindFBO();
    
protected:
    // 创建FBO实例
    unsigned int CreateFrameBufferObj(int nWidth, int nHeight);
    
private:
    unsigned int            m_uHandleID;                    // FBO ID
    unsigned int            m_uTexColorBufferID;            // texture ID
    unsigned int            m_uRBO;                         // 缓存ID
    int                     m_nViewportWidth;               // 绑定时，viewport宽、高
    int                     m_nViewportHeight;
    bool                    m_bAttachTexture;               // 是否关联texture，默认关联
    bool                    m_bAttachRenderBuffer;          // 是否关联缓存，默认不关联
    bool                    m_bBindFBO;
};

#endif /* CBaseFrameBufferObj_hpp */
