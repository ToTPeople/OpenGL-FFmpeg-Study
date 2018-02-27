//
//  CBaseShape.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/18.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "CBaseShape.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "common_define.h"
#include "CDataObject.hpp"
#include "CShader.hpp"
#include "CBaseTexture.hpp"
#include "CDataObjMgr.hpp"
#include "CMatrixTrans.hpp"
#include "CShaderMgr.hpp"

CBaseShape::CBaseShape()
: m_pMatrixTrans(NULL)
, m_strDataPath("")
, m_strTexturePath("")
, m_strVertexShaderPath("")
, m_strFragShaderPath("")
, m_bIsLocateIn(false)
, m_bOutLine(false)
{
}

CBaseShape::~CBaseShape()
{
    ReleaseMatrixTrans();
}

void CBaseShape::BindMatrixTrans(CMatrixTrans *pMatrixTrans)
{
    m_pMatrixTrans = pMatrixTrans;
}

CMatrixTrans* CBaseShape::GetMatrixTrans()
{
    return m_pMatrixTrans;
}

void CBaseShape::SetDataPath(const std::string& strDataPath)
{
    m_strDataPath = strDataPath;
}

void CBaseShape::GetDataPath(std::string& strDataPath)
{
    strDataPath = m_strDataPath;
}

void CBaseShape::SetTexturePath(const std::string& strTexturePath)
{
    m_strTexturePath = strTexturePath;
}

void CBaseShape::GetTexturePath(std::string& strTexturePath)
{
    strTexturePath = m_strTexturePath;
}

void CBaseShape::SetVertexShaderPath(const std::string& strVertexShaderPath)
{
    m_strVertexShaderPath = strVertexShaderPath;
}

void CBaseShape::GetVertexShaderPath(std::string& strVertexShaderPath)
{
    strVertexShaderPath = m_strVertexShaderPath;
}

void CBaseShape::SetFragShaderPath(const std::string& strFragShaderPath)
{
    m_strFragShaderPath = strFragShaderPath;
}

void CBaseShape::GetFragShaderPath(std::string& strFragShaderPath)
{
    strFragShaderPath = m_strFragShaderPath;
}

 void CBaseShape::SetLocateIn(bool bLocateIn)
{
    m_bIsLocateIn = bLocateIn;
}

bool CBaseShape::IsLocateIn()
{
    return m_bIsLocateIn;
}

void CBaseShape::SetOutline(bool bOutline)
{
    m_bOutLine = bOutline;
}

bool CBaseShape::IsVertexLocationIn(glm::vec3 &node, int nWidth, int nHeight)
{
    CDataObject* pDataObj = g_pDataObjMgr->GetDataObj(m_strDataPath);
    if (NULL == pDataObj || NULL == m_pMatrixTrans)
    {
        printf("[CBaseShape::IsVertexLocationIn] NULL == pDataObj\n");
        return false;
    }
    bool bFind = false;
    bool bUseIndex = false;
    
    std::vector<glm::vec3> vecVertexs;
    pDataObj->GetVertexsData(vecVertexs);
    int nSize = vecVertexs.size();
    // 是否使用序号进行索引
    std::vector<unsigned short> vecIndices;
    if (bUseIndex = pDataObj->IsUseIndex())
    {
        pDataObj->GetIndices(vecIndices);
        nSize = vecIndices.size();
    }
    if (nSize < 3)
    {
        return false;
    }
    
    float fMaxZ = MIN_Z_DEPTH;
    glm::mat4 mvp = m_pMatrixTrans->GetMVPMatrix();
    glm::vec3 vecRange = glm::vec3(1.0, 1.0, 0.0);
#ifndef TEST_2D_TRANSLATION
    vecRange = mvp * glm::vec4(vecRange, 1.0);
#endif
    
    printf("-=-=--=- x[%f], y[%f], nSize[%d] $$$$$$\n", node.x, node.y, nSize);
    glm::vec3 transNode;
    transNode.x = node.x*2.0f*vecRange.x/nWidth;
    transNode.y = (nHeight - node.y)*2.0f*vecRange.y/nHeight;
    transNode.z = 0.0;
    transNode.x -= vecRange.x;
    transNode.y -= vecRange.y;
    
    SetLocateIn(false);
    
    for (int i = 0; i+2 < nSize; i += 3)
    {
        // 如果是序号，则更新index值
        int nIdx1 = bUseIndex ? vecIndices[i] : i;
        int nIdx2 = bUseIndex ? vecIndices[i+1] : (i+1);
        int nIdx3 = bUseIndex ? vecIndices[i+2] : (i+2);
        
        glm::vec3 a = mvp * glm::vec4(vecVertexs[nIdx1], 1.0);
        glm::vec3 b = mvp * glm::vec4(vecVertexs[nIdx2], 1.0);
        glm::vec3 c = mvp * glm::vec4(vecVertexs[nIdx3], 1.0);
        float fTmpAz = a.z;
        float fTmpBz = b.z;
        float fTmpCz = c.z;
        //printf("-=-=-=-=-=-=- i[%d] a(%f,%f,%f), b(%f,%f,%f), c(%f,%f,%f)\n"
        //       , i, a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z);
        a.z = b.z = c.z = 0.0;      // 矩阵缩放、旋转、平移后，投影到xOy平面
        transNode.z = 0.0;
        
        float fS = CalTriangleArea(a, b, c);
        float fS1 = CalTriangleArea(a, b, transNode);
        float fS2 = CalTriangleArea(a, c, transNode);
        float fS3 = CalTriangleArea(b, c, transNode);
        /*if (-1 == i)
        {
            printf("===========================================\n\n");
            printf("a.x[%f], a.y[%f], a.z[%f]; b.x[%f], b.y[%f], b.z[%f]; c.x[%f], c.y[%f], c.z[%f]; node.x[%f], node.y[%f]\n"
                   , a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z, transNode.x, transNode.y);
            printf("S=%f, ST=%f, S1=%f, S2=%f, S3=%f\n"
                   , fS, (fS1+fS2+fS3), fS1, fS2, fS3);
            printf("=======================End====================\n\n");
        }*/
         
        if (fabs(fS1 + fS2 + fS3 - fS) <= 0.0005)
        {
            // 取得Z轴最大值，不要直接返回
            SetLocateIn(true);
            a.z = fTmpAz;
            b.z = fTmpBz;
            c.z = fTmpCz;
            CalZValue(a, b, c, transNode);
            if (fMaxZ < transNode.z)
            {
                fMaxZ = transNode.z;
            }
            printf("^^^^^^^^^^^^^^ path[%s], z[%f], fMaxZ[%f], a(%f,%f,%f), b(%f,%f,%f), c(%f,%f,%f)\n"
                   , m_strDataPath.c_str(), transNode.z, fMaxZ
                   , a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z);
            bFind = true;
        }
    }
    
    node.z = fMaxZ;
    return bFind;
}

void CBaseShape::ReleaseMatrixTrans()
{
    if (NULL != m_pMatrixTrans)
    {
        delete m_pMatrixTrans;
        m_pMatrixTrans = NULL;
    }
}

void CBaseShape::DealWithKeyEvent(int nKey, float fDeltaTime, bool bPress)
{
    if (bPress && NULL != m_pMatrixTrans)
    {
        m_pMatrixTrans->UpdatePositionByArrowKey(nKey, fDeltaTime);
    }
}

void CBaseShape::CalZValue(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, glm::vec3& node)
{
    // 计算平面方程式 Ax + By + Cz + D = 0
    float fA, fB, fC, fD;
    float x1 = a.x;
    float y1 = a.y;
    float z1 = a.z;
    float x2 = b.x;
    float y2 = b.y;
    float z2 = b.z;
    float x3 = c.x;
    float y3 = c.y;
    float z3 = c.z;
    //
    /*
    fA = y1*(z2-z3) + y2*(z3-z1) + y3*(z1-z2);
    fB = z1*(x2-x3) + z2*(x3-x1) + z3*(x1-x2);
    fC = x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2);
    fD = -x1*(y2*z3 - y3*z2) - x2*(y3*z1 - y1*z3) - x3*(y1*z2 - y2*z1);
    */
    fA = (y2-y1)*(z3-z1) - (y3-y1)*(z2-z1);
    fB = (z2-z1)*(x3-x1) - (z3-z1)*(x2-x1);
    fC = (x2-x1)*(y3-y1) - (x3-x1)*(y2-y1);
    fD = -fA*x1 - fB*y1 - fC*z1;
    // 代入方程式，求z坐标  z = -((D + Ax + By) / C)
    node.z = -((fD + fA*node.x + fB*node.y) / fC);
}

float CBaseShape::CalTriangleArea(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c)
{
    float area = -1;
    
    float side[3];//存储三条边的长度;
    
    side[0] = sqrt(pow(a.x - b.x,2)+pow(a.y - b.y,2) + pow(a.z - b.z,2));
    side[1] = sqrt(pow(a.x - c.x,2)+pow(a.y - c.y,2) + pow(a.z - c.z,2));
    side[2] = sqrt(pow(c.x - b.x,2)+pow(c.y - b.y,2) + pow(c.z - b.z,2));
    
    //不能构成三角形;
    if ((side[0]+side[1]<=side[2]) || (side[0]+side[2]<=side[1]) || (side[1]+side[2]<=side[0]))
    {
        return area;
    }
    
    //利用海伦公式。s=sqr(p*(p-a)(p-b)(p-c));
    float p = (side[0]+side[1]+side[2])/2; //半周长;
    area = sqrt(p*(p-side[0])*(p-side[1])*(p-side[2]));
    
    return area;
}

CShader* CBaseShape::GetShader(bool bOutLine)
{
    CShader* pShader = bOutLine ? g_pShaderMgr->GetShader(m_strVertexShaderPath, kszOutlineFragShader) : g_pShaderMgr->GetShader(m_strVertexShaderPath, m_strFragShaderPath);
    
    return pShader;
}

