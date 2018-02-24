//
//  main.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/12.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include <stdio.h>
#include "Test.hpp"
#include "othertest.hpp"
#include "ffmpegtest.hpp"
#include "CFFmpegYUV2H264.hpp"
#include "mux_test.hpp"

int main(int argc, const char * argv[])
{
//    return mux_mp4();
    return OpenGLTest();
    //return DrawElementTest();
    //return FFmpegTest();
    //return FFmpeg2pic();
    
    g_pFFmpegYUV2H264Instance->SetYUVFilePath("./video/ds_480x272.yuv");
    g_pFFmpegYUV2H264Instance->Run();
    
    return 0;
}
