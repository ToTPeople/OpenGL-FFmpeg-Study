//
//  CShapeManager.hpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/18.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CShapeManager_hpp
#define CShapeManager_hpp

#include <stdio.h>
#include <cstring>
#include <string>

class CBaseShape;

class CShapeManager
{
public:
    static CShapeManager* GetInstance();
    
    CBaseShape* GenShape(int eShapeType, int eDrawType, bool bVideoPlay = false);   // 第二个参数需要优化掉
    
public:
    // clear OpenGL default frame buffer
    void ClearBuffer();
    // set OpenGl clear backgroud color
    void SetBgColor(float fRed, float fGreen, float fBlue, float fAlpha);
    void Init();
    
private:
    CShapeManager();
    
    CBaseShape* GenerateShape(int eShapeType);
    void InitShape(CBaseShape* pShape, int eTextureLoadType, const std::string& strDataPath, const std::string& strTexturePath
                                , const std::string& strVtxShader, const std::string& strFragShader, bool bVideoPlay = false);
    
private:
    static CShapeManager* m_pInstance;
};

#define g_pShapeManager (CShapeManager::GetInstance())

#endif /* CShapeManager_hpp */
