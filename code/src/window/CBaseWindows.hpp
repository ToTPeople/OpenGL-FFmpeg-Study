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

class CBaseWindows
{
public:
    CBaseWindows();
    ~CBaseWindows();
    
public:
    static void KeyCallBackFunc(GLFWwindow *pWindow, int nKey, int nScanMode, int nAction, int nMods);
    static void MouseCallBackFunc(GLFWwindow *pWindow, int nButton, int nAction, int nMods);
    
public:
    GLFWwindow* GetWindows();
    std::list<CBaseShape*>& GetShapeList();
    void AddShape(CBaseShape* pShape);
    
    void BindFrameBufferObj(CBaseFrameBufferObj* pFrameBufferObj);
    CBaseFrameBufferObj* GetFrameBufferObj();
    
    void BindTargetShape(CBaseShape* pShape);
    CBaseShape* GetTargetShape();
    
    void SetUseFrameBufferObj(bool bUseFrameBufferObj);
    bool IsUseFrameBufferObj();
    
    float GetDeltaTime();
    
    void SetWinWidth(int iWidth);
    int GetWinWidth();
    void SetWinHeight(int iHeight);
    int GetWinHeight();
    void SetWinTitle(const std::string& strTitle);
    void SetOpType(int eOpType);
    
    // 设置窗口属性，调用CreateWindows()创建窗口实例前设置
    void SetWindowsAttr(int nAttr, int nValue);
    
public:
    GLFWwindow* CreateWindows();
    
    void SetInputMode(int nMode, int nValue);
    void SetCurrentContext();
    
    void Show();
    
protected:
    bool IsWindowValid();
    void UpdateDeltaTime();
    
    void InsertShape(CBaseShape* pShape);
    void DeleteShape(CBaseShape* pShape);
    void ClearShape();
    // 遍历图形list显示
    void ShowShape();
    
    void ReleaseFrameBufferObj();
    void ReleaseTargetShape();
    
private:
    int             m_iWinWidth;
    int             m_iWinHeight;
    std::string     m_strTitle;
    
    float           m_fDeltaTime;           // 前后绘制间隔时间
    int             m_eOpType;              // 操作类型：拖拽或缩放
    
    GLFWwindow*     m_Windows;
    std::list<CBaseShape*>  m_listShape;
    std::map<CBaseShape*, int> m_mpShape;
    
    bool            m_bUseFrameBufferObj;
    CBaseFrameBufferObj*    m_pFBO;
    CBaseShape*             m_pTargetShape; // FBO绘制到的形状
};

#endif /* CBaseWindows_hpp */
