//
//  CScreenShape.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/15.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "CScreenShape.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CDataObject.hpp"
#include "CShader.hpp"
#include "CBaseTexture.hpp"
#include "CMatrixTrans.hpp"

#include "CDataObjMgr.hpp"
#include "CShaderMgr.hpp"
#include "CTextureMgr.hpp"

// 着色器变量
const char* const kszVertexPos = "vertexPosition_modelspace";
const char* const kszUVValue = "vertexUV";

const char* const kszTextureSampler = "myTextureSampler";
const char* const kszMatrixMVP = "MVP";

CScreenShape::CScreenShape()
: CTriangleShape()
, m_pTexture(NULL)
{
}

CScreenShape::~CScreenShape()
{
    ReleaseTexture();
}

void CScreenShape::SetTexture(CBaseTexture *pTexture)
{
    ReleaseTexture();
    m_pTexture = pTexture;
}

void CScreenShape::RenderTexture(bool bOutLine /* = false */)
{
    CShader* pShader = g_pShaderMgr->GetShader(m_strVertexShaderPath, m_strFragShaderPath);
    if (NULL == pShader || NULL == m_pTexture)
    {
        return;
    }
    
    pShader->UseProgram();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(m_pTexture->GetTarget(), m_pTexture->GetHandleID());
    GLuint textureSampler = pShader->GetAttribByString(SHADER_ATTR_GET_UNIFORM, kszTextureSampler);
    pShader->SendUniform1iToShader(textureSampler, 0);
}

void CScreenShape::RenderMaxtrix(bool bOutLine /* = false */)
{
    CShader* pShader = g_pShaderMgr->GetShader(m_strVertexShaderPath, m_strFragShaderPath);
    if (NULL == pShader)
    {
        return;
    }
    
    pShader->UseProgram();
    GLuint mvpID = pShader->GetAttribByString(SHADER_ATTR_GET_UNIFORM, kszMatrixMVP);
    glm::mat4 mvp = glm::mat4(1.0);
    //mvp = glm::rotate(mvp, glm::radians(180.0f), glm::vec3(0,0,1));
    pShader->SendUniformMatrix4fvToShader(mvpID, 1, mvp);
}

void CScreenShape::ReleaseTexture()
{
    if (NULL != m_pTexture)
    {
        delete m_pTexture;
        m_pTexture = NULL;
    }
}

