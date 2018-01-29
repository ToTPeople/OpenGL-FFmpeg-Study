//
//  CBaseWindowMgr.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/15.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CBaseWindowMgr.hpp"
#include "CBaseWindows.hpp"

CBaseWindowMgr* CBaseWindowMgr::pInstance = NULL;

CBaseWindowMgr* CBaseWindowMgr::GetInstance()
{
    if (NULL == pInstance)
    {
        pInstance = new CBaseWindowMgr();
    }
    
    return pInstance;
}

void CBaseWindowMgr::SetBaseWindow(CBaseWindows* pBaseWindow)
{
    m_pBaseWindow = pBaseWindow;
}

CBaseWindows* CBaseWindowMgr::GetBaseWindow()
{
    return m_pBaseWindow;
}

CBaseWindowMgr::CBaseWindowMgr()
{
}

