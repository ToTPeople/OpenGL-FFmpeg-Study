//
//  CFFmpegVideoPlay.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/29.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "CFFmpegVideoPlay.hpp"
#include <unistd.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
/*
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
*/

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#ifdef __cplusplus
};
#endif

#include "common_define.h"
#include "CShader.hpp"
#include "CShaderMgr.hpp"
#include "CBaseTexture.hpp"

namespace
{
    static const GLfloat g_vertex_buffer_data[] = {
        // 缩小左、右、下
        -0.7f, 1.0f, 0.0f,
        -0.7f, -0.8f, 0.0f,
        0.70f,  1.0f, 0.0f,
        0.70f, -0.80f, 0.0f,
        // 正常充满屏幕
        -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
        1.0f, -1.00f, 0.0f,
    };
    
    static const GLfloat g_uv_buffer_data[] = {
        // 180度旋转texture坐标
        /*1.0, 0.0,
        1.0, 1.0,
        0.0, 0.0,
        0.0, 1.0,*/
        // 向上texture坐标
        0.0, 1.0,
        0.0, 0.0,
        1.0, 1.0,
        1.0, 0.0,
    };
    
    static const GLuint g_index_vertex_data[] = {
        0, 1, 2,
        1, 2, 3,
    };
    
    const char* const kszOtherVtxPath = "other_vertex_shader";
    const char* const kszOtherFragPath = "other_frag_shader";
    
    const char* const kszVtxPos = "vertexPosition_modelspace";
    const char* const kszUVPos = "vertexUV";
    const char* const kszMyTextureSampler = "myTextureSampler";
    
    const char* const kszTexturePath = "441H.jpg";

    const int g_nSleepTime = 30 * 1000; // 单位：微秒
}

CFFmpegVideoPlay* CFFmpegVideoPlay::m_pInstance = NULL;
CFFmpegVideoPlay* CFFmpegVideoPlay::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CFFmpegVideoPlay();
    }
    
    return m_pInstance;
}

CFFmpegVideoPlay::CFFmpegVideoPlay()
: m_strVideoFile("")
, m_bInited(false)
{
}

CFFmpegVideoPlay::~CFFmpegVideoPlay()
{
    if (m_bInited)
    {
        m_bInited = false;
        
        glDeleteBuffers(1, &vtxBuffer);
        glDeleteBuffers(1, &uvBuffer);
        
        delete m_pTexture;
        delete m_pShader;
        
        glfwTerminate();
    }
}

void CFFmpegVideoPlay::SetVideoFilePath(const std::string &strVideoFilePath)
{
    m_strVideoFile = strVideoFilePath;
}

int CFFmpegVideoPlay::InitWindow()
{
    if (m_bInited)
    {
        printf("[CFFmpegVideoPlay::InitWindow()] warning: has initialize before, do not need init again.\n");
        return 0;
    }
    
    m_bInited = true;
    if (!glfwInit())
    {
        printf("Failed to initialize GLFW.\n");
        return -1;
    }
    
    // 创建窗口并初始化
    m_window.SetWinTitle("OpenGL Test");
    m_window.SetWindowsAttr(GLFW_SAMPLES, 4);             // 4个采样数
    if (NULL == m_window.CreateWindows())
    {
        printf("Create windows failed.\n");
        return -1;
    }
    
    m_window.SetCurrentContext();
    m_window.SetInputMode(GLFW_STICKY_KEYS, GL_TRUE);
    m_window.SetInputMode(GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
    
    // 放在创建windows并设置current context后
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize GLEW.\n");
        return -1;
    }
    
    // vertex
    glGenBuffers(1, &vtxBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vtxBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    
    // UV
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
    
    // index
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_index_vertex_data), g_index_vertex_data, GL_STATIC_DRAW);
    
    m_pShader = new CShader();
    if (NULL == m_pShader)
    {
        printf("[CFFmpegVideoPlay::InitWindow()] error: create shader failed.\n");
        return -1;
    }
    m_pShader->SetVertexFilePath(kszOtherVtxPath);
    m_pShader->SetFragmentFilePath(kszOtherFragPath);
    m_pShader->LoadShaders();
    
    m_pShader->UseProgram();
    vtxPos = m_pShader->GetAttribByString(SHADER_ATTR_GET_NORMAL, kszVtxPos);
    uvPos = m_pShader->GetAttribByString(SHADER_ATTR_GET_NORMAL, kszUVPos);
    
    m_pTexture = new CBaseTexture();
    if (NULL == m_pTexture)
    {
        printf("[CFFmpegVideoPlay::InitWindow()] error: create texture failed.\n");
        return -1;
    }
    m_pTexture->SetImagePath(kszTexturePath);
    m_pTexture->LoadTexture(LOAD_TEXTURE_TYPE_BMP);
    
    textureID = m_pShader->GetAttribByString(SHADER_ATTR_GET_UNIFORM, kszMyTextureSampler);
    
    return 0;
}

void CFFmpegVideoPlay::Play()
{
    if (m_strVideoFile.empty())
    {
        printf("[CFFmpegVideoPlay::Play()] error: video file is empty! Please set before play.\n");
        return;
    }
    
    if (!m_bInited && 0 != InitWindow())
    {
        printf("[CFFmpegVideoPlay::Play()] error: initialize windows failed.\n");
        return;
    }
    
    unsigned int i = 0, videoStream = -1;
    AVCodecContext *pCodecCtx;
    AVFormatContext *pFormatCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameRGB;        // 存储非压缩数据
    struct SwsContext *pSwsCtx;
    const char *filename = m_strVideoFile.c_str();
    AVPacket packet;                    // 存储压缩数据
    int frameFinished;
    int PictureSize;
    uint8_t *buf;
    
    //注册编解码器
    av_register_all();
    avformat_network_init();
    //这里是分配一块内存，保存视频的属性信息
    pFormatCtx = avformat_alloc_context();
    //打开视频文件
    if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0)
    {
        printf ("av open input file failed!\n");
        exit (1);
    }
    //获取流信息
    if (avformat_find_stream_info(pFormatCtx,NULL) < 0)
    {
        printf ("av find stream info failed!\n");
        exit (1);
    }
    //获取视频流
    for (i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1)
    {
        printf ("find video stream failed!\n");
        exit (1);
    }
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);         // 获取解码器
    if (pCodec == NULL)
    {
        printf ("avcode find decoder failed!\n");
        exit (1);
    }
    //打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) // Initialize the AVCodecContext to use the given AVCodec.
    {
        printf ("avcode open failed!\n");
        exit (1);
    }
    //为每帧图像分配内存
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    if ((pFrame == NULL) || (pFrameRGB == NULL))
    {
        printf("avcodec alloc frame failed!\n");
        exit (1);
    }
    //获得帧图大小
    enum AVPixelFormat nFormat = AV_PIX_FMT_RGB24;//AV_PIX_FMT_BGR24; // ## 设置不同，形成的图片RGB不一样，显示可能会异常
    // 计算长、宽，格式为nFormat，align为1的帧需要多少存储空间
    PictureSize =  av_image_get_buffer_size(nFormat, pCodecCtx->width, pCodecCtx->height, 1);
    buf = (uint8_t*)av_malloc(PictureSize);
    if (NULL == buf)
    {
        printf( "av malloc failed!\n");
        exit(1);
    }
    // Setup the data pointers and linesizes based on the specified image parameters and the provided array.
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buf, nFormat, pCodecCtx->width, pCodecCtx->height, 1);
    //设置图像转换上下文
    pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, nFormat, SWS_BICUBIC, NULL, NULL, NULL);
    i = 0;
    // Return the next frame of a stream.
    while(av_read_frame(pFormatCtx, &packet) >= 0)
    {
        if (!(glfwWindowShouldClose(m_window.GetWindows()) == 0
         && glfwGetKey(m_window.GetWindows(), GLFW_KEY_ESCAPE) != GLFW_PRESS))
        {
            break;
        }
        
        if(packet.stream_index==videoStream)
        {
            //真正解码
            //avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
            //if(frameFinished)
            frameFinished = avcodec_send_packet(pCodecCtx, &packet);    // Supply raw packet data as input to a decoder.
            frameFinished = avcodec_receive_frame(pCodecCtx, pFrame);   // Return decoded output data from a decoder.
            if (0 == frameFinished)
            {
                //反转图像 ，否则生成的图像是上下调到的
                 pFrame->data[0] += pFrame->linesize[0] * (pCodecCtx->height - 1);
                 pFrame->linesize[0] *= -1;
                 pFrame->data[1] += pFrame->linesize[1] * (pCodecCtx->height / 2 - 1);
                 pFrame->linesize[1] *= -1;
                 pFrame->data[2] += pFrame->linesize[2] * (pCodecCtx->height / 2 - 1);
                 pFrame->linesize[2] *= -1;
                
                //转换图像格式，将解压出来的YUV420P的图像转换为BRG24的图像
                // sws_getContext创建的上下文、原始数据、步长、起始行、终止行、目标数据、目标数据间隔步长
                sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                
                // 绘制
                {
                    glClearColor(0.0f, 0.7f, 0.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT);
                    
                    m_pShader->UseProgram();
                    
                    // 更新纹理
                    m_pTexture->UpdateTexture((pFrameRGB->data[0]), pCodecCtx->width, pCodecCtx->height);
                    
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(m_pTexture->GetTarget(), m_pTexture->GetHandleID());
                    m_pShader->SendUniform1iToShader(textureID, 0);
                    
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
                    
                    glfwSwapBuffers(m_window.GetWindows());
                    glfwPollEvents();
                    
                    usleep(g_nSleepTime);
                }
                
                i++;
            }
            av_packet_unref(&packet);
        }
    }
    
    sws_freeContext(pSwsCtx);
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}
