//
//  CBaseWindows.hpp
//  OpenGL_Product
//  窗口类
//  Created by meitu on 2018/1/12.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CBaseWindows_hpp
#define CBaseWindows_hpp

#include <stdio.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <glm/glm.hpp>


struct GLFWwindow;
class CBaseShape;
class CBaseFrameBufferObj;
class CBaseVideoPlay;

class CBaseWindows
{
public:
    CBaseWindows();
    ~CBaseWindows();
    
public:
    static void KeyCallBackFunc(GLFWwindow *pWindow, int nKey, int nScanMode, int nAction, int nMods);
    static void MouseCallBackFunc(GLFWwindow *pWindow, int nButton, int nAction, int nMods);
    // 视频播放回调函数
    static void VideoPlayCallBackFunc(void* pData[], int *width, int *height, bool *firstCB, const std::string* strTexture);
    
public:
    // 获取窗口实例
    GLFWwindow* GetWindows();
    
    // 获取、添加形状实例
    std::list<CBaseShape*>& GetShapeList();
    void AddShape(CBaseShape* pShape);
    
    // 绑定、获取FBO对象
    void BindFrameBufferObj(CBaseFrameBufferObj* pFrameBufferObj);
    CBaseFrameBufferObj* GetFrameBufferObj();
    
    // 绑定、获取FBO目标绘制对象
    void BindTargetShape(CBaseShape* pShape);
    CBaseShape* GetTargetShape();
    
    // 绑定、获取视频对象
    void BindVideoPlay(CBaseVideoPlay* pVideoPlay);
    CBaseVideoPlay* GetVideoPlay();
    
    // 设置、获取是否使用用FBO属性 ## 未使用，可删除
    void SetUseFrameBufferObj(bool bUseFrameBufferObj);
    bool IsUseFrameBufferObj();
    
    // 获取鼠标、键盘操作间隔时间
    float GetDeltaTime();
    
    // 设置/获取窗口属性
    void SetWinWidth(int iWidth);
    int GetWinWidth();
    void SetWinHeight(int iHeight);
    int GetWinHeight();
    void SetWinTitle(const std::string& strTitle);
    
    // 设置操作类型：拖拽、缩放
    void SetOpType(int eOpType);
    
    // 设置窗口属性，调用CreateWindows()创建窗口实例前设置
    void SetWindowsAttr(int nAttr, int nValue);
    
public:
    // 创建GLFW窗口实例
    GLFWwindow* CreateWindows();
    // 设置GLFW窗口属性
    void SetInputMode(int nMode, int nValue);
    void SetCurrentContext();
    
    // 对外显示接口
    void Show();
    void VideoCallBackShow();
    
    void ScreenShots();
    
protected:
    // glfw窗口是否正确创建
    bool IsWindowValid();
    // 更新操作前后2次间隔时长
    void UpdateDeltaTime();
    
    // 形状管理操作：加入、删除、清空
    void InsertShape(CBaseShape* pShape);
    void DeleteShape(CBaseShape* pShape);
    void ClearShape();
    // 遍历图形list显示
    void ShowShape();
    
    // 释放绑定对象：FBO、目标绘制形状（FBO测试时使用）、视频
    void ReleaseFrameBufferObj();
    void ReleaseTargetShape();
    void ReleaseVideoPlay();
    
    // 显示接口
    void VideoTestShow();
    void NormalTestShow();
    
    void SetUpDown(unsigned char *pData, int image_width, int image_height);
    
private:
    int             m_iWinWidth;            // 窗口宽
    int             m_iWinHeight;           // 窗口高
    std::string     m_strTitle;             // 窗口显示名称
    
    float           m_fDeltaTime;           // 前后绘制间隔时间
    int             m_eOpType;              // 操作类型：拖拽或缩放
    
    GLFWwindow*     m_Windows;              // GLFW窗口实例
    std::list<CBaseShape*>  m_listShape;    // 形状链表
    std::map<CBaseShape*, int> m_mpShape;   // 形状实例是否存在
    
    bool            m_bUseFrameBufferObj;   // 是否使用FBO
    CBaseFrameBufferObj*    m_pFBO;         // FBO对象
    CBaseShape*             m_pTargetShape; // FBO绘制到的形状
    
    CBaseVideoPlay*         m_pVideoPlay;   // 视频播放对象
    
public:
    bool            m_bEndRecord;
    bool            m_bStartRecord;
};

#endif /* CBaseWindows_hpp */
