//
//  CScreenShape.hpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/24.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CScreenShape_hpp
#define CScreenShape_hpp

#include "CTriangleShape.hpp"

class CBaseTexture;

class CScreenShape : public CTriangleShape
{
public:
    CScreenShape();
    virtual ~CScreenShape();
    
public:
    void SetTexture(CBaseTexture* pTexture);
    
protected:
    virtual void RenderTexture(bool bOutLine = false);
    virtual void RenderMaxtrix(bool bOutLine = false);
    
    void ReleaseTexture();
    
private:
    CBaseTexture*       m_pTexture;
};

#endif /* CScreenShape_hpp */
