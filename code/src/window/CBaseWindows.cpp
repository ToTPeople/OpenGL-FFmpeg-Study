//
//  CBaseWindows.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/12.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CBaseWindows.hpp"
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "common_define.h"
#include "commonfun.hpp"
#include "CBaseShape.hpp"
#include "CScreenShape.hpp"
#include "CMatrixTrans.hpp"
#include "CDataObject.hpp"
#include "CBaseWindowMgr.hpp"
#include "CDataObjMgr.hpp"
#include "CShapeManager.hpp"
#include "CBaseFrameBufferObj.hpp"
#include "CShaderMgr.hpp"
#include "CShader.hpp"
#include "CBaseVideoPlay.hpp"
#include "CTextureMgr.hpp"
#include "CBaseTexture.hpp"
#include "CFFmpegYUV2H264.hpp"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

const int g_iDefaultWindowsWidth = 1024;
const int g_iDefaultWindowsHeight = 768;

const int g_iDefaultContextVersionMajor = 2;
const int g_iDefaultContextVersionMinor = 1;

CBaseWindows::CBaseWindows()
: m_iWinWidth(g_iDefaultWindowsWidth)
, m_iWinHeight(g_iDefaultWindowsHeight)
, m_strTitle("")
, m_Windows(NULL)
, m_eOpType(OP_TYPE_DRAG)
, m_bUseFrameBufferObj(true)
, m_pFBO(NULL)
, m_pTargetShape(NULL)
, m_pVideoPlay(NULL)
, m_bEndRecord(false)
, m_bStartRecord(false)
{
    m_listShape.clear();
    m_mpShape.clear();
}

CBaseWindows::~CBaseWindows()
{
    ClearShape();
    ReleaseFrameBufferObj();
    ReleaseTargetShape();
    ReleaseVideoPlay();
    
    if (NULL != m_Windows)
    {
        glfwTerminate();
    }
}

// 按空格操作：添加方形或视频
static void DealWithSpace(CBaseWindows* pBaseWin)
{
    bool bVideoPlay = false;
    
    if (NULL == pBaseWin)
    {
        printf("[DealWithSpace] error: param invalid. pBaseWin[%p]\n", pBaseWin);
        return;
    }
    
#ifdef VIDEO_ASYNCHRONOUS_TEST
    if (NULL != pBaseWin->GetVideoPlay()
        && pBaseWin->GetVideoPlay()->GetCreateThreadCnt() >= CBaseVideoPlay::THREAD_MAX_COUNT)
    {
        printf("[DealWithSpace] warning: max thread limit is %d, can't create.\n", CBaseVideoPlay::THREAD_MAX_COUNT);
        return;
    }
    
    bVideoPlay = true;
    g_pTextureMgr->GenNewVideoPlayPic();
#endif // endif VIDEO_ASYNCHRONOUS_TEST
    
    CBaseShape* pShape = g_pShapeManager->GenShape(SHAPE_TYPE_TRIANGLE, RENDERER_TYPE_SQUARE, bVideoPlay);
    if (NULL != pShape)
    {
        pBaseWin->AddShape(pShape);
        
#ifdef VIDEO_ASYNCHRONOUS_TEST
        if (NULL != pBaseWin->GetVideoPlay())
        {
            pBaseWin->GetVideoPlay()->Run(g_pTextureMgr->GetVideoPlayPic());
        }
#endif // endif VIDEO_ASYNCHRONOUS_TEST
    }
    return;
}

void CBaseWindows::KeyCallBackFunc(GLFWwindow *pWindow, int nKey, int nScanMode, int nAction, int nMods)
{
    if (NULL == pWindow)
    {
        return;
    }
    
    printf("[CBaseWindows::KeyCallBackFunc] window[%p], key[%d], action[%d]..\n", pWindow, nKey, nAction);
    CBaseWindows* pBaseWin = CBaseWindowMgr::GetInstance()->GetBaseWindow();
    if (NULL == pBaseWin || pBaseWin->GetWindows() != pWindow)
    {
        printf("[CBaseWindows::KeyCallBackFunc] pWindow[%p], pBaseWin[%p], baseWinWin[%p]\n"
               , pWindow, pBaseWin, pBaseWin->GetWindows());
        return;
    }
    
    // 临时设置 按空格加图形
    if (GLFW_PRESS == nAction && GLFW_KEY_SPACE == nKey)
    {
        DealWithSpace(pBaseWin);
        return;
    }
    // 按esc退出
    if (GLFW_PRESS == nAction && GLFW_KEY_ESCAPE == nKey)
    {
        exit(0);
        return;
    }
    // 截屏保存
    if (GLFW_PRESS == nAction && GLFW_KEY_S == nKey)
    {
        //pBaseWin->ScreenShots();
        pBaseWin->m_bStartRecord = true;
        pBaseWin->m_bEndRecord = false;
        return;
    }
    else if (GLFW_PRESS == nAction && GLFW_KEY_E == nKey)
    {
        pBaseWin->m_bEndRecord = true;
        return;
    }
    
    // 更新相机位置坐标
    std::list<CBaseShape*>  listShape = pBaseWin->GetShapeList();
    std::list<CBaseShape*>::iterator it = listShape.begin();
    CBaseShape* pShape = NULL;
    for (; it != listShape.end(); ++it)
    {
        pShape = *it;
        if (NULL == pShape)
        {
            continue;
        }
        
        float fDeltaTime = pBaseWin->GetDeltaTime();
        if (NULL != pShape && (GLFW_PRESS == nAction || GLFW_REPEAT == nAction)
            && NULL != pShape->GetMatrixTrans())
        {
            int nArrowKey = TransGLFWKey2Normal(nKey);
            if (ARROW_KEY_TYPE_BASE < nArrowKey && nArrowKey < ARROW_KEY_TYPE_MAX)
            {
                pShape->GetMatrixTrans()->UpdatePositionByArrowKey(nArrowKey, fDeltaTime);
            }
        }
    }
}

void CBaseWindows::MouseCallBackFunc(GLFWwindow *pWindow, int nButton, int nAction, int nMods)
{
    if (NULL == pWindow)
    {
        return;
    }
    
    printf("[CBaseWindows::MouseCallBackFunc] window[%p], nButton[%d], action[%d]..\n", pWindow, nButton, nAction);
    CBaseWindows* pBaseWin = CBaseWindowMgr::GetInstance()->GetBaseWindow();
    if (NULL == pBaseWin || pBaseWin->GetWindows() != pWindow)
    {
        printf("[CBaseWindows::MouseCallBackFunc] pWindow[%p], pBaseWin[%p], baseWinWin[%p]\n"
               , pWindow, pBaseWin, pBaseWin->GetWindows());
        return;
    }
    
    std::list<CBaseShape*>  listShape = pBaseWin->GetShapeList();
    std::list<CBaseShape*>::iterator it = listShape.begin();
    CBaseShape* pShape = NULL;
    CBaseShape* pSelectShape = NULL;
    float fMinZ = MAX_Z_DEPTH;
    for (; it != listShape.end(); ++it)
    {
        pShape = *it;
        if (NULL == pShape)
        {
            continue;
        }
        
        float fDeltaTime = pBaseWin->GetDeltaTime();
        
        double lfXPos, lfYPos;                  // 窗口坐标
        double lfTransXPos, lfTransYPos;        // 转换后的坐标
        
        glfwGetCursorPos(pWindow, &lfXPos, &lfYPos);
        lfTransXPos = lfXPos;
        lfTransYPos = lfYPos;
#ifdef TEST_2D_TRANSLATION
        lfTransXPos = lfXPos*2.0f/g_iDefaultWindowsWidth;
        lfTransYPos = (g_iDefaultWindowsHeight - lfYPos)*2.0f/g_iDefaultWindowsHeight;
        lfTransXPos -= 1.0;
        lfTransYPos -= 1.0;
#endif
        
        if (NULL != pShape && GLFW_PRESS == nAction
            && NULL != pShape->GetMatrixTrans())
        {
            printf("[CBaseWindows::MouseCallBackFunc] xpos[%lf], ypos[%lf]\n", lfXPos, lfYPos);
            glm::vec3 node = glm::vec3(lfXPos, lfYPos, 0);
            int nWidth, nHeight;
            glfwGetWindowSize(pWindow, &nWidth, &nHeight);
            if (pShape->IsVertexLocationIn(node, nWidth, nHeight))
            {
                pShape->GetMatrixTrans()->SetCenterPosX((float)lfTransXPos);
                pShape->GetMatrixTrans()->SetCenterPosY((float)lfTransYPos);
                
                if (fMinZ >= node.z)
                {
                    fMinZ = node.z;
                    pSelectShape = pShape;
                }
            }
        }
        else
        {
            if (NULL != pShape
                && NULL != pShape->GetMatrixTrans()
                && pShape->IsLocateIn())
            {
                if (pShape->IsLocateIn())
                {
                    pShape->GetMatrixTrans()->SetPosMove(lfTransXPos, lfTransYPos);
                }
            }
            
            pShape->SetLocateIn(false);
        }
    }
    
    if (NULL != pSelectShape)
    {
        printf("----------- final z[%f] \n", fMinZ);
        for (it = listShape.begin(); it != listShape.end(); ++it)
        {
            if (pSelectShape != *it && NULL != *it)
            {
                (*it)->SetLocateIn(false);
            }
        }
    }
}

void CBaseWindows::VideoPlayCallBackFunc(void *pData[], int *width, int *height, bool *firstCB, const std::string* strTexture)
{
    if (NULL == pData || NULL == width || NULL == height || NULL == firstCB || NULL == strTexture)
    {
        printf("[CBaseWindows::VideoPlayCallBackFunc] warning: param invalid. pData[%p], width[%p], height[%p], firstCB[%p], strTexture[%p]\n", pData, width, height, firstCB, strTexture);
        return;
    }
    
    CBaseWindows* pBaseWin = CBaseWindowMgr::GetInstance()->GetBaseWindow();
    if (NULL == pBaseWin)
    {
        printf("[CBaseWindows::VideoPlayCallBackFunc] pBaseWin[%p]\n", pBaseWin);
        return;
    }
    // 按窗口X关闭窗口
    if (0 != glfwWindowShouldClose(pBaseWin->GetWindows()))
    {
        exit(0);
    }
    
    // 更新纹理数据
    bool bNeedFresh = false;
    int cnt = CBaseVideoPlay::THREAD_MAX_COUNT;
    for (int i = 0; i < cnt; ++i)
    {
        if (NULL == pData[i])
        {
            continue;
        }
        bNeedFresh = true;
        
        CBaseTexture* pTexture = g_pTextureMgr->GetTexture(strTexture[i]);
        if (NULL == pTexture)
        {
            printf("[CBaseWindows::VideoPlayCallBackFunc] error: texture[%s] is null.\n", strTexture[i].c_str());
            return;
        }
        pTexture->UpdateTexture(pData[i], width[i], height[i], firstCB[i]);
    }
    // 更新窗口显示
    if (bNeedFresh)
    {
        pBaseWin->VideoCallBackShow();
    }
}

GLFWwindow* CBaseWindows::GetWindows()
{
    return m_Windows;
}

std::list<CBaseShape*>& CBaseWindows::GetShapeList()
{
    return m_listShape;
}

void CBaseWindows::AddShape(CBaseShape* pShape)
{
    InsertShape(pShape);
}

void CBaseWindows::BindFrameBufferObj(CBaseFrameBufferObj *pFrameBufferObj)
{
    ReleaseFrameBufferObj();
    m_pFBO = pFrameBufferObj;
}

CBaseFrameBufferObj* CBaseWindows::GetFrameBufferObj()
{
    return m_pFBO;
}

void CBaseWindows::BindTargetShape(CBaseShape *pShape)
{
    ReleaseTargetShape();
    m_pTargetShape = pShape;
}

CBaseShape* CBaseWindows::GetTargetShape()
{
    return m_pTargetShape;
}

void CBaseWindows::BindVideoPlay(CBaseVideoPlay *pVideoPlay)
{
    ReleaseVideoPlay();
    m_pVideoPlay = pVideoPlay;
}

CBaseVideoPlay* CBaseWindows::GetVideoPlay()
{
    return m_pVideoPlay;
}

void CBaseWindows::SetUseFrameBufferObj(bool bUseFrameBufferObj)
{
    m_bUseFrameBufferObj = bUseFrameBufferObj;
}

bool CBaseWindows::IsUseFrameBufferObj()
{
    return m_bUseFrameBufferObj;
}

float CBaseWindows::GetDeltaTime()
{
    return m_fDeltaTime;
}

void CBaseWindows::SetWinWidth(int iWidth)
{
    m_iWinWidth = iWidth;
}

int CBaseWindows::GetWinWidth()
{
    return m_iWinWidth;
}

void CBaseWindows::SetWinHeight(int iHeight)
{
    m_iWinHeight = iHeight;
}

int CBaseWindows::GetWinHeight()
{
    return m_iWinHeight;
}

void CBaseWindows::SetWinTitle(const std::string& strTitle)
{
    m_strTitle = strTitle;
}

void CBaseWindows::SetOpType(int eOpType)
{
    m_eOpType = eOpType;
}

// 设置属性，调用CreateWindows()前设置
void CBaseWindows::SetWindowsAttr(int nAttr, int nValue)
{
    glfwWindowHint(nAttr, nValue);
}

GLFWwindow* CBaseWindows::CreateWindows()
{
    if (NULL != m_Windows)
    {
        return m_Windows;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, g_iDefaultContextVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, g_iDefaultContextVersionMinor);
    
    m_Windows = glfwCreateWindow(m_iWinWidth, m_iWinHeight, m_strTitle.c_str(), NULL, NULL);
    if (NULL == m_Windows)
    {
        return NULL;
    }
    
    return m_Windows;
}

void CBaseWindows::SetInputMode(int nMode, int nValue)
{
    if (!IsWindowValid())
    {
        return;
    }
    
    glfwSetInputMode(m_Windows, nMode, nValue);
}

void CBaseWindows::SetCurrentContext()
{
    if (!IsWindowValid())
    {
        return;
    }
    
    glfwMakeContextCurrent(m_Windows);
}

void CBaseWindows::Show()
{
#ifdef VIDEO_RUN_TEST
    return VideoTestShow();
#endif
    
    return NormalTestShow();
}

void CBaseWindows::VideoCallBackShow()
{
#ifdef VIDEO_RUN_TEST
    if (NULL == m_Windows)
    {
        fprintf(stderr, "[CBaseWindows::Show()] m_Windows[%p]\n", m_Windows);
        return;
    }
#ifdef TEST_FBO
    if (NULL == m_pFBO || NULL == m_pTargetShape)
    {
        fprintf(stderr, "fbo[%p], m_pTargetShape[%p] .\n", m_pFBO, m_pTargetShape);
        return;
    }
#endif
    
#ifdef TEST_FBO
    int nWinWidth, nWinHeight;
    glfwGetWindowSize(m_Windows, &nWinWidth, &nWinHeight);
    m_pFBO->BindFBO(nWinWidth, nWinHeight);
#endif
    g_pShapeManager->ClearBuffer();
    
    ShowShape();
        
#ifdef TEST_FBO
    m_pFBO->UnBindFBO();
    
    g_pShapeManager->ClearBuffer();
    
    m_pTargetShape->Renderer();
#endif
        
    glfwSwapBuffers(m_Windows);
    glfwPollEvents();
#endif
    
    
    // tmp
    if (m_bStartRecord)
    {
        // mac屏幕2倍
        int nWidth = m_iWinWidth * 2;
        int nHeight = m_iWinHeight * 2;
        
        GLsizei bufSize = nWidth * nHeight * sizeof(GLubyte) * 3;
        GLubyte* pImgData = (GLubyte*)malloc(bufSize);
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glReadPixels(0, 0, nWidth, nHeight, GL_RGB, GL_UNSIGNED_BYTE, pImgData);
        
        
        //g_pFFmpegYUV2H264Instance->Record(pImgData, nWidth, nHeight, m_bEndRecord);
        m_pVideoPlay->Record(pImgData, nWidth, nHeight, m_bEndRecord);
        if (m_bEndRecord)
        {
            m_bEndRecord = false;
            m_bStartRecord = false;
        }
        free(pImgData);
    }
    // tmp
}

void CBaseWindows::ScreenShots()
{
    // 成像上下颠倒
    // mac屏幕2倍
    int nWidth = m_iWinWidth * 2;
    int nHeight = m_iWinHeight * 2;
    
    GLsizei bufSize = nWidth * nHeight * sizeof(GLubyte) * 3;
    GLubyte* pImgData = (GLubyte*)malloc(bufSize);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glReadPixels(0, 0, nWidth, nHeight, GL_RGB, GL_UNSIGNED_BYTE, pImgData);
    
    //反转图像 ，否则生成的图像是上下调到的
    SetUpDown(pImgData, nWidth, nHeight);
    
    stbi_write_png("./bmp/screenshots.png", nWidth, nHeight, 3, pImgData, 0);
    stbi_write_jpg("./bmp/screen.jpg", nWidth, nHeight, 3, pImgData, 0);
    //stbi_write_bmp("./bmp/screenshots.bmp", nWidth, nHeight, 3, pImgData);
}

bool CBaseWindows::IsWindowValid()
{
    if (NULL == m_Windows)
    {
        fprintf(stderr, "Windows is NULL,,,,\n");
        return false;
    }
    
    return true;
}

void CBaseWindows::UpdateDeltaTime()
{
    static double lastTime = glfwGetTime();
    double curTime = glfwGetTime();
    m_fDeltaTime = float(curTime - lastTime);
    
    lastTime = curTime;
}

void CBaseWindows::InsertShape(CBaseShape* pShape)
{
    if (NULL == pShape
        || (m_mpShape.end() != m_mpShape.find(pShape) && m_mpShape[pShape] > 0))
    {
        return;
    }
    
    m_mpShape[pShape] = 1;
    m_listShape.push_back(pShape);
}

void CBaseWindows::DeleteShape(CBaseShape* pShape)
{
    if (NULL == pShape || m_mpShape.end() == m_mpShape.find(pShape))
    {
        return;
    }
    m_mpShape[pShape] = 0;
    
    std::list<CBaseShape*>::iterator it = m_listShape.begin();
    for (; it != m_listShape.end();)
    {
        if (*it == pShape)
        {
            delete pShape;
            pShape = NULL;
            
            it = m_listShape.erase(it);
            return;
        }
        else
        {
            ++it;
        }
    }
}

void CBaseWindows::ClearShape()
{
    std::list<CBaseShape*>::iterator it = m_listShape.begin();
    for (; it != m_listShape.end();)
    {
        CBaseShape* pShape = *it;
        if (NULL != pShape)
        {
            delete pShape;
            pShape = NULL;
        }
        
        it = m_listShape.erase(it);
    }
    
    m_listShape.clear();
    m_mpShape.clear();
}

void CBaseWindows::ShowShape()
{
    std::list<CBaseShape*>::iterator it = m_listShape.begin();
    CBaseShape* pShape = NULL;
    for (; it != m_listShape.end(); ++it)
    {
        pShape = *it;
        if (NULL == pShape)
        {
            continue;
        }
        
        if (OP_TYPE_DRAG == m_eOpType)
        {
            UpdateDeltaTime();
            
            if (NULL != pShape->GetMatrixTrans() && pShape->IsLocateIn())
            {
                double lfXPos, lfYPos;
                glfwGetCursorPos(m_Windows, &lfXPos, &lfYPos);
#ifdef TEST_2D_TRANSLATION
                lfXPos = lfXPos*2.0f/g_iDefaultWindowsWidth;
                lfYPos = (g_iDefaultWindowsHeight - lfYPos)*2.0f/g_iDefaultWindowsHeight;
                lfXPos -= 1.0;
                lfYPos -= 1.0;
#endif
                pShape->GetMatrixTrans()->UpdateDirectorByCursorPos((float)lfXPos, (float)lfYPos);
            }
        }
        else if (OP_TYPE_SCALE == m_eOpType)
        {
            if (NULL != pShape->GetMatrixTrans())
            {
                pShape->GetMatrixTrans()->ScaleMatrix();
            }
        }
        
        pShape->Renderer();
    }
}

void CBaseWindows::ReleaseFrameBufferObj()
{
    if (NULL != m_pFBO)
    {
        delete m_pFBO;
        m_pFBO = NULL;
    }
}

void CBaseWindows::ReleaseTargetShape()
{
    if (NULL != m_pTargetShape)
    {
        delete m_pTargetShape;
        m_pTargetShape = NULL;
    }
}

void CBaseWindows::ReleaseVideoPlay()
{
    if (NULL != m_pVideoPlay)
    {
        delete m_pVideoPlay;
        m_pVideoPlay = NULL;
    }
}

void CBaseWindows::VideoTestShow()
{
    if (NULL == m_Windows)
    {
        printf("[CBaseWindows::VideoTestShow()] m_Windows[%p]\n", m_Windows);
        return;
    }
#ifdef TEST_FBO
    if (NULL == m_pFBO || NULL == m_pTargetShape)
    {
        printf("[CBaseWindows::VideoTestShow()] fbo[%p], m_pTargetShape[%p] .\n", m_pFBO, m_pTargetShape);
        return;
    }
#endif
    
    int nWinWidth, nWinHeight;
    glfwGetWindowSize(m_Windows, &nWinWidth, &nWinHeight);
    // 设置中心
#ifndef TEST_2D_TRANSLATION
    glfwPollEvents();
    glfwSetCursorPos(m_Windows, nWinWidth / 2.0, nWinHeight / 2.0);
#endif
    
    if (OP_TYPE_DRAG == m_eOpType)
    {
        glfwSetKeyCallback(m_Windows, CBaseWindows::KeyCallBackFunc);
        glfwSetMouseButtonCallback(m_Windows, CBaseWindows::MouseCallBackFunc);
    }
    
    if (NULL == m_pVideoPlay)
    {
        printf("[CBaseWindows::VideoTestShow()] error: m_pVideoPlay is null.\n");
        return;
    }
    
    m_pVideoPlay->Run(kszSquareImagePath);
    
    int aa = 101010;
    printf("#################### aa = %d $$$$$$$$$$$$$$$$\n\n\n", aa);
}

void CBaseWindows::NormalTestShow()
{
    if (NULL == m_Windows)
    {
        fprintf(stderr, "[CBaseWindows::Show()] m_Windows[%p]\n", m_Windows);
        return;
    }
#ifdef TEST_FBO
    if (NULL == m_pFBO || NULL == m_pTargetShape)
    {
        fprintf(stderr, "fbo[%p], m_pTargetShape[%p] .\n", m_pFBO, m_pTargetShape);
        return;
    }
#endif
    
    int nWinWidth, nWinHeight;
    glfwGetWindowSize(m_Windows, &nWinWidth, &nWinHeight);
    // 设置中心
#ifndef TEST_2D_TRANSLATION
    glfwPollEvents();
    glfwSetCursorPos(m_Windows, nWinWidth / 2.0, nWinHeight / 2.0);
#endif
    
    if (OP_TYPE_DRAG == m_eOpType)
    {
        glfwSetKeyCallback(m_Windows, CBaseWindows::KeyCallBackFunc);
        glfwSetMouseButtonCallback(m_Windows, CBaseWindows::MouseCallBackFunc);
    }
    
    do
    {
#ifdef TEST_FBO
        m_pFBO->BindFBO(nWinWidth, nWinHeight);
#endif
        g_pShapeManager->ClearBuffer();
        
        ShowShape();
        
#ifdef TEST_FBO
        m_pFBO->UnBindFBO();
        
        g_pShapeManager->ClearBuffer();
        
        m_pTargetShape->Renderer();
#endif
        
        glfwSwapBuffers(m_Windows);
        glfwPollEvents();
    } while (glfwWindowShouldClose(m_Windows) == 0
             && glfwGetKey(m_Windows, GLFW_KEY_ESCAPE) != GLFW_PRESS);
}

void CBaseWindows::SetUpDown(unsigned char *pData, int image_width, int image_height)
{
    int index = 3;
    for (int h = 0; h < image_height/2; ++h)
    {
        for (int w = 0; w < image_width; ++w)
        {
            const int iCoordM = index * (h * image_width + w);
            const int iCoordN = index * ((image_height - h -1) * image_width + w);
            unsigned char Tmp = pData[iCoordM];
            pData[iCoordM] = pData[iCoordN];
            pData[iCoordN] = Tmp;
            Tmp = pData[iCoordM+1];
            pData[iCoordM + 1] = pData[iCoordN + 1];
            pData[iCoordN + 1] = Tmp;
            Tmp = pData[iCoordM + 2];
            pData[iCoordM + 2] = pData[iCoordN + 2];
            pData[iCoordN + 2] = Tmp;
        }
    }
}
