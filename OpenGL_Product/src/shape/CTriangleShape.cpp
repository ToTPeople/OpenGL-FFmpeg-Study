//
//  CTriangleShape.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/15.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "CTriangleShape.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "CDataObject.hpp"
#include "CShader.hpp"
#include "CBaseTexture.hpp"
#include "CMatrixTrans.hpp"
#include "CBufferObj.hpp"

#include "CDataObjMgr.hpp"
#include "CShaderMgr.hpp"
#include "CTextureMgr.hpp"

namespace
{
    // 着色器变量
    const char* const kszVertexPos = "vertexPosition_modelspace";
    const char* const kszUVValue = "vertexUV";
    
    const char* const kszTextureSampler = "myTextureSampler";
    const char* const kszMatrixMVP = "MVP";
    
#define DEFAULT_SCALE_OUTLINE       (1.035f)
}

CTriangleShape::CTriangleShape()
: CBaseShape()
{
}

CTriangleShape::~CTriangleShape()
{
}

void CTriangleShape::Renderer()
{
    CDataObject* pDataObj = g_pDataObjMgr->GetDataObj(m_strDataPath);
    if (NULL == pDataObj)
    {
        printf("[CTriangleShape::Renderer()] Data object not exist, please create first.\n");
        return CBaseShape::Renderer();
    }
    
    if (m_bOutLine)
    {
        // 使能，并标记形状区域
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xff);
        glStencilFunc(GL_ALWAYS, 1, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }
    // 绘制形状
    RenderMaxtrix();
    RenderTexture();
    RenderBuffer();
    
    if (pDataObj->IsUseIndex())
    {
        int nOff = sizeof(unsigned short) * pDataObj->GetBufferIndicesOff();
        glDrawElements(GL_TRIANGLES, pDataObj->GetIndices().size(), GL_UNSIGNED_SHORT, (GLvoid*)nOff);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, pDataObj->GetVertexsData().size());
    }
    AfterDraw();
    
    if (m_bOutLine)
    {
        // 设置只绘制扩大（即边框）部分
        glStencilFunc(GL_NOTEQUAL, 1, 0xff);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        // 绘制边框
        RenderMaxtrix(true);
        RenderBuffer(true);
        if (pDataObj->IsUseIndex())
        {
            int nOff = sizeof(unsigned short) * pDataObj->GetBufferIndicesOff();
            glDrawElements(GL_TRIANGLES, pDataObj->GetIndices().size(), GL_UNSIGNED_SHORT, (GLvoid*)nOff);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, pDataObj->GetVertexsData().size());
        }
        AfterDraw(true);
        // 恢复设置
        glEnable(GL_DEPTH_TEST);
        glStencilMask(0xff);
    }
}

void CTriangleShape::AfterDraw(bool bOutLine /* = false */)
{
    CShader* pShader = GetShader(bOutLine);
    if (NULL == pShader)
    {
        return;
    }
    
    pShader->UseProgram();
    // vertex
    GLuint vtxPos = pShader->GetAttribByString(SHADER_ATTR_GET_NORMAL, kszVertexPos);
    glDisableVertexAttribArray(vtxPos);
    // UV
    GLuint uvValue = pShader->GetAttribByString(SHADER_ATTR_GET_NORMAL, kszUVValue);
    glDisableVertexAttribArray(uvValue);
}

void CTriangleShape::RenderBuffer(bool bOutLine /* = false */)
{
    CShader* pShader = GetShader(bOutLine);
    CDataObject* pData = g_pDataObjMgr->GetDataObj(m_strDataPath);
    if (NULL == pShader || NULL == pData)
    {
        return;
    }
    
    pShader->UseProgram();
    int nOff = sizeof(PackedVertex) * pData->GetBufferVertexOff();
    // 顶点
    GLuint vtxPos = pShader->GetAttribByString(SHADER_ATTR_GET_NORMAL, kszVertexPos);
    glEnableVertexAttribArray(vtxPos);
    glBindBuffer(g_pBufferObj->GetTarget(), g_pBufferObj->GetHandleID());
    glVertexAttribPointer(vtxPos, 3, GL_FLOAT, GL_FALSE, sizeof(PackedVertex), (GLvoid*)nOff);
    
    // UV
    GLuint uvValue = pShader->GetAttribByString(SHADER_ATTR_GET_NORMAL, kszUVValue);
    glEnableVertexAttribArray(uvValue);
    glBindBuffer(g_pBufferObj->GetTarget(), g_pBufferObj->GetHandleID());
    glVertexAttribPointer(uvValue, 2, GL_FLOAT, GL_FALSE, sizeof(PackedVertex), (GLvoid*)(nOff + sizeof(glm::vec3)));
    
    // index
    glBindBuffer(g_pBufferObj->GetIndexTarget(), g_pBufferObj->GetIndexHandleID());
}

void CTriangleShape::RenderTexture(bool bOutLine /* = false */)
{
    CBaseTexture* pTexture = g_pTextureMgr->GetTexture(m_strTexturePath);
    CShader* pShader = GetShader(bOutLine);
    if (NULL == pShader || NULL == pTexture)
    {
        return;
    }
    
    pShader->UseProgram();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(pTexture->GetTarget(), pTexture->GetHandleID());
    GLuint textureSampler = pShader->GetAttribByString(SHADER_ATTR_GET_UNIFORM, kszTextureSampler);
    pShader->SendUniform1iToShader(textureSampler, 0);
}

void CTriangleShape::RenderMaxtrix(bool bOutLine /* = false */)
{
    CDataObject* pDataObj = g_pDataObjMgr->GetDataObj(m_strDataPath);
    CShader* pShader = GetShader(bOutLine);
    if (NULL == pShader || NULL == m_pMatrixTrans)
    {
        return;
    }
    
    pShader->UseProgram();
    GLuint mvpID = pShader->GetAttribByString(SHADER_ATTR_GET_UNIFORM, kszMatrixMVP);
    m_pMatrixTrans->SetScaleOutline(bOutLine ? DEFAULT_SCALE_OUTLINE : 1.0f);
    glm::mat4 mvp = m_pMatrixTrans->GetMVPMatrix();
    pShader->SendUniformMatrix4fvToShader(mvpID, 1, mvp);
}
