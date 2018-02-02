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
    
    // 添加、获取 texture 对象
    bool AddTexture(const std::string& strPath, CBaseTexture* pTexture);
    CBaseTexture* GetTexture(const std::string& strPath);
    
    // 生成、获取video play图片名称
    void GenNewVideoPlayPic();
    std::string GetVideoPlayPic();
    
private:
    CTextureMgr();
    
private:
    static CTextureMgr* m_pInstance;
    std::map<std::string, CBaseTexture*> m_mpTexture;
    int                 m_nVideoPlayPicIndex;
};

#define g_pTextureMgr (CTextureMgr::GetInstance())

#endif /* CTextureMgr_hpp */
