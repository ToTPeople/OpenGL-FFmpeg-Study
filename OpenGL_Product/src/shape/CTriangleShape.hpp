//
//  CTriangleShape.hpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/18.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CTriangleShape_hpp
#define CTriangleShape_hpp

#include "CBaseShape.hpp"

class CTriangleShape : public CBaseShape
{
public:
    CTriangleShape();
    virtual ~CTriangleShape();
    
public:
    virtual void Renderer();
    
protected:
    virtual void AfterDraw(bool bOutLine = false);
    
    virtual void RenderBuffer(bool bOutLine = false);
    virtual void RenderTexture(bool bOutLine = false);
    virtual void RenderMaxtrix(bool bOutLine = false);
};

#endif /* CTriangleShape_hpp */
