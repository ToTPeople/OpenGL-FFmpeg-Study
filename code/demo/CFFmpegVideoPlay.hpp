//
//  CFFmpegVideoPlay.hpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/29.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CFFmpegVideoPlay_hpp
#define CFFmpegVideoPlay_hpp

#include <stdio.h>
#include <string>

#include "CBaseWindows.hpp"

class CShader;
class CBaseTexture;

class CFFmpegVideoPlay
{
public:
    static CFFmpegVideoPlay* GetInstance();
    
public:
    void SetVideoFilePath(const std::string& strVideoFilePath);
    void Play();
    
private:
    // 初始化窗口数据，返回0成功，其他失败
    int InitWindow();
    
private:
    CFFmpegVideoPlay();
    ~CFFmpegVideoPlay();
    
private:
    static CFFmpegVideoPlay* m_pInstance;
    
    std::string              m_strVideoFile;
    bool                     m_bInited;
    CBaseWindows             m_window;
    unsigned int vtxBuffer;
    unsigned int uvBuffer;
    unsigned int indexBuffer;
    unsigned int textureID;
    unsigned int vtxPos;
    unsigned int uvPos;
    CShader* m_pShader;
    CBaseTexture* m_pTexture;
};

#define g_pFFmpegVideoPlayInstance (CFFmpegVideoPlay::GetInstance())

#endif /* CFFmpegVideoPlay_hpp */
