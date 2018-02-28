//
//  CDataObjMgr.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/18.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "CDataObjMgr.hpp"
#include "CDataObject.hpp"


CDataObjMgr* CDataObjMgr::m_pInstance = NULL;

CDataObjMgr* CDataObjMgr::GetInstance()
{
    if (NULL == m_pInstance) {
        m_pInstance = new CDataObjMgr();
    }
    
    return m_pInstance;
}

bool CDataObjMgr::AddDataObj(const std::string& strPath, CDataObject* pDataObj)
{
    if (NULL == pDataObj || strPath.empty()) {
        printf("[CDataObjMgr::AddDataObj] error: pDataObj[%p], strPath[%s]\n", pDataObj, strPath.c_str());
        return false;
    } else if (m_mpDataObj.end() != m_mpDataObj.find(strPath)) {
        printf("[CDataObjMgr::AddDataObj] warning: strPath[%s] has exist.\n", strPath.c_str());
        return false;
    }
    
    m_mpDataObj[strPath] = pDataObj;
    return true;
}

CDataObject* CDataObjMgr::GetDataObj(const std::string& strPath)
{
    if (strPath.empty() || m_mpDataObj.end() == m_mpDataObj.find(strPath)) {
        printf("[CDataObjMgr::GetDataObj] warning: path[%s] is empty, or not exist\n", strPath.c_str());
        return NULL;
    }
    
    return m_mpDataObj[strPath];
}


CDataObjMgr::CDataObjMgr()
{
    m_mpDataObj.clear();
}
