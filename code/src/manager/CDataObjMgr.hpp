//
//  CCDataObjMgr.hpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/18.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CDataObjMgr_hpp
#define CDataObjMgr_hpp

#include <stdio.h>
#include <map>
#include <string>

class CDataObject;

class CDataObjMgr
{
public:
    static CDataObjMgr* GetInstance();
    
    bool AddDataObj(const std::string& strPath, CDataObject* pDataObj);
    CDataObject* GetDataObj(const std::string& strPath);
    
private:
    CDataObjMgr();
    
private:
    static CDataObjMgr* m_pInstance;
    std::map<std::string, CDataObject*> m_mpDataObj;
};

#define g_pDataObjMgr (CDataObjMgr::GetInstance())

#endif /* CDataObjMgr_hpp */
