//
//  CShader.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/12.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "CShader.hpp"
#include <unistd.h>
#include <fstream>
#include <vector>
#include <GL/glew.h>
#include "CShaderMgr.hpp"

CShader::CShader()
: m_uProgramID(0)
, m_strVertexFilePath("")
, m_strFragmentFilePath("")
, m_bUsedProgram(false)
{
}

CShader::~CShader()
{
    printf("[CShader::~CShader()] IsProgramValid[%d] m_uProgramID[%d] \n", IsProgramValid(), m_uProgramID);
    if (IsProgramValid() && m_bUsedProgram)
    {
        glDeleteProgram(m_uProgramID);
    }
}

void CShader::SetVertexFilePath(const std::string strVtxFilePath)
{
    m_strVertexFilePath = strVtxFilePath;
}

void CShader::SetFragmentFilePath(const std::string strFragFilePath)
{
    m_strFragmentFilePath = strFragFilePath;
}

bool CShader::LoadShaders()
{
    if (m_strVertexFilePath.empty())
    {
        printf("[CShader::LoadShaders()] vertex path is empty, please set path first.\n");
        return false;
    }
    
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);        // 创建着色器对象
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(m_strVertexFilePath, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }else{
        char * dir = getcwd(NULL, 0); // Platform-dependent, see reference link below
        printf("Current dir: %s\n\n", dir);
        
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", m_strVertexFilePath.c_str());
        getchar();
        return 0;
    }
    
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(m_strFragmentFilePath, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }else{
        char * dir = getcwd(NULL, 0); // Platform-dependent, see reference link below
        printf("Current dir: %s\n\n", dir);
        
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", m_strFragmentFilePath.c_str());
        getchar();
        return 0;
    }
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    
    // Compile Vertex Shader
    printf("Compiling shader : %s\n", m_strVertexFilePath.c_str());
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL); // 存源码到指定ID，， ID 、数组个数、源码数组、NULL表示数组以NULL结尾 或 其他值表示数组长度
    glCompileShader(VertexShaderID);     // 编译存有源码的着色器对象
    
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }
    
    
    
    // Compile Fragment Shader
    printf("Compiling shader : %s\n", m_strFragmentFilePath.c_str());
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s === \n", &FragmentShaderErrorMessage[0]);
    }
    
    
    
    // Link the program
    printf("Linking program\n");
    m_uProgramID = glCreateProgram();  // 创建程序对象，可关联着色器，有生成程序机制
    glAttachShader(m_uProgramID, VertexShaderID);  // 关联着色器（允许 在装载源码、编译前关联结；多个着色器 可 关联多个程序）
    glAttachShader(m_uProgramID, FragmentShaderID);
    glLinkProgram(m_uProgramID);  // 链接生成程序（如果已在使用，则添加更新的；如果失败，则之前在使用的是旧的，直到重新调用glUseProgram())
    
    // Check the program
    glGetProgramiv(m_uProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(m_uProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(m_uProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s :-(\n", &ProgramErrorMessage[0]);
    }
    
    
    glDetachShader(m_uProgramID, VertexShaderID); // 删除关联
    glDetachShader(m_uProgramID, FragmentShaderID);
    
    glDeleteShader(VertexShaderID); // 删除着色器，，和关联没有前后关系
    glDeleteShader(FragmentShaderID);
    
    return true;
}

void CShader::UseProgram()
{
    if (!IsProgramValid())
    {
        return;
    }
    
    if (g_pShaderMgr->IsPreUseProgramObj(this))
    {
        return;
    }
    
    g_pShaderMgr->SetPreUseProgramObj(this);
    
    m_bUsedProgram = true;
    glUseProgram(m_uProgramID);
}

unsigned int CShader::GetAttribByString(int nType, const std::string& strAttr)
{
    if (!IsProgramValid())
    {
        return 0;
    }
    
    int nAttr = 0;
    
    switch (nType) {
        case SHADER_ATTR_GET_NORMAL:
            nAttr = glGetAttribLocation(m_uProgramID, strAttr.c_str());
            break;
        case SHADER_ATTR_GET_UNIFORM:
            nAttr = glGetUniformLocation(m_uProgramID, strAttr.c_str());
            break;
            
        default:
            break;
    }
    
    return nAttr;
}

void CShader::SendUniform1iToShader(int nAttr, int nValue)
{
    if (!IsProgramValid())
    {
        return;
    }
    
    glUniform1i(nAttr, nValue);
}

void CShader::SendUniformMatrix4fvToShader(int nAttr, int nCnt, glm::mat4& value)
{
    if (!IsProgramValid())
    {
        return;
    }
    
    glUniformMatrix4fv(nAttr, nCnt, GL_FALSE, &value[0][0]);
}

bool CShader::IsProgramValid()
{
    if (0 == m_uProgramID)
    {
        fprintf(stderr, "#################### not create shader program, please complime first.\n");
        return false;
    }
    
    return true;
}
