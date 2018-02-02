//
//  CBaseVideoPlay.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/30.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CBaseVideoPlay.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include "common_define.h"
#include "CVideoPlayMgr.hpp"

namespace
{
    const int g_nDecodeSleepTime = 50 * 1000; // 单位：微秒
    const int g_nSendDataSleepTime = 20 * 1000;
    
    const int g_nQueueDeep = 3000;
}

CBaseVideoPlay::CBaseVideoPlay(int eOpType, const std::string& strVideoPath)
: m_pFuncVideoCB(NULL)
, m_eOpType(eOpType)
, m_strVideoPath(strVideoPath)
, m_strSavePath("")
, m_bInit(false)
, m_nPre(0)
, m_nBack(0)
, m_nThreadCnt(0)
{
}

CBaseVideoPlay::~CBaseVideoPlay()
{
}


void CBaseVideoPlay::Play(const char* filename, const std::string& strTexture)
{
    unsigned int idx, nVideoStreamIdx = -1;
    AVFormatContext* pFmtContext;       // 格式内容
    AVCodecContext* pCodecCtx;          // 解码内容
    AVCodec*        pCodec;             // 解码器
    
    if (NULL == filename)
    {
        printf("[CBaseVideoPlay::Play] error: filename[%p].\n", filename);
        return;
    }
    
    pFmtContext = avformat_alloc_context();
    if (NULL == pFmtContext)
    {
        printf("[CBaseVideoPlay::Play] error: alloc AVFormatContext failed.\n");
        exit(1);
    }
    
    // 打开文件，获取mux
    if (0 != avformat_open_input(&pFmtContext, filename, NULL, NULL))
    {
        printf("[CBaseVideoPlay::Play] error: avformat_open_input failed.\n");
        exit(1);
    }
    // 获取流信息
    if (0 > avformat_find_stream_info(pFmtContext, NULL))
    {
        printf("[CBaseVideoPlay::Play] error: avformat_find_stream_info failed.\n");
        exit(1);
    }
    // 判断是否是视频
    for (idx = 0; idx < pFmtContext->nb_streams; ++idx)
    {
        //if (AVMEDIA_TYPE_VIDEO == pFmtContext->streams[idx]->codec->codec_type)
        if (AVMEDIA_TYPE_VIDEO == pFmtContext->streams[idx]->codecpar->codec_type)
        {
            nVideoStreamIdx = idx;
            break;
        }
    }
    if (-1 == nVideoStreamIdx)
    {
        printf("[CBaseVideoPlay::Play] error: not a video file.\n");
        exit(1);
    }
    
    // 获取压缩头信息
    AVCodecParameters* pCodecParame;
    pCodecParame = pFmtContext->streams[nVideoStreamIdx]->codecpar;
    //AVCodecParameters;
    pCodecCtx = pFmtContext->streams[nVideoStreamIdx]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (0 != avcodec_open2(pCodecCtx, pCodec, NULL))
    {
        printf("[CBaseVideoPlay::Play] error: avcodec_open2 failed.\n");
        exit(1);
    }
    
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGB = av_frame_alloc();
    SwsContext* pSwsCtx;
    enum AVPixelFormat nFormat = AV_PIX_FMT_RGB24;
    int nPicSize;
    if (NULL == pFrame || NULL == pFrameRGB)
    {
        printf("[CBaseVideoPlay::Play] error: av_frame_alloc failed.\n");
        exit(1);
    }
    
    nPicSize = av_image_get_buffer_size(nFormat, pCodecCtx->width, pCodecCtx->height, 1);
    uint8_t *buf = (uint8_t*)av_malloc(nPicSize);
    if (NULL == buf)
    {
        printf("[CBaseVideoPlay::Play] error: av_malloc failed.\n");
        exit(1);
    }
    
    pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, nFormat, SWS_BICUBIC, NULL, NULL, NULL);
    if (NULL == pSwsCtx)
    {
        printf("[CBaseVideoPlay::Play] error: sws_getContext failed.\n");
        exit(1);
    }
    
    if (0 > av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buf, nFormat, pCodecCtx->width, pCodecCtx->height, 1))
    {
        printf("[CBaseVideoPlay::Play] error: av_image_fill_arrays failed.\n");
        exit(1);
    }
    
    idx = 0;
    AVPacket packet;
    while (av_read_frame(pFmtContext, &packet) >= 0)
    {
        if (nVideoStreamIdx == packet.stream_index)
        {
            int nResult = avcodec_send_packet(pCodecCtx, &packet);
            nResult += avcodec_receive_frame(pCodecCtx, pFrame);
            if (0 == nResult)
            {
                //反转图像 ，否则生成的图像是上下调到的
                pFrame->data[0] += pFrame->linesize[0] * (pCodecCtx->height - 1);
                pFrame->linesize[0] *= -1;
                pFrame->data[1] += pFrame->linesize[1] * (pCodecCtx->height / 2 - 1);
                pFrame->linesize[1] *= -1;
                pFrame->data[2] += pFrame->linesize[2] * (pCodecCtx->height / 2 - 1);
                pFrame->linesize[2] *= -1;
                
                sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                
                //if (VIDEO_OP_TYPE_SAVE_BMP == m_eOpType)
                {
                    //SaveAsBMP(pFrameRGB, pCodecCtx->width, pCodecCtx->height, idx);
                }
                //else if (VIDEO_OP_TYPE_PLAY == m_eOpType)
                {
                    CBaseVideoPlay* pObj = g_pVideoPlayMgr->GetBaseVideoPlay();
                    if (NULL != pObj)
                    {
                        pObj->SetSharedData(pFrameRGB->data[0], pCodecCtx->width, pCodecCtx->height, (0==idx), nPicSize, strTexture);
                    }
                }
                
                usleep(g_nDecodeSleepTime);
                
                ++idx;
            }
            
            av_packet_unref(&packet);
        }
    }
    
    // free
    sws_freeContext(pSwsCtx);
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFmtContext);
    
    CBaseVideoPlay* pObj = g_pVideoPlayMgr->GetBaseVideoPlay();
    if (NULL != pObj)
    {
        pObj->DecreaceThreadCount();
    }
}


void CBaseVideoPlay::Run(const std::string& strTexture)
{
    if (!CheckValid())
    {
        return;
    }
    
    if (!m_bInit)
    {
        av_register_all();
        avformat_network_init();
    }
    
    std::thread t(Play, m_strVideoPath.c_str(), strTexture);
    t.detach();
    
    ++m_nThreadCnt;
    if (!m_bInit)
    {
        m_bInit = true;
        bool bHasData = true;
        while (1)
        {
            bHasData = SendData();
            if (m_nThreadCnt <= 0 && !bHasData)
            {
                break;
            }
        }
    }
}

void CBaseVideoPlay::SetSavePath(const std::string& strSavePath)
{
    m_strSavePath = strSavePath;
}

void CBaseVideoPlay::SetVideoCbFunc(FuncVideoCB pFuncVideoCB)
{
    if (NULL == pFuncVideoCB)
    {
        printf("[CBaseVideoPlay::SetVideoCbFunc] warning: set call back function NULL.\n");
    }
    
    m_pFuncVideoCB = pFuncVideoCB;
}

bool CBaseVideoPlay::CheckValid()
{
    if (m_strVideoPath.empty())
    {
        printf("[CBaseVideoPlay::CheckValid] warning: not set video file path, default is current.\n");
        return false;
    }
    
    if (VIDEO_OP_TYPE_SAVE_BMP == m_eOpType)
    {
        if (m_strSavePath.empty())
        {
            printf("[CBaseVideoPlay::CheckValid] warning: not set save picture file path, default is current.\n");
        }
    }
    else if (VIDEO_OP_TYPE_PLAY == m_eOpType)
    {
        if (NULL == m_pFuncVideoCB)
        {
            printf("[CBaseVideoPlay::CheckValid] error: not set call back function, please set before run.\n");
            return false;
        }
    }
    else if (m_nThreadCnt >= THREAD_MAX_COUNT)
    {
        printf("[CBaseVideoPlay::CheckValid] thread has use up.\n");
        return false;
    }
    
    return true;
}

// 保存BMP文件的函数
void CBaseVideoPlay::SaveAsBMP(AVFrame *pFrameRGB, int width, int height, int index)
{
    char *filename = new char[255];
    //char filename[255];
    //文件存放路径，根据自己的修改
    if (m_strSavePath.empty())
    {
        sprintf(filename, "./bmp/%s_%d.bmp", "filename", index);
    }
    else
    {
        sprintf(filename, "%s_%d.bmp", m_strSavePath.c_str(), index);
    }
    stbi_write_bmp(filename, width, height, 3, pFrameRGB->data[0]);
}

void CBaseVideoPlay::SetSharedData(void *pData, int width, int height, bool bFirstCB, int nSize, const std::string& strTexture)
{
    if (NULL == pData)
    {
        return;
    }
    
    m_mutex.lock();
    //printf("[CBaseVideoPlay::SetSharedData] ~~~~~~ set ~$$$$$$$$$$$$$$$$$$$$$############# \n");
    m_sSharedData[m_nBack].pData = malloc(nSize);
    memcpy(m_sSharedData[m_nBack].pData, pData, nSize);
    m_sSharedData[m_nBack].nWidth = width;
    m_sSharedData[m_nBack].nHeight = height;
    m_sSharedData[m_nBack].bFirstCB = bFirstCB;
    m_sSharedData[m_nBack].strTexture = strTexture;
    
    ++m_nBack;
    if (m_nBack >= g_nQueueDeep)
    {
        m_nBack = 0;
    }
    
    m_mutex.unlock();
}

void CBaseVideoPlay::DecreaceThreadCount()
{
    --m_nThreadCnt;
    if (m_nThreadCnt < 0)
    {
        m_nThreadCnt = 0;
    }
}

bool CBaseVideoPlay::SendData()
{
    if (NULL != m_pFuncVideoCB)
    {
        void* pData;
        int width, height;
        bool bFirstCB;
        bool bHasData = false;
        std::string strTexture;
        m_mutex.lock();
        if (m_nBack != m_nPre)
        {
            //printf("[CBaseVideoPlay::SendData()] ~~~~~~ send ~############# \n");
            bHasData = true;
            
            pData = m_sSharedData[m_nPre].pData;
            width = m_sSharedData[m_nPre].nWidth;
            height = m_sSharedData[m_nPre].nHeight;
            bFirstCB = m_sSharedData[m_nPre].bFirstCB;
            strTexture = m_sSharedData[m_nPre].strTexture;
            
            ++m_nPre;
            if (m_nPre >= g_nQueueDeep)
            {
                m_nPre = 0;
            }
        }
        m_mutex.unlock();
        
        if (bHasData)
        {
            (*m_pFuncVideoCB)(pData, width, height, bFirstCB, strTexture);
            usleep(g_nSendDataSleepTime);
            return true;
        }
    }
    
    return false;
}

