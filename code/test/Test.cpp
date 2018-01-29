//
//  Test.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/13.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "Test.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "common_define.h"
#include "commonfun.hpp"
#include "CBaseWindows.hpp"

#include "CBaseWindowMgr.hpp"
#include "CScreenShape.hpp"
#include "CShapeManager.hpp"
#include "CBaseTexture.hpp"
#include "CBufferObj.hpp"

#include "CShader.hpp"
#include "CShaderMgr.hpp"
#include "CBaseFrameBufferObj.hpp"

void ShapeInit(CBaseWindows& window);
void FBOTestInit(CBaseWindows& window);

int OpenGLTest()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW.\n");
        return -1;
    }
    
    // 创建窗口并初始化
    CBaseWindows window;
    window.SetWinTitle("OpenGL Test");
    window.SetWindowsAttr(GLFW_SAMPLES, 4);             // 4个采样数
    if (NULL == window.CreateWindows())
    {
        fprintf(stderr, "Create windows failed.\n");
        return -1;
    }
    
    window.SetCurrentContext();
    window.SetInputMode(GLFW_STICKY_KEYS, GL_TRUE);
    window.SetInputMode(GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
    
    // 放在创建windows并设置current context后
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW.\n");
        return -1;
    }
    
    CBaseWindowMgr::GetInstance()->SetBaseWindow(&window);
    
    // 形状初始化
    ShapeInit(window);
    // FBO初始化
    FBOTestInit(window);
    
    g_pBufferObj->GenerateBuffer();
    
#ifdef SCALE_OPERATOR
    window.SetOpType(OP_TYPE_SCALE);
#endif
    window.Show();
    
    return 0;
}

void ShapeInit(CBaseWindows& window)
{
    CBaseShape* pShape = NULL;
#if 0
    pShape = g_pShapeManager->GenShape(SHAPE_TYPE_TRIANGLE, RENDERER_TYPE_SQUARE);    // 方形
    
    if (NULL != pShape)
    {
        window.AddShape(pShape);
    }
#endif
#if 1
    pShape = g_pShapeManager->GenShape(SHAPE_TYPE_TRIANGLE, RENDERER_TYPE_CUBE);    // 立方体
    if (NULL != pShape)
    {
        window.AddShape(pShape);
    }
#endif
#if 1
    pShape = g_pShapeManager->GenShape(SHAPE_TYPE_TRIANGLE, RENDERER_TYPE_SQUARE);    // 方形
    
    if (NULL != pShape)
    {
        window.AddShape(pShape);
    }
#endif
#if 0
    pShape = g_pShapeManager->GenShape(SHAPE_TYPE_TRIANGLE, RENDERER_TYPE_CUBE);    // 立方体
    if (NULL != pShape)
    {
        window.AddShape(pShape);
    }
#endif
#if 0
    pShape = g_pShapeManager->GenShape(SHAPE_TYPE_SCREEN, RENDERER_TYPE_SCREEN);    // screen
    
    if (NULL != pShape)
    {
        window.AddShape(pShape);
    }
#endif
    
    g_pShapeManager->Init();
}

void FBOTestInit(CBaseWindows& window)
{
#ifdef TEST_FBO
    // target shape
    CScreenShape* pTargetShape = (CScreenShape*)g_pShapeManager->GenShape(SHAPE_TYPE_SCREEN, RENDERER_TYPE_SCREEN);// screen
    if (NULL != pTargetShape)
    {
        window.BindTargetShape(pTargetShape);
    }
    
    // fbo
    CBaseFrameBufferObj* pFBO = new CBaseFrameBufferObj(window.GetWinWidth(), window.GetWinHeight(), true, true);
    if (NULL != pFBO)
    {
        window.BindFrameBufferObj(pFBO);
    }
    
    // target shape bind texture
    CBaseTexture* pTexture = new CBaseTexture();
    if (NULL != pTexture && NULL != pFBO && NULL != pTargetShape)
    {
        pTexture->SetHandleID(pFBO->GetTexureID());
        pTargetShape->SetTexture(pTexture);
    }
    
    window.SetUseFrameBufferObj(true);
#endif
}

