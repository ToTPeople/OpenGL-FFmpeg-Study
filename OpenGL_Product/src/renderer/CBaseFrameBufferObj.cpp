//
//  CBaseFrameBufferObj.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/23.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "CBaseFrameBufferObj.hpp"
#include <GL/glew.h>


CBaseFrameBufferObj::CBaseFrameBufferObj(int nWidth, int nHeight, bool bAttachRenderBuffer/* = false */, bool bAttachTexture/* = true */)
: m_uHandleID(0)
, m_uTexColorBufferID(0)
, m_uRBO(0)
, m_nViewportWidth(0)
, m_nViewportHeight(0)
, m_bAttachTexture(bAttachTexture)
, m_bAttachRenderBuffer(bAttachRenderBuffer)
, m_bBindFBO(false)
{
    CreateFrameBufferObj(nWidth, nHeight);
}

CBaseFrameBufferObj::~CBaseFrameBufferObj()
{
    if (0 != m_uHandleID) {
        glDeleteFramebuffers(1, &m_uHandleID);
    }
    
    if (0 != m_uTexColorBufferID) {
        glDeleteTextures(1, &m_uTexColorBufferID);
    }
    
    if (0 != m_uRBO) {
        glDeleteRenderbuffers(1, &m_uRBO);
    }
}

unsigned int CBaseFrameBufferObj::GetHandleID()
{
    return m_uHandleID;
}

unsigned int CBaseFrameBufferObj::GetTexureID()
{
    return m_uTexColorBufferID;
}

void CBaseFrameBufferObj::BindFBO(int nViewportWidth, int nViewportHeight)
{
    if (m_bBindFBO) {
        printf("[CBaseFrameBufferObj::BindFBO] warning: frame buffer object has binded, do not bind again!\n");
        return;
    }
    
    m_bBindFBO = true;
    m_nViewportWidth = nViewportWidth;
    m_nViewportHeight = nViewportHeight;
    glBindFramebuffer(GL_FRAMEBUFFER, m_uHandleID);
    glViewport(0, 0, nViewportWidth, nViewportHeight);
    glEnable(GL_DEPTH_TEST);
}

void CBaseFrameBufferObj::UnBindFBO()
{
    if (!m_bBindFBO) {
        printf("[CBaseFrameBufferObj::UnBindFBO] warning: frame buffer object didn't binded, not need to unbind!\n");
        return;
    }
    
    m_bBindFBO = false;
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // mac re... 显示器分辨率多2倍
    glViewport(0, 0, m_nViewportWidth*2, m_nViewportHeight*2);
}

unsigned int CBaseFrameBufferObj::CreateFrameBufferObj(int nWidth, int nHeight)
{
    if (!m_bAttachRenderBuffer && !m_bAttachTexture) {
        printf("[CBaseFrameBufferObj::CreateFrameBufferObj] error: frame buffer object must attach texture or render buffer, please check attribute first. m_bAttachRenderBuffer[%d], m_bAttachTexture[%d]\n", m_bAttachRenderBuffer, m_bAttachTexture);
        return 0;
    }
    // 帧缓冲
    glGenFramebuffers(1, &m_uHandleID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_uHandleID);
    
    // texture、渲染缓冲
    if (m_bAttachTexture) {
        // texture
        glGenTextures(1, &m_uTexColorBufferID);
        glBindTexture(GL_TEXTURE_2D, m_uTexColorBufferID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        // attach
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uTexColorBufferID, 0);
    }
    
    // 渲染缓冲
    if (m_bAttachRenderBuffer) {
        glGenRenderbuffers(1, &m_uRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_uRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, nWidth, nHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        // attach
        //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uRBO);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_uRBO);
        //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_uRBO);
    }
    
    // check
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        fprintf(stderr, "[CBaseFrameBufferObj::CreateFrameBufferObj] Framebuffer is not complete.\n");
        return 0;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return m_uHandleID;
}
