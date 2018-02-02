//
//  CVideoPlayMgr.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/2/1.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CVideoPlayMgr.hpp"


CVideoPlayMgr* CVideoPlayMgr::m_pInstance = NULL;

CVideoPlayMgr* CVideoPlayMgr::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CVideoPlayMgr();
    }
    
    return m_pInstance;
}

void CVideoPlayMgr::SetBaseVideoPlay(CBaseVideoPlay* pBaseVideoPlay)
{
    if (m_pBaseVideoPlay == pBaseVideoPlay && NULL != pBaseVideoPlay)
    {
        printf("[CVideoPlayMgr::SetBaseVideoPlay] warning: do not set the same object.\n");
        return;
    }
    
    if (NULL == pBaseVideoPlay)
    {
        printf("[CVideoPlayMgr::SetBaseVideoPlay] warning: pBaseVideoPlay is null.\n");
        return;
    }
    
    if (NULL != m_pBaseVideoPlay)
    {
        printf("[CVideoPlayMgr::SetBaseVideoPlay] warning: had been seted before, do not set again.\n");
        return;
    }
    m_pBaseVideoPlay = pBaseVideoPlay;
}

CBaseVideoPlay* CVideoPlayMgr::GetBaseVideoPlay()
{
    return m_pBaseVideoPlay;
}

CVideoPlayMgr::CVideoPlayMgr()
: m_pBaseVideoPlay(NULL)
{
}
