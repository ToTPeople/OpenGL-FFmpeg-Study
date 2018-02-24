//
//  CShaderMgr.hpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/18.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CShaderMgr_hpp
#define CShaderMgr_hpp

#include <stdio.h>
#include <map>
#include <string>

class CShader;

class CShaderMgr
{
public:
    static CShaderMgr* GetInstance();
    
    bool AddShader(const std::string& strVtxPath, const std::string& strFragPath, CShader* pShader);
    CShader* GetShader(const std::string& strVtxPath, const std::string& strFragPath);
    
    void SetPreUseProgramObj(CShader* pPreUseProgramObj);
    bool IsPreUseProgramObj(CShader* pPreUseProgramObj);
    
private:
    CShaderMgr();
    
private:
    static CShaderMgr* m_pInstance;
    std::map<std::string, CShader*> m_mpShader;
    CShader*           m_pPreUseProgramObj;
};

#define g_pShaderMgr (CShaderMgr::GetInstance())

#endif /* CShaderMgr_hpp */
