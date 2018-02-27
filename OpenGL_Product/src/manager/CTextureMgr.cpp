//
//  CTextureMgr.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/18.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "CTextureMgr.hpp"
#include "CBaseTexture.hpp"

namespace
{
#define PIC_FILENAME_LENGTH (128)
}

CTextureMgr* CTextureMgr::m_pInstance = NULL;

CTextureMgr* CTextureMgr::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CTextureMgr();
    }
    
    return m_pInstance;
}

bool CTextureMgr::AddTexture(const std::string& strPath, CBaseTexture* pTexture)
{
    if (NULL == pTexture || strPath.empty())
    {
        printf("[CTextureMgr::AddTexture] pTexture[%p], strPath[%s]\n", pTexture, strPath.c_str());
        return false;
    }
    else if (m_mpTexture.end() != m_mpTexture.find(strPath))
    {
        printf("[CTextureMgr::AddTexture] strPath[%s] has exist.\n", strPath.c_str());
    }
    
    m_mpTexture[strPath] = pTexture;
    printf("[CTextureMgr::AddTexture] strPath[%s], texture[%p] =======\n", strPath.c_str(), pTexture);
    return true;
}

CBaseTexture* CTextureMgr::GetTexture(const std::string& strPath)
{
    if (strPath.empty() || m_mpTexture.end() == m_mpTexture.find(strPath))
    {
        printf("[CTextureMgr::GetTexture] path[%s] is empty, or not exist\n", strPath.c_str());
        return NULL;
    }
    
    return m_mpTexture[strPath];
}

void CTextureMgr::GenNewVideoPlayPic()
{
    ++ m_nVideoPlayPicIndex;
}

std::string CTextureMgr::GetVideoPlayPic()
{
    char strPic[PIC_FILENAME_LENGTH];
    sprintf(strPic, "%s%d", kszVideoPlayImagePath, m_nVideoPlayPicIndex);
    return strPic;
}


CTextureMgr::CTextureMgr()
: m_nVideoPlayPicIndex(0)
{
    m_mpTexture.clear();
}