//
//  CVideoPlayMgr.hpp
//  OpenGL_Product
//
//  Created by meitu on 2018/2/1.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CVideoPlayMgr_hpp
#define CVideoPlayMgr_hpp

#include <stdio.h>

class CBaseVideoPlay;

class CVideoPlayMgr
{
public:
    static CVideoPlayMgr* GetInstance();
    
    void SetBaseVideoPlay(CBaseVideoPlay* pBaseVideoPlay);
    CBaseVideoPlay* GetBaseVideoPlay();
    
private:
    CVideoPlayMgr();
    
private:
    static CVideoPlayMgr* m_pInstance;
    CBaseVideoPlay*       m_pBaseVideoPlay;
};

#define g_pVideoPlayMgr (CVideoPlayMgr::GetInstance())

#endif /* CVideoPlayMgr_hpp */
