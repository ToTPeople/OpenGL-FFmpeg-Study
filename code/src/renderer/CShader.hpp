//
//  CShader.hpp
//  OpenGL_Product
//  着色器处理类
//  Created by lifushan on 2018/1/12.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CShader_hpp
#define CShader_hpp

#include <stdio.h>
#include <iostream>
#include <string>

#include <glm/glm.hpp>

enum ShaderAttributesGet
{
    SHADER_ATTR_GET_NORMAL = 0,
    SHADER_ATTR_GET_UNIFORM,        // 获取Uniform变量
};

// 只加载单个着色器
class CShader
{
public:
    CShader();
    virtual ~CShader();
    
public:
    void SetVertexFilePath(const std::string strVtxFilePath);
    void SetFragmentFilePath(const std::string strFragFilePath);
    
public:
    virtual bool LoadShaders();
    
    void UseProgram();
    // 着色器变量属性
    unsigned int GetAttribByString(int nType, const std::string& strAttr);
    void SendUniform1iToShader(int nAttr, int nValue);
    void SendUniformMatrix4fvToShader(int nAttr, int nCnt, glm::mat4& value);
    
protected:
    bool IsProgramValid();
    
protected:
    std::string         m_strVertexFilePath;        // 顶点着色器路径
    std::string         m_strFragmentFilePath;      // 片着色器路径
    
    unsigned int        m_uProgramID;               // 生成程序ID
    bool                m_bUsedProgram;
};

#endif /* CShader_hpp */
