//
//  CTextureMgr.hpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/18.
//  Copyright © 2018年 lifs. All rights reserved.
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
    
    // add and get texture object
    bool AddTexture(const std::string& strPath, CBaseTexture* pTexture);
    CBaseTexture* GetTexture(const std::string& strPath);
    
    // generate and get VideoPlay picture name
    void GenNewVideoPlayPic();
    std::string GetVideoPlayPic();
    
private:
    CTextureMgr();
    
private:
    static CTextureMgr* m_pInstance;
    std::map<std::string, CBaseTexture*> m_mpTexture;
    int                 m_nVideoPlayPicIndex;           // how much VideoPlay picture has generate
};

#define g_pTextureMgr (CTextureMgr::GetInstance())

#endif /* CTextureMgr_hpp */
