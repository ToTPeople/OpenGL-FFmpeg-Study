//
//  CTextureMgr.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/18.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CTextureMgr.hpp"
#include "CBaseTexture.hpp"


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


CTextureMgr::CTextureMgr()
{
    m_mpTexture.clear();
}
