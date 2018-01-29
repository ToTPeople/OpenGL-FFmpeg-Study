//
//  CBaseWindowMgr.hpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/15.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CBaseWindowMgr_hpp
#define CBaseWindowMgr_hpp

#include <stdio.h>

class CBaseWindows;

class CBaseWindowMgr
{
public:
    static CBaseWindowMgr* GetInstance();
    
    void SetBaseWindow(CBaseWindows* pBaseWindow);
    CBaseWindows* GetBaseWindow();
    
private:
    CBaseWindowMgr();
    
    static CBaseWindowMgr* pInstance;
    
private:
    CBaseWindows*       m_pBaseWindow;
};

#endif /* CBaseWindowMgr_hpp */
