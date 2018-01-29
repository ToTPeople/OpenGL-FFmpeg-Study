//
//  CMatrixTrans.hpp
//  OpenGL_Product
//  坐标系统处理类
//  Created by meitu on 2018/1/12.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CMatrixTrans_hpp
#define CMatrixTrans_hpp

#include <stdio.h>
#include <vector>
#include <glm/glm.hpp>

struct GLFWwindow;




class CMatrixTrans
{
public:
    CMatrixTrans();
    virtual ~CMatrixTrans();
    
public:
    // 属性
    void SetHorizontalAngle(float fHorizontalAngle);
    void SetVerticalAngle(float fVerticalAngle);
    void SetCenterPosX(float fCenterPosX);
    void SetCenterPosY(float fCenterPosY);
    void SetPosMove(float fPosX, float fPosY);
    void SetSpeed(float fSpeed);
    void SetMouseSpeed(float fMouseSpeed);
    void SetAspectRatio(float fAspectRatio);
    void SetScaleOutline(float fScaleOutline);
    void SetNearClippingPlane(float fNearClippingPlane);
    void SetFarClippingPlane(float fFarClippingPlane);
    void SetFoV(float fFoV);
    void SetVertexVector(std::vector<glm::vec3> &vecVtx);
    void SetScaleRatio(float fScaleRatio);
    void SetMoveXY(float fPosX, float fPosY, float fPosZ);
    
public:
    glm::mat4 GetProjectionMatrix();
    glm::mat4 GetViewMatrix();
    glm::mat4 GetMVPMatrix();
    // 鼠标、键盘事件响应
    virtual void UpdatePositionByArrowKey(int eArrowKey, float fDeltaTime);
    virtual void UpdateDirectorByCursorPos(float fXPos, float fYPos);
    // 收放矩阵
    virtual void ScaleMatrix();
    // 更新xOy上x、y平移量
    void UpdateTransMatrix(float fXPos, float fYPos);
    
protected:
    float           m_fHorizontalAngle;         // horizontal angle : toward -Z
    float           m_fVerticalAngle;           // vertical angle : 0, look at the horizon
    float           m_fCenterPosX;
    float           m_fCenterPosY;
    float           m_fScaleFrequency;          // 缩放频
    float           m_fScaleRatio;              // 综放比例
    float           m_fScaleOutline;            // 边框缩放比
    
    float           m_fSpeed;                   // 相机移动速度（键盘上下左右键盘设置）
    float           m_fMouseSpeed;              // 鼠标移动速度
    float           m_fAspectRatio;             // 成像比例
    float           m_fNearClippingPlane;       // 最近可看到的距离
    float           m_fFarClippingPlane;        // 最远可看到的距离
    float           m_fFoV;                     // 角度
    glm::vec3       m_vec3_up;                  // 相机向上方向 向量
    glm::vec3       m_vec3_direction;           // 相机镜头方向 向量
    glm::vec3       m_vec3_position;            // 相机位置 向量
    glm::vec3       m_vec3_right;
    std::vector<glm::vec3>  m_vec_vtxs;         // 顶点数据
    float           m_arr_mvp[16];              // 2D平移矩阵
    float           m_fTmpPosMoveX;
    float           m_fTmpPosMoveY;
};

#endif /* CMatrixTrans_hpp */
