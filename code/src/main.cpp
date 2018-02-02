//
//  main.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/12.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include <stdio.h>
#include "Test.hpp"
#include "othertest.hpp"
#include "ffmpegtest.hpp"
//#include "CFFmpegVideoPlay.hpp"
#include "CFFmpegYUV2H264.hpp"

int main(int argc, const char * argv[])
{
    return OpenGLTest();
    //return DrawElementTest();
    //return FFmpegTest();
    //return FFmpeg2pic();
    
    g_pFFmpegYUV2H264Instance->SetYUVFilePath("./video/ds_480x272.yuv");
    g_pFFmpegYUV2H264Instance->Run();
    
    return 0;
}
