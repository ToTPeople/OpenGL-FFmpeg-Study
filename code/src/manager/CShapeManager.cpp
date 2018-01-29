//
//  CShapeManager.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/18.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CShapeManager.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "CScreenShape.hpp"
#include "common_define.h"

#include "CDataObject.hpp"
#include "CShader.hpp"
#include "CBaseTexture.hpp"
#include "CMatrixTrans.hpp"
#include "CBufferDataObject.hpp"
#include "CBufferObj.hpp"

#include "CDataObjMgr.hpp"
#include "CTextureMgr.hpp"
#include "CShaderMgr.hpp"

// 数据路径
const char* const kszScreenDataPath = "./screen_data";
const char* const kszSquareDataPath = "./square_data";
const char* const kszCubeDataPath = "./cube_data";
// 着色器路径
const char* const kszVertexShader = "./vertex_shader";
const char* const kszFragementShader = "./frag_shader";
const char* const kszSquareVertexShader = "./square_vertex_shader";
const char* const kszSquareFragementShader = "./square_frag_shader";
// 纹理图片路径
#ifndef TEST_BLEND
const char* const kszSquareImagePath = "./441H.jpg";//"./uvtemplate.bmp";
#else
const char* const kszSquareImagePath = "window.png";//"grass.png";//"./441H.jpg";//"./uvtemplate.bmp";
#endif
const char* const kszCubeImagePath = "./uvmap.DDS";

CShapeManager* CShapeManager::m_pInstance = NULL;

CShapeManager* CShapeManager::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CShapeManager();
    }
    
    return m_pInstance;
}

CShapeManager::CShapeManager()
{
}

CBaseShape* CShapeManager::GenShape(int eShapeType, int eDrawType)
{
    CBaseShape* pShape = GenerateShape(eShapeType);
    if (NULL == pShape)
    {
        printf("[CShapeManager::GenShape] Failed to create. eShapeType[%d]", eShapeType);
        return NULL;
    }
    
    switch (eDrawType)
    {
        case RENDERER_TYPE_SQUARE:
            InitShape(pShape, LOAD_TEXTURE_TYPE_BMP, kszSquareDataPath, kszSquareImagePath, kszSquareVertexShader, kszSquareFragementShader);
            break;
        case RENDERER_TYPE_CUBE:
            InitShape(pShape, LOAD_TEXTURE_TYPE_DDS, kszCubeDataPath, kszCubeImagePath, kszVertexShader, kszFragementShader);
            break;
        case RENDERER_TYPE_SCREEN:
            InitShape(pShape, LOAD_TEXTURE_TYPE_BMP, kszScreenDataPath, "", kszScreenVtxShader, kszScreenFragShader);
            break;
            
        default:
            break;
    }
    
#ifdef TEST_STENCIL
    if (NULL != pShape
        && (RENDERER_TYPE_SQUARE == eDrawType
        || RENDERER_TYPE_CUBE == eDrawType))
    {
        pShape->SetOutline(true);
    }
#endif
    
    return pShape;
}

void CShapeManager::ClearBuffer()
{
#ifdef TEST_STENCIL
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#else
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
}

void CShapeManager::SetBgColor(float fRed, float fGreen, float fBlue, float fAlpha)
{
    glClearColor(fRed, fGreen, fBlue, fAlpha);
}

void CShapeManager::Init()
{
    // 设置远近显示，防止远的显示在近的前面
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // 立方体内部时，应该不能看到后面的面，逆时针三角形的顶点将隐藏
    glEnable(GL_CULL_FACE);
    
    // 颜色混合测试
#ifdef TEST_BLEND
    // 使能混合
    glEnable(GL_BLEND);
    
    // 混合方式，默认add
    glBlendEquation(GL_FUNC_ADD_EXT);// 混合方程式
    //glBlendEquationSeparate(GL_FUNC_SUBTRACT, GL_FUNC_ADD_EXT);
    
    // src、dst混合因子选取
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR);// 混合因子设置
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_DST_ALPHA);
    
    // 针对 混合因子是常量时 颜色混合比重
    glBlendColor(1.0, 1.0, 0.50, 1.0);// GL_CONSTANT_COLOR和GL_ONE_MINUS_CONSTANT_COLOR因子是这两个时才生效
#endif // 颜色混合测试
}

CBaseShape* CShapeManager::GenerateShape(int eShapeType)
{
    CBaseShape* pShape = NULL;
    switch (eShapeType)
    {
        case SHAPE_TYPE_TRIANGLE:
            pShape = new CTriangleShape();
            break;
        case SHAPE_TYPE_SCREEN:
            pShape = new CScreenShape();
            break;
            
        default:
            pShape = new CBaseShape();
            break;
    }
    
    return pShape;
}

void CShapeManager::InitShape(CBaseShape* pShape, int eTextureLoadType, const std::string& strDataPath, const std::string& strTexturePath
                            , const std::string& strVtxShader, const std::string& strFragShader)
{
    if (NULL == pShape)
    {
        printf("[CShapeManager::InitShape] NULL == pShape");
        return;
    }
    
    printf("[CShapeManager::InitShape] imageLoadType[%d], imagePath[%s], dataPath[%s], vtxShader[%s], fragShader[%s]\n"
           , eTextureLoadType, strTexturePath.c_str(), strDataPath.c_str(), strVtxShader.c_str(), strFragShader.c_str());
    
    // 创建/获取数据
    CDataObject* pDataObj = g_pDataObjMgr->GetDataObj(strDataPath);
    if (NULL == pDataObj)
    {
        pDataObj = new CDataObject();
        if (NULL != pDataObj)
        {
            pDataObj->SetDDSPicData((LOAD_TEXTURE_TYPE_DDS == eTextureLoadType));
            pDataObj->SetDataFilePath(strDataPath);
            //pDataObj->SetUseIndex(true);
            //pDataObj->SetUseIndex(false);
            pDataObj->LoadData();
            
            if (g_pDataObjMgr->AddDataObj(strDataPath, pDataObj))
            {
                g_pBufferDataObject->AddData(pDataObj);
            }
        }
    }
    if (NULL == pDataObj)
    {
        printf("[CShapeManager::InitShape] error: create/get data object failed.\n");
        return;
    }
    pShape->SetDataPath(strDataPath);
    
    // 创建/获取 形状shader
    CShader* pShader = g_pShaderMgr->GetShader(strVtxShader, strFragShader);
    if (NULL == pShader)
    {
        pShader = new CShader();
        if (NULL != pShader)
        {
            pShader->SetVertexFilePath(strVtxShader);
            pShader->SetFragmentFilePath(strFragShader);
            pShader->LoadShaders();
            
            g_pShaderMgr->AddShader(strVtxShader, strFragShader, pShader);
        }
    }
    if (NULL == pShader)
    {
        printf("[CShapeManager::InitShape] error: Create/Get shader failed.\n");
        return;
    }
    pShape->SetVertexShaderPath(strVtxShader);
    pShape->SetFragShaderPath(strFragShader);
    // 创建/获取 边框shader
    CShader* pOutlineShader = g_pShaderMgr->GetShader(strVtxShader, kszOutlineFragShader);
    if (NULL == pOutlineShader)
    {
        pOutlineShader = new CShader();
        if (NULL != pOutlineShader)
        {
            pOutlineShader->SetVertexFilePath(strVtxShader);
            pOutlineShader->SetFragmentFilePath(kszOutlineFragShader);
            pOutlineShader->LoadShaders();
            
            g_pShaderMgr->AddShader(strVtxShader, kszOutlineFragShader, pOutlineShader);
        }
    }
    if (NULL == pOutlineShader)
    {
        printf("[CShapeManager::InitShape] error: Create/Get outline shader failed.\n");
        return;
    }
    // 创建/获取 Texture
    CBaseTexture* pTexture = g_pTextureMgr->GetTexture(strTexturePath);
    if (NULL == pTexture)
    {
        pTexture = new CBaseTexture();
        if (NULL != pTexture)
        {
            pTexture->SetImagePath(strTexturePath);
            pTexture->LoadTexture(eTextureLoadType);
            
            if (g_pTextureMgr->AddTexture(strTexturePath, pTexture))
            {
                pShape->SetTexturePath(strTexturePath);
            }
        }
    }
    if (NULL == pTexture)
    {
        printf("[CShapeManager::InitShape] error: Create/Get texture failed.\n");
        return;
    }
    
    CMatrixTrans* pMatrixTrans = new CMatrixTrans();
    if (NULL != pMatrixTrans)
    {
        pMatrixTrans->SetAspectRatio(4.0f / 3.0f);
        if (NULL != pDataObj)
        {
            pMatrixTrans->SetVertexVector(pDataObj->GetVertexsData());
        }
        
        if (LOAD_TEXTURE_TYPE_DDS == eTextureLoadType)
        {
#ifndef SCALE_OPERATOR
            pMatrixTrans->SetScaleRatio(5.0);
#endif
            static int jj = 0;
            //pMatrixTrans->SetMoveXY(jj*0.3, jj*0.3, jj*0.5);//jj*0.6);
            ++jj;
        }
        else if (LOAD_TEXTURE_TYPE_BMP == eTextureLoadType)
        {
            static int ii = 0;
            //pMatrixTrans->SetMoveXY(ii*0.14, ii*0.14, ii*0.6);
            //pMatrixTrans->SetMoveXY(ii*0.1, ii*0.1, ii*0.11);
            ++ii;
        }
        
        pShape->BindMatrixTrans(pMatrixTrans);
    }
    
    SetBgColor(0.0f, 0.0f, 0.4f, 0.0f);
}
