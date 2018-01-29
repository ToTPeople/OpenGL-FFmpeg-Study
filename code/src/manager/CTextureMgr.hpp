//
//  CTextureMgr.hpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/18.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CTextureMgr_hpp
#define CTextureMgr_hpp

#include <stdio.h>
#include <map>
#include <string>

class CBaseTexture;

class CTextureMgr
{
public:
    static CTextureMgr* GetInstance();
    
    bool AddTexture(const std::string& strPath, CBaseTexture* pTexture);
    CBaseTexture* GetTexture(const std::string& strPath);
    
private:
    CTextureMgr();
    
private:
    static CTextureMgr* m_pInstance;
    std::map<std::string, CBaseTexture*> m_mpTexture;
};

#define g_pTextureMgr (CTextureMgr::GetInstance())

#endif /* CTextureMgr_hpp */
