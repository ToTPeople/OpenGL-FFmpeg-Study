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
#include <pthread.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

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
#include "CThreadProductConsumerHelper.hpp"

namespace
{
    const int g_nSendDataSleepTime = 30 * 1000;// 单位：微秒 发数据刷新界面频率
    const int g_nMaxImageWidth = 2048;
    const int g_nMaxImageHeight = 1536;
}

CBaseVideoPlay::CBaseVideoPlay(int eOpType, const std::string& strVideoPath)
: m_pFuncVideoCB(NULL)
, m_eOpType(eOpType)
, m_strVideoPath(strVideoPath)
, m_strSavePath("")
, m_bInit(false)
, m_nThreadCnt(0)
, m_nCurThreadCnt(0)
, idx(0)
, m_shmid(-1)
, m_pSharedData(NULL)
{
}

CBaseVideoPlay::~CBaseVideoPlay()
{
}

static void AddData2Queue(CBaseVideoPlay::SharedData_s *pShareData, uint8_t* pRGBData, AVCodecContext* pCodecCtx, int nRGBSize, int thread_idx, int thread_max, int idx)
{
    if (NULL == pShareData || NULL == pRGBData || NULL == pCodecCtx || nRGBSize <= 0)
    {
        printf("[AddData2Queue] warning: param invalid. pShareData[%p], pRGBData[%p], pCodecCtx[%p], nRGBSize[%d]\n"
               , pShareData, pRGBData, pCodecCtx, nRGBSize);
        return;
    }
    g_pThreadProductConsumerHelper->BufferLock();
    int back = pShareData->back[thread_idx];
    
    if (back+1 == pShareData->pre[thread_idx]
        || (back+1 == thread_max && 0 == pShareData->pre[thread_idx]))
    {
        printf("[CBaseVideoPlay::Play] info: queue is full, waiting...\n");
        g_pThreadProductConsumerHelper->BufferFullWait();
    }
    
    pShareData->pData[thread_idx][back] = malloc(nRGBSize);
    if (NULL == pShareData->pData[thread_idx][back])
    {
        printf("[CBaseVideoPlay::Play] warning: malloc memory failed.\n");
        g_pThreadProductConsumerHelper->BufferUnlock();
        return;
    }
    memcpy(pShareData->pData[thread_idx][back], pRGBData, nRGBSize);
    pShareData->nWidth[thread_idx] = pCodecCtx->width;
    pShareData->nHeight[thread_idx] = pCodecCtx->height;
    pShareData->bFirstCB[thread_idx][back] = (0 == idx);
    
    ++pShareData->back[thread_idx];
    if (pShareData->back[thread_idx] >= thread_max)
    {
        pShareData->back[thread_idx] = 0;
    }
    // 解析这个数据前，队列是否为空，为空则发消息队列不为空消息出来
    if (pShareData->pre[thread_idx] == back)
    {
        printf("[CBaseVideoPlay::Play] info: queue not empty signal.\n");
        g_pThreadProductConsumerHelper->BufferNotEmptySignal();
    }
    g_pThreadProductConsumerHelper->BufferUnlock();
}

void CBaseVideoPlay::Play(const char* filename, int* cur_thread_cnt, int thread_idx)
{
    unsigned int idx, nVideoStreamIdx = -1;
    AVFormatContext* pFmtContext;       // 格式内容
    AVCodecContext* pCodecCtx;          // 解码内容
    AVCodec*        pCodec;             // 解码器
    
    if (thread_idx >= THREAD_MAX_COUNT)
    {
        printf("[CBaseVideoPlay::Play] error: thread max count is 5, the index is %d.\n", thread_idx);
        return;
    }
    
    if (NULL == filename || NULL == cur_thread_cnt)
    {
        printf("[CBaseVideoPlay::Play] error: param invalid, filename[%p], cur_thread_cnt[%p].\n", filename, cur_thread_cnt);
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
        if (AVMEDIA_TYPE_VIDEO == pFmtContext->streams[idx]->codec->codec_type)
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
    
    // 共享内存
    int shmid;
    shmid = open(kszSharedFilePath, O_RDWR, 0);
    if (-1 == shmid)
    {
        printf("[CBaseVideoPlay::Play] error: shm_open failed.\n");
        exit(1);
    }
    struct stat stat_buf;
    if (-1 == fstat(shmid, &stat_buf))
    {
        printf("[CBaseVideoPlay::Play] error: fstat failed.\n");
        exit(1);
    }
    SharedData_s *pShareData;
    pShareData = (SharedData_s*)mmap(NULL, stat_buf.st_size, PROT_WRITE, MAP_SHARED, shmid, 0);
    if (MAP_FAILED == pShareData)
    {
        printf("[CBaseVideoPlay::Play] error: mmap failed.\n");
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
                // 反转图像 ，否则生成的图像是上下调到的
                pFrame->data[0] += pFrame->linesize[0] * (pCodecCtx->height - 1);
                pFrame->linesize[0] *= -1;
                pFrame->data[1] += pFrame->linesize[1] * (pCodecCtx->height / 2 - 1);
                pFrame->linesize[1] *= -1;
                pFrame->data[2] += pFrame->linesize[2] * (pCodecCtx->height / 2 - 1);
                pFrame->linesize[2] *= -1;
                
                sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                // 设置数据到共享内存中
                AddData2Queue(pShareData, pFrameRGB->data[0], pCodecCtx, nPicSize, thread_idx, THREAD_MAX_COUNT, idx);
                
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
    // 关闭共享内存文件标识符
    close(shmid);
    // 更新当前活动线程个数
    *cur_thread_cnt -= 1;
}

void CBaseVideoPlay::Run(const std::string& strTexture)
{
    if (!CheckValid())
    {
        return;
    }
    
    if (m_nThreadCnt >= THREAD_MAX_COUNT)
    {
        printf("[CBaseVideoPlay::Run] warning: max thread is %d, current is %d.\n", THREAD_MAX_COUNT, m_nThreadCnt);
        return;
    }
    
    if (!m_bInit)
    {
        av_register_all();
        avformat_network_init();
        // 打开共享内存
        m_shmid = open(kszSharedFilePath, O_CREAT | O_RDWR, 0666);
        if (-1 == m_shmid)
        {
            printf("[CBaseVideoPlay::Run] error: shm_open failed.\n");
            exit(1);
        }
        __int64_t nSize = g_nMaxImageWidth * g_nMaxImageHeight * SHARED_DATA_QUEUE_COUNT * THREAD_MAX_COUNT * sizeof(uint8_t); // ## 5组，1组10张图片数据，每张大小2048*1536
        if (-1 == ftruncate(m_shmid, nSize + sizeof(SharedData_s)))
        {
            printf("[CBaseVideoPlay::Run] error: ftruncate failed, errno[%d].\n", errno);
            exit(1);
        }
        if (-1 == fstat(m_shmid, &m_statBuf))
        {
            printf("[CBaseVideoPlay::Run] error: fstat fstat.\n");
            exit(1);
        }
        printf("[CBaseVideoPlay::Run] info: size=%ld mode=%o.\n", m_statBuf.st_size, m_statBuf.st_mode & 0777);
        m_pSharedData = (SharedData_s*)mmap(NULL, m_statBuf.st_size, PROT_WRITE, MAP_SHARED, m_shmid, 0);
        if (MAP_FAILED == m_pSharedData)
        {
            printf("[CBaseVideoPlay::Run] error: mmap failed.\n");
            exit(1);
        }
        memset(m_pSharedData, 0, m_statBuf.st_size);
    }
    
    m_strTexture[m_nThreadCnt] = strTexture;
    ++m_nCurThreadCnt;
    ++m_nThreadCnt;
    
    std::thread t(Play, m_strVideoPath.c_str(), &m_nCurThreadCnt, m_nThreadCnt-1);
    t.detach();
    
    if (!m_bInit)
    {
        m_bInit = true;
        bool bHasData = true;
        while (1)
        {
            bHasData = SendData();
            if (m_nCurThreadCnt <= 0 && !bHasData)
            {
                break;
            }
        }
        
        if (-1 != m_shmid)
        {
            close(m_shmid);
            unlink(kszSharedFilePath);
        }
    }
}

void CBaseVideoPlay::Record(uint8_t *pImgData, int nWidth, int nHeight, bool bEnd)
{
    if (!m_bInit)
    {
        av_register_all();
        avformat_network_init();
    }
    
    std::thread t(CompressRGB, pImgData, nWidth, nHeight, bEnd, (0 == idx), idx);
    
    
    t.detach();
    
    idx = bEnd ? 0 : (idx + 1);
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

int CBaseVideoPlay::GetCreateThreadCnt()
{
    return m_nThreadCnt;
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

bool CBaseVideoPlay::SendData()
{
    if (NULL != m_pFuncVideoCB)
    {
        void* pData[THREAD_MAX_COUNT];
        int width[THREAD_MAX_COUNT], height[THREAD_MAX_COUNT];
        bool bFirstCB[THREAD_MAX_COUNT];
        bool bHasData = false;
        bool bPreFull = false;
        std::string strTexture[THREAD_MAX_COUNT];
        for (int i = 0; i < THREAD_MAX_COUNT; ++i)
        {
            pData[i] = NULL;
        }
        
        g_pThreadProductConsumerHelper->BufferLock();
        
        for (int i = 0; i < m_nThreadCnt; ++i)
        {
            if (m_pSharedData->pre[i] != m_pSharedData->back[i])
            {
                bHasData = true;
                break;
            }
        }
        if (!bHasData)
        {
            if (m_nCurThreadCnt <= 0)
            {
                return false;
            }
            printf("[CBaseVideoPlay::SendData] info: queue is empty, waiting.\n");
            g_pThreadProductConsumerHelper->BufferEmptyWait();
            bHasData = true;
        }
        
        for (int i = 0; i < m_nThreadCnt; ++i)
        {
            int pre = m_pSharedData->pre[i];
            
            pData[i] = m_pSharedData->pData[i][pre];
            m_pSharedData->pData[i][pre] = NULL;
            width[i] = m_pSharedData->nWidth[i];
            height[i] = m_pSharedData->nHeight[i];
            bFirstCB[i] = m_pSharedData->bFirstCB[i][pre];
            strTexture[i] = m_strTexture[i];
            
            ++m_pSharedData->pre[i];
            if (m_pSharedData->pre[i] >= THREAD_MAX_COUNT)
            {
                m_pSharedData->pre[i] = 0;
            }
            
            // 判断取数据之前是否队列满了
            if (m_pSharedData->back[i] + 1 == pre
                || (0 == pre && m_pSharedData->back[i]+1 == THREAD_MAX_COUNT))
            {
                bPreFull = true;
            }
        }
        
        if (bPreFull)
        {
            printf("[CBaseVideoPlay::SendData] info: queue not full signal.\n");
            g_pThreadProductConsumerHelper->BufferNotFullSignal();
        }
        
        g_pThreadProductConsumerHelper->BufferUnlock();
        
        bool bRealData = false;
        for (int i = 0; i < m_nThreadCnt; ++i)
        {
            if (NULL != pData[i])
            {
                bRealData = true;
                break;
            }
        }
        if (!bRealData)
        {
            usleep(g_nSendDataSleepTime);
            return false;
        }
        
        if (bHasData)
        {
            (*m_pFuncVideoCB)(pData, width, height, bFirstCB, strTexture);
            for (int i=0; i<m_nThreadCnt; ++i)
            {
                if (NULL != pData[i])
                {
                    free(pData[i]);
                }
            }
            usleep(g_nSendDataSleepTime);
            return true;
        }
    }
    
    return false;
}

static int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index)
{
    int ret, got_frame;
    AVPacket enc_pkt;
    
    if (NULL == fmt_ctx->streams || NULL == *(fmt_ctx->streams)
        || NULL == fmt_ctx->streams[stream_index]
        || !(fmt_ctx->streams[stream_index]->codec->codec->capabilities & CODEC_CAP_DELAY))
    {
        return 0;
    }
    
    while (1)
    {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        
        ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt, NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
        {
            break;
        }
        if (!got_frame)
        {
            ret = 0;
            break;
        }
        printf("###### Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
        
        ret = av_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
        {
            break;
        }
    }
    
    return ret;
}

void CBaseVideoPlay::CompressRGB(uint8_t *pImgData, int nWidth, int nHeight, bool bEnd, bool bFirst, int idx)
{
    bool is_error = false;
    
    do
    {
        if (NULL == pImgData)
        {
            is_error = true;
            break;
        }
        const char* out_file = "./video/screenst.h264";
        
        // 用作之后写入视频帧并编码成 h264，贯穿整个工程当中
        AVFormatContext* pFormatCtx;
        pFormatCtx = avformat_alloc_context();
        if (NULL == pFormatCtx)
        {
            printf("[CBaseVideoPlay::CompressRGB] error: avformat_alloc_context failed.\n");
            is_error = true;
            break;
        }
        
        // 通过这个函数可以获取输出文件的编码格式, 那么这里我们的 fmt 为 h264 格式(AVOutputFormat *)
        AVOutputFormat* fmt;
        avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
        fmt = pFormatCtx->oformat;
        
        // 打开文件的缓冲区输入输出，flags 标识为  AVIO_FLAG_READ_WRITE ，可读写
        if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0)
        {
            printf("[CBaseVideoPlay::CompressRGB] error: avio_open failed.\n");
            is_error = true;
            break;
        }
        
        AVStream *video_st;
        // 通过媒体文件控制者获取输出文件的流媒体数据，这里 AVCodec * 写 0 ， 默认会为我们计算出合适的编码格式
        video_st = avformat_new_stream(pFormatCtx, 0);
        if (NULL == video_st)
        {
            printf("[CBaseVideoPlay::CompressRGB] error: avformat_new_stream failed.\n");
            is_error = true;
            break;
        }
        // 设置 25 帧每秒 ，也就是 fps 为 25
        video_st->time_base.num = 1;
        video_st->time_base.den = 25;
        
        // 用户存储编码所需的参数格式等等
        AVCodecContext* pCodecCtx;
        // 从媒体流中获取到编码结构体，他们是一一对应的关系，一个 AVStream 对应一个  AVCodecContext
        pCodecCtx = video_st->codec;
        // 设置编码器的 id，每一个编码器都对应着自己的 id，例如 h264 的编码 id 就是 AV_CODEC_ID_H264
        pCodecCtx->codec_id = fmt->video_codec;
        // 设置编码类型为 视频编码
        pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
        // 设置像素格式为 yuv 格式
        pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        // 设置视频的宽高
        pCodecCtx->width = nWidth;
        pCodecCtx->height = nHeight;
        
        // 设置比特率，每秒传输多少比特数 bit，比特率越高，传送速度越快，也可以称作码率，
        // 视频中的比特是指由模拟信号转换为数字信号后，单位时间内的二进制数据量。
        pCodecCtx->bit_rate = 4000;//400000;
        // 设置图像组层的大小。
        // 图像组层是在 MPEG 编码器中存在的概念，图像组包 若干幅图像, 组头包 起始码、GOP 标志等,如视频磁带记录器时间、控制码、B 帧处理码等;
        pCodecCtx->gop_size = 250;
        // 设置 25 帧每秒 ，也就是 fps 为 25
        pCodecCtx->time_base.num = 1;
        pCodecCtx->time_base.den = 25;
        //设置 H264 中相关的参数
        pCodecCtx->qmin = 10;
        pCodecCtx->qmax = 25;
        
        // 设置 B 帧最大的数量，B帧为视频图片空间的前后预测帧， B 帧相对于 I、P 帧来说，压缩率比较大，也就是说相同码率的情况下，
        // 越多 B 帧的视频，越清晰，现在很多打视频网站的高清视频，就是采用多编码 B 帧去提高清晰度，
        // 但同时对于编解码的复杂度比较高，比较消耗性能与时间
        pCodecCtx->max_b_frames = 3;
        // 可选设置
        AVDictionary* param = 0;
        if (AV_CODEC_ID_H264 == pCodecCtx->codec_id)
        {
            // 通过--preset的参数调节编码速度和质量的平衡。
            //av_dict_set(&param, "preset", "slow", 0);
            av_dict_set(&param, "preset", "fast", 0);
            // 通过--tune的参数值指定片子的类型，是和视觉优化的参数，或有特别的情况。
            // zerolatency: 零延迟，用在需要非常低的延迟的情况下，比如电视电话会议的编码
            av_dict_set(&param, "tune", "zerolatency", 0);
        }
        
        
        avcodec_parameters_from_context(video_st->codecpar, pCodecCtx);
        AVCodec* pCodec;
        pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
        if (NULL == pCodec)
        {
            printf("[CBaseVideoPlay::CompressRGB] error: avcodec_find_encoder failed.\n");
            is_error = true;
            break;
        }
        int ret;
        // 打开编码器，并设置参数 param
        if ((ret = avcodec_open2(pCodecCtx, pCodec, &param)) < 0)
        {
            printf("[CBaseVideoPlay::CompressRGB] error: avcodec_open2 failed.[%d]\n", ret);
            is_error = true;
            break;
        }
        // 数据信息
        av_dump_format(pFormatCtx, 0, out_file, 1);
   
        // 通过 codec_id 找到对应的编码器

        
        // 目标frame
        AVFrame* pFrame;
        pFrame = av_frame_alloc();
        if (NULL == pFrame)
        {
            printf("[CBaseVideoPlay::CompressRGB] error: av_frame_alloc failed.\n");
            is_error = true;
            break;
        }
        // 通过像素格式(这里为 YUV)获取图片的真实大小，例如将 480 * 272 转换成 int 类型
        int nPicSize = av_image_get_buffer_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
        // 将 picture_size 转换成字节数据，byte
        unsigned char* pPicBuf = (uint8_t*)av_malloc(nPicSize);
        // 设置原始数据 AVFrame 的每一个frame 的图片大小，AVFrame 这里存储着 YUV 非压缩数据
        av_image_fill_arrays(pFrame->data, pFrame->linesize, pPicBuf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
        
        if (bFirst)
        {
            // 编写 h264 封装格式的文件头部，基本上每种编码都有着自己的格式的头部，想看具体实现的同学可以看看 h264 的具体实现
            avformat_write_header(pFormatCtx, NULL);
            bEnd = false;
        }
        
        AVPacket packet;
        if (0 != av_new_packet(&packet, nPicSize))
        {
            printf("[CBaseVideoPlay::CompressRGB] error: av_new_packet failed.\n");
            is_error = true;
            break;
        }
        
        if (!bEnd)
        {
            // 源frame
            AVFrame *pSrcFrame = av_frame_alloc();
            if (NULL == pSrcFrame)
            {
                printf("[CBaseVideoPlay::CompressRGB] error: av_frame_alloc failed.\n");
                is_error = true;
                break;
            }
            // 通过像素格式(这里为 YUV)获取图片的真实大小，例如将 480 * 272 转换成 int 类型
            int nRGBSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
            // 将 picture_size 转换成字节数据，byte
            unsigned char* pRGBBuf = (uint8_t*)av_malloc(nRGBSize);
            pRGBBuf = pImgData;
            // 设置原始数据 AVFrame 的每一个frame 的图片大小，AVFrame 这里存储着 YUV 非压缩数据
            av_image_fill_arrays(pSrcFrame->data, pSrcFrame->linesize, pRGBBuf, AV_PIX_FMT_RGB24, nWidth, nHeight, 1);
            
            SwsContext* pSwsCtx = sws_getContext(nWidth, nHeight, AV_PIX_FMT_RGB24, nWidth, nHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
            if (NULL == pSwsCtx)
            {
                is_error = true;
                break;
            }
            // 翻转RGB图像
            pSrcFrame->data[0]  += pSrcFrame->linesize[0] * (nHeight - 1);
            pSrcFrame->linesize[0] *= -1;
            pSrcFrame->data[1]  += pSrcFrame->linesize[1] * (nHeight / 2 - 1);
            pSrcFrame->linesize[1] *= -1;
            pSrcFrame->data[2]  += pSrcFrame->linesize[2] * (nHeight / 2 - 1);
            pSrcFrame->linesize[2] *= -1;
            
            sws_scale(pSwsCtx, pSrcFrame->data, pSrcFrame->linesize, 0, nHeight, pFrame->data, pFrame->linesize);
            // PTS
            // 设置这一帧的显示时间
            pFrame->pts = idx * (video_st->time_base.den) / ((video_st->time_base.num) * 25);
            int got_pic = 0;

            // 利用编码器进行编码，将 pFrame 编码后的数据传入 pkt 中
            if (avcodec_encode_video2(pCodecCtx, &packet, pFrame, &got_pic) < 0)
            {
                printf("[CBaseVideoPlay::CompressRGB] error: avcodec_encode_video2 failed.\n");
                is_error = true;
                break;
            }
            if (got_pic != 1)
            {
                break;
            }
            
            packet.stream_index = video_st->index;
            av_write_frame(pFormatCtx, &packet);
            
            av_packet_unref(&packet);
        }
        else
        {
            if (flush_encoder(pFormatCtx, 0) < 0)
            {
                printf("[CBaseVideoPlay::CompressRGB] error: flush_encoder failed.\n");
                is_error = true;
                break;
            }
            
            // 写入数据流尾部到输出文件当中，并释放文件的私有数据
            av_write_trailer(pFormatCtx);
            
            if (NULL != video_st)
            {
                // 关闭编码器
                avcodec_close(video_st->codec);
                // 释放 AVFrame
                av_frame_free(&pFrame);
                // 释放图片 buf，就是 free() 函数，硬要改名字，当然这是跟适应编译环境有关系的
                av_free(pPicBuf);
            }
            
            // 关闭输入数据的缓存
            avio_close(pFormatCtx->pb);
            // 释放 AVFromatContext 结构体
            avformat_free_context(pFormatCtx);
        }
    } while(0);
    
    if (is_error)
    {
        printf("^^^^^^$$$$$$$$#####@@@##@#@#@##@#@#@##@#@#@#@$@@#@$#$##$@$#@$#@$#$#@$@$#@$##$@$#@\n\n\n");
        exit(1);
    }
}
/*
void CBaseVideoPlay::CompressRGB(AVCodecContext* pCodecCtx, uint8_t *pImgData, int nWidth, int nHeight)
{
    bool is_error = false;
    
    do
    {
        const char* out_file = "./video/screenshots.h264";
        
        // 目标frame
        int nPicSize = 0;
        AVFrame *pFrame;
        {
            pFrame = av_frame_alloc();
            if (NULL == pFrame)
            {
                printf("[CBaseVideoPlay::CompressRGB] error: av_frame_alloc failed.\n");
                is_error = true;
                break;
            }
            
            // 通过像素格式(这里为 YUV)获取图片的真实大小，例如将 480 * 272 转换成 int 类型
            nPicSize = av_image_get_buffer_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
            // 将 picture_size 转换成字节数据，byte
            uint8_t* pPicBuf = (uint8_t*)av_malloc(nPicSize);
            // 设置原始数据 AVFrame 的每一个frame 的图片大小，AVFrame 这里存储着 YUV 非压缩数据
            av_image_fill_arrays(pFrame->data, pFrame->linesize, pPicBuf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
        }
        
        AVPacket packet;
        if (0 != av_new_packet(&packet, nPicSize))
        {
            printf("[CBaseVideoPlay::CompressRGB] error: av_new_packet failed.\n");
            is_error = true;
            break;
        }
        
        // 源frame
        AVFrame *pSrcFrame = av_frame_alloc();
        if (NULL == pSrcFrame)
        {
            printf("[CBaseVideoPlay::CompressRGB] error: av_frame_alloc failed.\n");
            is_error = true;
            break;
        }
        
        // 通过像素格式(这里为 YUV)获取图片的真实大小，例如将 480 * 272 转换成 int 类型
        int nRGBSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
        // 将 picture_size 转换成字节数据，byte
        unsigned char* pRGBBuf = (uint8_t*)av_malloc(nRGBSize);
        pRGBBuf = pImgData;
        // 设置原始数据 AVFrame 的每一个frame 的图片大小，AVFrame 这里存储着 YUV 非压缩数据
        av_image_fill_arrays(pSrcFrame->data, pSrcFrame->linesize, pRGBBuf, AV_PIX_FMT_RGB24, nWidth, nHeight, 1);
        
        SwsContext* pSwsCtx = sws_getContext(nWidth, nHeight, AV_PIX_FMT_RGB24, nWidth, nHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
        
        // 翻转RGB图像
        pSrcFrame->data[0]  += pSrcFrame->linesize[0] * (nHeight - 1);
        pSrcFrame->linesize[0] *= -1;
        pSrcFrame->data[1]  += pSrcFrame->linesize[1] * (nHeight / 2 - 1);
        pSrcFrame->linesize[1] *= -1;
        pSrcFrame->data[2]  += pSrcFrame->linesize[2] * (nHeight / 2 - 1);
        pSrcFrame->linesize[2] *= -1;
        
        sws_scale(pSwsCtx, pSrcFrame->data, pSrcFrame->linesize, 0, nHeight, pFrame->data, pFrame->linesize);
        
        {
            // PTS
            // 设置这一帧的显示时间
            //pFrame->pts = idx * (video_st->time_base.den) / ((video_st->time_base.num) * 25);
            pFrame->pts = idx * (pCodecCtx->time_base.den) / (pCodecCtx->time_base.num * 25);
            int got_pic = 0;
            
            // 利用编码器进行编码，将 pFrame 编码后的数据传入 pkt 中
            if (avcodec_encode_video2(pCodecCtx, &packet, pFrame, &got_pic) < 0)
            {
                printf("[CBaseVideoPlay::CompressRGB] error: avcodec_encode_video2 failed.\n");
                is_error = true;
                break;
            }
            if (got_pic != 1)
            {
                break;
            }
            
            // 编码成功后写入 AVPacket 到 输入输出数据操作着 pFormatCtx 中，当然，记得释放内存
            printf("------- Succeed to encode frame: %5d\tsize:%5d\n", idx, packet.size);
            ++idx;
            
            packet.stream_index = video_st->index;
            av_write_frame(pFormatCtx, &packet);
            
            av_packet_unref(&packet);
        }
    } while(0);
    
    if (is_error)
    {
        exit(1);
    }
}
 */
