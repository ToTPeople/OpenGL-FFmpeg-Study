//
//  CBaseShape.hpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/18.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CBaseShape_hpp
#define CBaseShape_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>

class CMatrixTrans;
class CShader;

class CBaseShape
{
public:
    CBaseShape();
    virtual ~CBaseShape();
    
public:
    void BindMatrixTrans(CMatrixTrans* pMatrixTrans);
    
    CMatrixTrans* GetMatrixTrans();
    
    void SetDataPath(const std::string& strDataPath);
    void GetDataPath(std::string& strDataPath);
    
    void SetTexturePath(const std::string& strTexturePath);
    void GetTexturePath(std::string& strTexturePath);
    
    void SetVertexShaderPath(const std::string& strVertexShaderPath);
    void GetVertexShaderPath(std::string& strVertexShaderPath);
    
    void SetFragShaderPath(const std::string& strFragShaderPath);
    void GetFragShaderPath(std::string& strFragShaderPath);
    
    void SetLocateIn(bool bLocateIn);
    bool IsLocateIn();
    
    void SetOutline(bool bOutline);
    
public:
    // 判断点是否在二维坐标系图形上
    virtual bool IsVertexLocationIn(glm::vec3 &node, int nWidth, int nHeight);
    
    // 鼠标、键盘处理
    virtual void DealWithKeyEvent(int nKey, float fDeltaTime, bool bPress);
    //virtual void DealWithMouseEvent(int nMouse);
    
public:
    // 渲染
    virtual void Renderer() {}
    
protected:
    void ReleaseMatrixTrans();
    
    virtual void AfterDraw(bool bOutLine = false) {}
    
    virtual void RenderBuffer(bool bOutLine = false) {}
    virtual void RenderTexture(bool bOutLine = false) {}
    virtual void RenderMaxtrix(bool bOutLine = false) {}
    
    // 获取z轴深度
    void CalZValue(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, glm::vec3& node);
    // 计算三角形面积
    float CalTriangleArea(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c);
    
    CShader* GetShader(bool bOutLine);
    
protected:
    CMatrixTrans*               m_pMatrixTrans;
    
    std::string                 m_strDataPath;
    std::string                 m_strTexturePath;
    std::string                 m_strVertexShaderPath;
    std::string                 m_strFragShaderPath;
    
    bool                        m_bIsLocateIn;
    bool                        m_bOutLine;
};

#endif /* CBaseShape_hpp */
