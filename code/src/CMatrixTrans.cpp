//
//  CMatrixTrans.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/12.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CMatrixTrans.hpp"
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "common_define.h"
#include "commonfun.hpp"

CMatrixTrans::CMatrixTrans()
: m_fHorizontalAngle(3.14f)
, m_fVerticalAngle(0.0f)
, m_fCenterPosX(0)
, m_fCenterPosY(0)
, m_fScaleFrequency(0.1)
, m_fScaleOutline(1.0)
#ifdef TEST_2D_TRANSLATION
, m_fScaleRatio(1.0)//(2.0)
#else
, m_fScaleRatio(1.0)
#endif
, m_fSpeed(7.0f)
, m_fMouseSpeed(0.005f)
, m_fAspectRatio(1.0f)
, m_fNearClippingPlane(0.1f)
, m_fFarClippingPlane(100.0f)
, m_fFoV(45.0f)
, m_fTmpPosMoveX(0.0)
, m_fTmpPosMoveY(0.0)
{
    m_vec3_position = glm::vec3(0, 0, 5);
    m_vec3_direction = glm::vec3(cos(m_fVerticalAngle) * sin(m_fHorizontalAngle),
                                 sin(m_fVerticalAngle),
                                 cos(m_fVerticalAngle) * cos(m_fHorizontalAngle));
    
    m_vec3_right = glm::vec3(sin(m_fHorizontalAngle - 3.14f/2.0f),
                             0,
                             cos(m_fHorizontalAngle - 3.14f/2.0f));
    
    m_vec3_up = glm::cross(m_vec3_right, m_vec3_direction);
    
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (i == j) m_arr_mvp[i*4 + j] = 1.0 / m_fScaleRatio;
            else m_arr_mvp[i*4 + j] = 0.0;
            
            m_arr_mvp[15] = 1.0;
            
            printf("%f ", m_arr_mvp[i*4 + j]);
        }
        printf("\n");
    }
}

CMatrixTrans::~CMatrixTrans()
{
    fprintf(stdout, "$$$$$$$$$$$$$$$$$$\n");
    printf("[CMatrixTrans::~CMatrixTrans()] \n");
}

void CMatrixTrans::SetHorizontalAngle(float fHorizontalAngle)
{
    m_fHorizontalAngle = fHorizontalAngle;
}

void CMatrixTrans::SetVerticalAngle(float fVerticalAngle)
{
    m_fVerticalAngle = fVerticalAngle;
}

void CMatrixTrans::SetCenterPosX(float fCenterPosX)
{
    m_fCenterPosX = fCenterPosX;
}

void CMatrixTrans::SetCenterPosY(float fCenterPosY)
{
    m_fCenterPosY = fCenterPosY;
}

void CMatrixTrans::SetPosMove(float fPosX, float fPosY)
{
    //printf("[CMatrixTrans::SetPosMove] tmpPosX[%f], tmpPosY[%f], fX[%f], fY[%f]\n", m_fTmpPosMoveX, m_fTmpPosMoveY, fPosX - m_fCenterPosX, fPosY - m_fCenterPosY);
    m_fTmpPosMoveX += fPosX - m_fCenterPosX;
    m_fTmpPosMoveY += fPosY - m_fCenterPosY;
}

void CMatrixTrans::SetSpeed(float fSpeed)
{
    m_fSpeed = fSpeed;
}

void CMatrixTrans::SetMouseSpeed(float fMouseSpeed)
{
    m_fMouseSpeed = fMouseSpeed;
}

void CMatrixTrans::SetAspectRatio(float fAspectRatio)
{
    m_fAspectRatio = fAspectRatio;
}

void CMatrixTrans::SetScaleOutline(float fScaleOutline)
{
    m_fScaleOutline = fScaleOutline;
}

void CMatrixTrans::SetNearClippingPlane(float fNearClippingPlane)
{
    m_fNearClippingPlane = fNearClippingPlane;
}

void CMatrixTrans::SetFarClippingPlane(float fFarClippingPlane)
{
    m_fFarClippingPlane = fFarClippingPlane;
}

void CMatrixTrans::SetFoV(float fFoV)
{
    m_fFoV = fFoV;
}

void CMatrixTrans::SetVertexVector(std::vector<glm::vec3> &vecVtx)
{
    CopyVertex(m_vec_vtxs, vecVtx);
}

void CMatrixTrans::SetScaleRatio(float fScaleRatio)
{
    m_fScaleRatio = fScaleRatio;
    m_arr_mvp[0] = m_arr_mvp[5] = m_arr_mvp[10] = 1.0 / m_fScaleRatio;
}

void CMatrixTrans::SetMoveXY(float fPosX, float fPosY, float fPosZ)
{
    m_fTmpPosMoveX = fPosX;
    m_fTmpPosMoveY = fPosY;
    m_arr_mvp[12] = m_fTmpPosMoveX;
    m_arr_mvp[13] = m_fTmpPosMoveY;
    m_arr_mvp[14] = fPosZ;
}

glm::mat4 CMatrixTrans::GetProjectionMatrix()
{
    return glm::perspective(glm::radians(m_fFoV), m_fAspectRatio, m_fNearClippingPlane, m_fFarClippingPlane);
}

glm::mat4 CMatrixTrans::GetViewMatrix()
{
    return glm::lookAt(m_vec3_position*m_fScaleRatio, (m_vec3_position + m_vec3_direction), m_vec3_up);
}

glm::mat4 CMatrixTrans::GetMVPMatrix()
{
#ifdef TEST_2D_TRANSLATION
    glm::mat4 mvp = glm::make_mat4(m_arr_mvp);
    mvp = glm::rotate(mvp, 45.0f, glm::vec3(1,0,0));
    mvp = glm::scale(mvp, glm::vec3(m_fScaleOutline));
    return (mvp);
#else
    glm::mat4 Model(1.0f);
    return (GetProjectionMatrix() * GetViewMatrix() * Model);
#endif
}

void CMatrixTrans::UpdatePositionByArrowKey(int eArrowKey, float fDeltaTime)
{
    // Move forward
    if (ARROW_KEY_TYPE_UP == eArrowKey)
    {
        m_vec3_position += m_vec3_direction * fDeltaTime * m_fSpeed;
    }
    
    // Move backward
    if (ARROW_KEY_TYPE_DOWN == eArrowKey)
    {
        m_vec3_position -= m_vec3_direction * fDeltaTime * m_fSpeed;
    }
    
    // Strafe right
    if (ARROW_KEY_TYPE_RIGHT == eArrowKey)
    {
        m_vec3_position += m_vec3_right * fDeltaTime * m_fSpeed;
    }
    
    // Strafe left
    if (ARROW_KEY_TYPE_LEFT == eArrowKey)
    {
        m_vec3_position -= m_vec3_right * fDeltaTime * m_fSpeed;
    }
}

void CMatrixTrans::UpdateDirectorByCursorPos(float fXPos, float fYPos)
{
#ifdef TEST_2D_TRANSLATION
    UpdateTransMatrix(fXPos - m_fCenterPosX, fYPos - m_fCenterPosY);
#else
    // Compute new orientation
    m_fHorizontalAngle += m_fMouseSpeed * float(m_fCenterPosX - fXPos);
    m_fVerticalAngle += m_fMouseSpeed * float(m_fCenterPosY - fYPos);
    
    // Direction : Spherical coordinates to Cartesian coordinates conversion
    m_vec3_direction = glm::vec3(cos(m_fVerticalAngle) * sin(m_fHorizontalAngle),
                                 sin(m_fVerticalAngle),
                                 cos(m_fVerticalAngle) * cos(m_fHorizontalAngle));
    
    // Right vector
    m_vec3_right = glm::vec3(sin(m_fHorizontalAngle - 3.14f/2.0f),
                             0,
                             cos(m_fHorizontalAngle - 3.14f/2.0f));
    
    // Up vector : perpendicular to both direction and right
    m_vec3_up = glm::cross(m_vec3_right, m_vec3_direction);
    
    SetCenterPosX(fXPos);
    SetCenterPosY(fYPos);
#endif
}

void CMatrixTrans::ScaleMatrix()
{
    bool bSleep = false;
    m_fScaleRatio += m_fScaleFrequency;
#ifdef TEST_2D_TRANSLATION
    if (m_fScaleRatio >= 15.0f || m_fScaleRatio <= 1.999999)
#else
    if (m_fScaleRatio >= 5.0f || m_fScaleRatio <= 0.999999)
#endif
    {
        m_fScaleFrequency *= (-1.0f);
        m_fScaleRatio += m_fScaleFrequency;
        bSleep = true;
    }
    // 设置矩阵 缩放比例
    m_arr_mvp[0] = m_arr_mvp[5] = m_arr_mvp[10] = 1.0 / m_fScaleRatio;
    
    if (bSleep)
    {
        usleep(500);
    }
}

void CMatrixTrans::UpdateTransMatrix(float fXPos, float fYPos)
{
    //printf("[CMatrixTrans::UpdateTransMatrix] arr12[%f], arr13[%f], tmpPosX[%f], tmpPosY[%f], fX[%f], fY[%f]\n"
      //     , m_arr_mvp[12], m_arr_mvp[13], m_fTmpPosMoveX, m_fTmpPosMoveY, fXPos, fYPos);
    // 设置矩阵x、y轴平移量
    m_arr_mvp[12] = m_fTmpPosMoveX + fXPos;
    m_arr_mvp[13] = m_fTmpPosMoveY + fYPos;
}

