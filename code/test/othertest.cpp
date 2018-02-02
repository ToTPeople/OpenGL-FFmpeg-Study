//
//  othertest.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/22.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "othertest.hpp"
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "common_define.h"
#include "CBaseWindows.hpp"
#include "CShader.hpp"
#include "CShaderMgr.hpp"
#include "CBaseTexture.hpp"

#define AAA (1.0f)
static const GLfloat g_vertex_buffer_data[] = {
    /*-1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
    1.0f, -1.00f, 0.0f,*/
    
    -0.7f, 1.0f, 0.0f,
    -0.7f, -0.8f, 0.0f,
    0.70f,  1.0f, 0.0f,
    0.70f, -0.80f, 0.0f,
};

static const GLfloat g_uv_buffer_data[] = {
    1.0, 0.0,
    1.0, 1.0,
    0.0, 0.0,
    0.0, 1.0,
    
    0.0, 1.0,//0.748573, 0.750412,
    0.0, 0.0,//0.749279, 0.501284,
    1.0, 1.0,//0.999110, 0.501077,
    1.0, 0.0,//0.999455, 0.750380,
    0.250471, 0.500702,
    0.249682, 0.749677,
};

static const GLuint g_index_vertex_data[] = {
    0, 1, 2,
    1, 2, 3,
};

//f 1/1/1 2/2/1 3/3/1
//f 2/4/2 4/5/2 3/6/2

const char* const kszOtherVtxPath = "other_vertex_shader";
const char* const kszOtherFragPath = "other_frag_shader";

const char* const kszVtxPos = "vertexPosition_modelspace";
const char* const kszUVPos = "vertexUV";
const char* const kszMyTextureSampler = "myTextureSampler";

const char* const kszTexturePath = "./bmp/filename_0.bmp";//"441H.jpg";


int DrawElementTest()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW.\n");
        return -1;
    }
    
    // 创建窗口并初始化
    CBaseWindows window;
    window.SetWinTitle("OpenGL Test");
    window.SetWindowsAttr(GLFW_SAMPLES, 4);             // 4个采样数
    if (NULL == window.CreateWindows())
    {
        fprintf(stderr, "Create windows failed.\n");
        return -1;
    }
    
    window.SetCurrentContext();
    window.SetInputMode(GLFW_STICKY_KEYS, GL_TRUE);
    window.SetInputMode(GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
    
    // 放在创建windows并设置current context后
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW.\n");
        return -1;
    }
    
    GLuint vtxBuffer;
    glGenBuffers(1, &vtxBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vtxBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    
    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
    
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_index_vertex_data), g_index_vertex_data, GL_STATIC_DRAW);
    
    CShader* pShader = new CShader();
    if (NULL == pShader)
    {
        return -1;
    }
    pShader->SetVertexFilePath(kszOtherVtxPath);
    pShader->SetFragmentFilePath(kszOtherFragPath);
    pShader->LoadShaders();
    
    pShader->UseProgram();
    GLuint vtxPos = pShader->GetAttribByString(SHADER_ATTR_GET_NORMAL, kszVtxPos);
    GLuint uvPos = pShader->GetAttribByString(SHADER_ATTR_GET_NORMAL, kszUVPos);
    
    CBaseTexture* pTexture = new CBaseTexture();
    if (NULL == pTexture)
    {
        return -1;
    }
    pTexture->SetImagePath(kszTexturePath);
    pTexture->LoadTexture(LOAD_TEXTURE_TYPE_BMP);
    
    GLuint textureID = pShader->GetAttribByString(SHADER_ATTR_GET_UNIFORM, kszMyTextureSampler);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(pTexture->GetTarget(), pTexture->GetHandleID());
    pShader->SendUniform1iToShader(textureID, 0);
    
#ifdef TEST_FBO
    // 帧缓冲
    GLuint frameBuffer;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    // texture
    GLuint texColorBuffer;
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window.GetWinWidth(), window.GetWinHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    // attach
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);
    
    // check
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
        fprintf(stderr, "Framebuffer is not complete.\n");
        return -1;
    }
    // ------------
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // init
    float arrQuadVertices[] = {
        // positions
        -1.0f,  1.0f,  0.0f,
        -1.0f, -1.0f,  0.0f,
        //0.0f, -1.0f,  0.0f,
        1.0f, -1.0f,  0.0f,
        
        -1.0f,  1.0f,  0.0f,
        1.0f, -1.0f,  0.0f,
        1.0f,  1.000f,  0.0f,
    };
    float arrUV[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };
    GLuint screenIdx[] = {
        0, 1, 2,
        3, 4, 5,
    };
    
    const char* const strSampler = "myTextureSampler";
    const char* const strVtx = "vertexPosition_modelspace";
    const char* const strUV = "vertexUV";
    
    GLuint vtxScreenBuffer;
    glGenBuffers(1, &vtxScreenBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vtxScreenBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrQuadVertices), arrQuadVertices, GL_STATIC_DRAW);
    
    GLuint uvScreenBuffer;
    glGenBuffers(1, &uvScreenBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvScreenBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrUV), arrUV, GL_STATIC_DRAW);
    
    GLuint idxScreenBuffer;
    glGenBuffers(1, &idxScreenBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxScreenBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screenIdx), screenIdx, GL_STATIC_DRAW);
    
    // shader
    CShader* pScreenShader = new CShader();
    if (NULL == pScreenShader)
    {
        return -1;
    }
    pScreenShader->SetVertexFilePath(kszScreenVtxShader);
    pScreenShader->SetFragmentFilePath(kszScreenFragShader);
    pScreenShader->LoadShaders();
    
    GLuint samplerID = pScreenShader->GetAttribByString(SHADER_ATTR_GET_UNIFORM, strSampler);
    GLuint vtxID = pScreenShader->GetAttribByString(SHADER_ATTR_GET_NORMAL, strVtx);
    GLuint uvID = pScreenShader->GetAttribByString(SHADER_ATTR_GET_NORMAL, strUV);
    
#endif
    
    int idx = 0;
    char *filename = new char[64];
    
    do
    {
        if (idx > 298)
        {
            break;
        }
        
#ifdef TEST_FBO
        //glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, window.GetWinWidth()*2, window.GetWinHeight()*2);
#endif
#if 1
        glClearColor(0.0f, 0.7f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        pShader->UseProgram();
        
        // 更新纹理
        sprintf(filename, "./bmp/filename_%d.bmp", idx++);
        pTexture->UpdateTexture(std::string(filename));
        printf("[############ 333]  idx = %d \n", idx);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(pTexture->GetTarget(), pTexture->GetHandleID());
        pShader->SendUniform1iToShader(textureID, 0);
        
        glEnableVertexAttribArray(vtxPos);
        glBindBuffer(GL_ARRAY_BUFFER, vtxBuffer);
        glVertexAttribPointer(vtxPos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glEnableVertexAttribArray(uvPos);
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glVertexAttribPointer(uvPos, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glDisableVertexAttribArray(vtxPos);
        glDisableVertexAttribArray(uvPos);
#endif
        
#ifdef TEST_FBO
#if 0
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glViewport(0, 0, window.GetWinWidth()*2, window.GetWinHeight()*2);
        //glViewport(0, 0, window.GetWinWidth(), window.GetWinHeight());
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // shader
        pScreenShader->UseProgram();
        // texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        pScreenShader->SendUniform1iToShader(samplerID, 0);
        
        // vtx
        glEnableVertexAttribArray(vtxID);
        glBindBuffer(GL_ARRAY_BUFFER, vtxScreenBuffer);
        glVertexAttribPointer(vtxID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // uv
        glEnableVertexAttribArray(uvID);
        glBindBuffer(GL_ARRAY_BUFFER, uvScreenBuffer);
        glVertexAttribPointer(uvID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // idx
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxScreenBuffer);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(vtxID);
        glDisableVertexAttribArray(uvID);
#endif
#endif
        
        glfwSwapBuffers(window.GetWindows());
        glfwPollEvents();
        
        usleep(30 * 1000);
    } while (glfwWindowShouldClose(window.GetWindows()) == 0
             && glfwGetKey(window.GetWindows(), GLFW_KEY_ESCAPE) != GLFW_PRESS);
    
    glDeleteBuffers(1, &vtxBuffer);
    glDeleteBuffers(1, &uvBuffer);
#ifdef TEST_FBO
    glDeleteBuffers(1, &vtxScreenBuffer);
    glDeleteBuffers(1, &uvScreenBuffer);
#endif
    
    delete pTexture;
    delete pShader;
    
    glfwTerminate();
    
    return 0;
}
