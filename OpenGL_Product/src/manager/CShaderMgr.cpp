//
//  CShaderMgr.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/18.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "CShaderMgr.hpp"
#include "CShader.hpp"


CShaderMgr* CShaderMgr::m_pInstance = NULL;

CShaderMgr* CShaderMgr::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CShaderMgr();
    }
    
    return m_pInstance;
}

bool CShaderMgr::AddShader(const std::string& strVtxPath, const std::string& strFragPath, CShader* pShader)
{
    if (NULL == pShader || strVtxPath.empty())
    {
        printf("[CShaderMgr::AddShader] pShader[%p], strVtxPath[%s], strFragPath[%s]\n"
               , pShader, strVtxPath.c_str(), strFragPath.c_str());
        return false;
    }
    
    std::string strTmp = strVtxPath + strFragPath;
    if (m_mpShader.end() != m_mpShader.find(strTmp))
    {
        printf("[CShaderMgr::AddShader] strPath[%s] has exist.\n", strTmp.c_str());
    }
    
    m_mpShader[strTmp] = pShader;
    return true;
}

CShader* CShaderMgr::GetShader(const std::string& strVtxPath, const std::string& strFragPath)
{
    std::string strTmp = strVtxPath + strFragPath;
    if (strTmp.empty() || m_mpShader.end() == m_mpShader.find(strTmp))
    {
        printf("[CShaderMgr::GetShader] path[%s] is empty, or not exist\n", strTmp.c_str());
        return NULL;
    }
    
    return m_mpShader[strTmp];
}

void CShaderMgr::SetPreUseProgramObj(CShader *pPreUseProgramObj)
{
    m_pPreUseProgramObj = pPreUseProgramObj;
}

bool CShaderMgr::IsPreUseProgramObj(CShader *pPreUseProgramObj)
{
    return (NULL != pPreUseProgramObj && m_pPreUseProgramObj == pPreUseProgramObj);
}

CShaderMgr::CShaderMgr()
: m_pPreUseProgramObj(NULL)
{
    m_mpShader.clear();
}
