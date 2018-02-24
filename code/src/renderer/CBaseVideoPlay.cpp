//
//  CBaseVideoPlay.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/30.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "CBaseVideoPlay.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
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
#include "CPthreadMuteHelper.hpp"
#include "CThreadProductConsumerHelper.hpp"

namespace
{
    const int g_nSendDataSleepTime = 30 * 1000;// 单位：微秒 发数据刷新界面频率
    const int g_nTimerDataSleepTime = 10 * 1000;// 单位：微秒 发数据刷新界面频率
    const int g_nMaxImageWidth = 2048;
    const int g_nMaxImageHeight = 1536;
    
//#define DECODE_LOG                          // 解码队列打印调试信息
//#define DECODE_TIMESTAMP_LOG                // 解码时间戳调试
//#define ENCODE_QUEUE_LOG                    // 压缩队列打印调试信息
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
, m_compress_pts(0)
, m_shmid(-1)
, m_compress_shmid(-1)
, m_pSharedData(NULL)
, m_pCompressSharedData(NULL)
{
}

CBaseVideoPlay::~CBaseVideoPlay()
{
}

// 返回单位ms
static long long getCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long ms = tv.tv_sec;
    return ms * 1000 + tv.tv_usec / 1000;
}

static void AddData2Queue(CBaseVideoPlay::SharedData_s *pShareData, uint8_t* pRGBData, AVCodecContext* pCodecCtx, int nRGBSize, int thread_idx, int queue_max, int idx, long long cur_frame_pts)
{
    if (NULL == pShareData || NULL == pRGBData || NULL == pCodecCtx || nRGBSize <= 0) {
        printf("[AddData2Queue] warning: param invalid. pShareData[%p], pRGBData[%p], pCodecCtx[%p], nRGBSize[%d]\n"
               , pShareData, pRGBData, pCodecCtx, nRGBSize);
        return;
    }
    
    static int ic = 0;
    
//    g_pThreadProductConsumerHelper->BufferLock();
    int back = pShareData->back[thread_idx];
    
    while (1) {
        if ( !((back+1)%queue_max == pShareData->pre[thread_idx]) ) {
    #ifdef DECODE_LOG
            printf("[CBaseVideoPlay::Play] info: thread_idx[%d] queue is full, waiting...\n", thread_idx);
    #endif
            break;
//            g_pThreadProductConsumerHelper->BufferFullWait();
        }
    }
    //
//    g_pThreadProductConsumerHelper->BufferUnlock();
    //
    
    pShareData->pData[thread_idx][back] = malloc(nRGBSize);
    if (NULL == pShareData->pData[thread_idx][back]) {
        printf("[CBaseVideoPlay::Play] warning: malloc memory failed.\n");
//        g_pThreadProductConsumerHelper->BufferUnlock();
        return;
    }
    memcpy(pShareData->pData[thread_idx][back], pRGBData, nRGBSize);
    pShareData->nWidth[thread_idx] = pCodecCtx->width;
    pShareData->nHeight[thread_idx] = pCodecCtx->height;
    pShareData->pts[thread_idx][back] = cur_frame_pts;
    if (0 == idx) {
        pShareData->start_pts[thread_idx] = getCurrentTime();
    }
    
    pShareData->back[thread_idx] = (back+1) % queue_max;
    // 解析这个数据前，队列是否为空，为空则发消息队列不为空消息出来
    if (pShareData->pre[thread_idx] == back) {
#ifdef DECODE_LOG
        printf("[CBaseVideoPlay::Play] info: queue not empty signal.\n");
#endif
//        g_pThreadProductConsumerHelper->BufferNotEmptySignal();
    }
//    g_pThreadProductConsumerHelper->BufferUnlock();
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
    AVRational tmp_time_base;
    tmp_time_base.num = 1;
    tmp_time_base.den = 1000;
    
    long long cur_frame_pts, cur_frame_duration;
    
    while (av_read_frame(pFmtContext, &packet) >= 0) {
        if (nVideoStreamIdx == packet.stream_index) {
            int nResult = avcodec_send_packet(pCodecCtx, &packet);
            nResult += avcodec_receive_frame(pCodecCtx, pFrame);
            if (0 == nResult) {
                // 播放时间转换 demux -> decode -> raw
                cur_frame_pts = av_rescale_q_rnd(pFrame->pts, pFmtContext->streams[nVideoStreamIdx]->time_base, pCodecCtx->time_base, AV_ROUND_INF);
                cur_frame_pts = av_rescale_q_rnd(cur_frame_pts, pCodecCtx->time_base, tmp_time_base, AV_ROUND_INF);
                
#ifdef DECODE_TIMESTAMP_LOG
                printf("$$$$$$$$$ idx[%d], framePTS[%ld], pkt_duration[%lld], cur_frame_pts[%lld]\n", idx, pFrame->pts, pFrame->pkt_duration, cur_frame_pts);
#endif
                // 反转图像 ，否则生成的图像是上下调到的
                pFrame->data[0] += pFrame->linesize[0] * (pCodecCtx->height - 1);
                pFrame->linesize[0] *= -1;
                pFrame->data[1] += pFrame->linesize[1] * (pCodecCtx->height / 2 - 1);
                pFrame->linesize[1] *= -1;
                pFrame->data[2] += pFrame->linesize[2] * (pCodecCtx->height / 2 - 1);
                pFrame->linesize[2] *= -1;
                
                sws_scale(pSwsCtx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                // 设置数据到共享内存中
#if 1
                AddData2Queue(pShareData, pFrameRGB->data[0], pCodecCtx, nPicSize, thread_idx, SHARED_DATA_QUEUE_COUNT, idx, cur_frame_pts);
#else
                
                char filename[255];
                //文件存放路径，根据自己的修改
                sprintf(filename, "./bmp/%s_%d.bmp", "filename", idx);
                
                stbi_write_bmp(filename, pCodecCtx->width, pCodecCtx->height, 3, pFrameRGB->data[0]);
#endif
                
                ++idx;
//                    break;
            } // end if(0 == nResult)
            
            av_packet_unref(&packet);
        }
    } // end while (av_read_frame(pFmtContext, &packet) >= 0)
    
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
    if (!CheckValid()) {
        return;
    }
    
    if (m_nThreadCnt >= THREAD_MAX_COUNT) {
        printf("[CBaseVideoPlay::Run] warning: max thread is %d, current is %d.\n", THREAD_MAX_COUNT, m_nThreadCnt);
        return;
    }
    
    if (!m_bInit) {
        av_register_all();
        avformat_network_init();
        // 打开队列数据共享内存
        m_shmid = open(kszSharedFilePath, O_CREAT | O_RDWR, 0666);
        if (-1 == m_shmid) {
            printf("[CBaseVideoPlay::Run] error: shm_open failed.\n");
            exit(1);
        }
        __int64_t nSize = g_nMaxImageWidth * g_nMaxImageHeight * SHARED_DATA_QUEUE_COUNT * THREAD_MAX_COUNT * sizeof(uint8_t); // ## 5组，1组10张图片数据，每张大小2048*1536
        if (-1 == ftruncate(m_shmid, nSize + sizeof(SharedData_s))) {
            printf("[CBaseVideoPlay::Run] error: ftruncate failed, errno[%d].\n", errno);
            exit(1);
        }
        if (-1 == fstat(m_shmid, &m_statBuf)) {
            printf("[CBaseVideoPlay::Run] error: fstat fstat.\n");
            exit(1);
        }
        printf("[CBaseVideoPlay::Run] info: size=%ld mode=%o.\n", m_statBuf.st_size, m_statBuf.st_mode & 0777);
        m_pSharedData = (SharedData_s*)mmap(NULL, m_statBuf.st_size, PROT_WRITE, MAP_SHARED, m_shmid, 0);
        if (MAP_FAILED == m_pSharedData) {
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
    
    m_thread_start_pts[m_nThreadCnt - 1] = getCurrentTime();
    if (!m_bInit) {
        m_bInit = true;
        // 循环取数据刷新
        bool bHasData = true;
        long long pre_t, cur_t, total_time = m_thread_start_pts[0];
        pre_t = getCurrentTime();
        
        while (1) {
            cur_t = getCurrentTime();
            long long tmp_tot = cur_t - pre_t;
            
            bHasData = SendData(total_time + tmp_tot);
            if (m_nCurThreadCnt <= 0 && !bHasData) {
                if (bHasData) {
                    printf("[-------------=-=-=-=-=-=-_###+_#+_#+#_+#_#+#_##_+#_#+#_#]\n");
                    continue;
                }
                break;
            }
            if (bHasData) {
                total_time += cur_t - pre_t;
                pre_t = cur_t;
            }
        }
        
        if (-1 != m_shmid) {
            close(m_shmid);
            unlink(kszSharedFilePath);
        }
    } // end if (!m_bInit)
}

void CBaseVideoPlay::Record(uint8_t *pImgData, int nWidth, int nHeight, bool bEnd)
{
    if (!m_bInit) {
        av_register_all();
        avformat_network_init();
    }
    
    // first time record init(include open shared memory data, start compress thread...)
    bool is_first = !m_is_recording;
    if (!m_is_recording) {
        m_is_recording = true;
        ++idx;
        // 打开共享内存
        int shmid = open(kszCompressionSharedFilePath, O_CREAT | O_RDWR, 0666);
        if (-1 == shmid) {
            printf("[CBaseVideoPlay::Record] error: shm_open failed.\n");
            exit(1);
        }
        __int64_t img_total_size = g_nMaxImageWidth * g_nMaxImageHeight * COMPRESS_SHARED_DATA_QUEUE_CAP * sizeof(uint8_t);
        if (-1 == ftruncate(shmid, img_total_size + sizeof(CompressSharedData_s))) {
            printf("[CBaseVideoPlay::Record] error: ftruncate failed, errno[%d].\n", errno);
            exit(1);
        }
        struct stat buf_stat;
        if (-1 == fstat(shmid, &buf_stat)) {
            printf("[CBaseVideoPlay::Record] error: fstat fstat.\n");
            exit(1);
        }
        printf("[CBaseVideoPlay::Record] info: size=%lld mode=%o.\n", buf_stat.st_size, buf_stat.st_mode & 0777);
        m_pCompressSharedData = (CompressSharedData_s*)mmap(NULL, buf_stat.st_size, PROT_WRITE, MAP_SHARED, shmid, 0);
        if (MAP_FAILED == m_pCompressSharedData) {
            printf("[CBaseVideoPlay::Record] error: mmap failed.\n");
            exit(1);
        }
        memset(m_pCompressSharedData, 0, buf_stat.st_size);
        
        std::thread t(CompressRGB);
        t.detach();
        m_compress_pts = getCurrentTime();
    }
    
    // 设置共享内存数据
    if (MAP_FAILED != m_pCompressSharedData && NULL != m_pCompressSharedData) {
        g_pThreadProductConsumerHelper->CompressBufferLock();
        int tail = m_pCompressSharedData->tail;
        // if shared memory data queue is full, waiting
        if ( (tail+1)%COMPRESS_SHARED_DATA_QUEUE_CAP == m_pCompressSharedData->head ) {
#ifdef ENCODE_QUEUE_LOG
            printf("[CBaseVideoPlay::Record] info: data queue is full, wait...\n");
#endif
            g_pThreadProductConsumerHelper->CompressBufferFullWait();
        }
        // set shared memory data
        int nRGBSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, nWidth, nHeight, 1);
        m_pCompressSharedData->img_data[tail] = (uint8_t*)malloc(nRGBSize);
        if (NULL == m_pCompressSharedData->img_data[tail]) {
            printf("[CBaseVideoPlay::Record] error: malloc memory failed.\n");
            g_pThreadProductConsumerHelper->CompressBufferUnlock();
            return;
        }
        memcpy(m_pCompressSharedData->img_data[tail], pImgData, nRGBSize);
        m_pCompressSharedData->width = nWidth;
        m_pCompressSharedData->height = nHeight;
        m_pCompressSharedData->is_end = bEnd;
        m_pCompressSharedData->is_first = is_first;
        m_pCompressSharedData->idx = idx;
        m_pCompressSharedData->pts = getCurrentTime() - m_compress_pts;
        
        m_pCompressSharedData->tail = (tail + 1) % COMPRESS_SHARED_DATA_QUEUE_CAP;
        // signal now is not empty
        if (tail == m_pCompressSharedData->head) {
#ifdef ENCODE_QUEUE_LOG
            printf("[CBaseVideoPlay::Record] info: data queue is not empty now.\n");
#endif
            g_pThreadProductConsumerHelper->CompressBufferNotEmptySignal();
        }
        g_pThreadProductConsumerHelper->CompressBufferUnlock();
    }
    
    if (bEnd) {
        m_is_recording = false;
    }
}

void CBaseVideoPlay::SetSavePath(const std::string& strSavePath)
{
    m_strSavePath = strSavePath;
}

void CBaseVideoPlay::SetVideoCbFunc(FuncVideoCB pFuncVideoCB)
{
    if (NULL == pFuncVideoCB) {
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
    if (m_strVideoPath.empty()) {
        printf("[CBaseVideoPlay::CheckValid] warning: not set video file path, default is current.\n");
        return false;
    }
    
    if (VIDEO_OP_TYPE_SAVE_BMP == m_eOpType) {
        if (m_strSavePath.empty()) {
            printf("[CBaseVideoPlay::CheckValid] warning: not set save picture file path, default is current.\n");
        }
    } else if (VIDEO_OP_TYPE_PLAY == m_eOpType) {
        if (NULL == m_pFuncVideoCB) {
            printf("[CBaseVideoPlay::CheckValid] error: not set call back function, please set before run.\n");
            return false;
        }
    } else if (m_nThreadCnt >= THREAD_MAX_COUNT) {
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
    if (m_strSavePath.empty()) {
        sprintf(filename, "./bmp/%s_%d.bmp", "filename", index);
    } else {
        sprintf(filename, "%s_%d.bmp", m_strSavePath.c_str(), index);
    }
    
    stbi_write_bmp(filename, width, height, 3, pFrameRGB->data[0]);
}

void CBaseVideoPlay::FillDataByPts(long long cur_clock, int idx, void *pData[THREAD_MAX_COUNT], std::string& strTexture, int &width, int &height, bool& bPreFull)
{
    int old_pre = m_pSharedData->pre[idx];
    int new_pre = old_pre;
    
    if (m_pSharedData->start_pts[idx] <= 0) {
        pData[idx] = NULL;
        return;
    }
    
    // 根据时间戳取数据，小于cur_clock的丢掉
    while (1) {
#ifdef DECODE_TIMESTAMP_LOG
        printf("############ pts[%lld], minus[%lld], new_pre[%d], back[%d] #####\n", m_pSharedData->pts[idx][new_pre], cur_clock - m_pSharedData->start_pts[idx], new_pre, m_pSharedData->back[idx]);
#endif
        if (cur_clock - m_pSharedData->start_pts[idx] > m_pSharedData->pts[idx][new_pre]) {
            if (NULL != m_pSharedData->pData[idx][new_pre]) {
                free(m_pSharedData->pData[idx][new_pre]);
                m_pSharedData->pData[idx][new_pre] = NULL;
            }
            new_pre = (new_pre + 1) % SHARED_DATA_QUEUE_COUNT;
            // without data any more
            if (new_pre >= m_pSharedData->back[idx]) {
                pData[idx] = NULL;
                goto EXIT;
            }
        } else {
            break;
        }
    }
    static long long pre = m_pSharedData->pts[idx][new_pre];
    static int cc = 0;
//    m_compress_pts = (m_pSharedData->pts[idx][new_pre] - pre) > m_compress_pts ? (m_pSharedData->pts[idx][new_pre] - pre) : m_compress_pts;
//    printf("-------=================== get pts[%lld] =============\n", m_pSharedData->pts[idx][new_pre]);
//    if (m_pSharedData->pts[idx][new_pre] - pre > 40) {
//        printf("**************************** cc=%d *******************************\n", cc++);
//    }
    
    pre = m_pSharedData->pts[idx][new_pre];
    
    pData[idx] = m_pSharedData->pData[idx][new_pre];
    width = m_pSharedData->nWidth[idx];
    height = m_pSharedData->nHeight[idx];
    strTexture = m_strTexture[idx];
    
EXIT:
    m_pSharedData->pre[idx] = new_pre;
    
    // 判断取数据之前队列是否满了
    if ((m_pSharedData->back[idx]+1)%SHARED_DATA_QUEUE_COUNT == old_pre) {
        bPreFull = true;
    }
}

bool CBaseVideoPlay::SendData(long long cur_clock)
{
    if (NULL != m_pFuncVideoCB) {
        // 读取数据刷新界面
        void* pData[THREAD_MAX_COUNT];
        int width[THREAD_MAX_COUNT], height[THREAD_MAX_COUNT];
        bool bHasData = false;
        bool bPreFull = false;
        std::string strTexture[THREAD_MAX_COUNT];
        for (int i = 0; i < THREAD_MAX_COUNT; ++i) {
            pData[i] = NULL;
        }
        
//        g_pThreadProductConsumerHelper->BufferLock();
        
        for (int i = 0; i < m_nThreadCnt; ++i) {
            if (m_pSharedData->pre[i] != m_pSharedData->back[i]) {
                bHasData = true;
                break;
            }
        }
        if (!bHasData) {
            if (m_nCurThreadCnt <= 0) {
//                g_pThreadProductConsumerHelper->BufferUnlock();
                return false;
            }
#ifdef DECODE_LOG
            printf("[CBaseVideoPlay::SendData] info: queue is empty, waiting.\n");
#endif
            return false;
//            g_pThreadProductConsumerHelper->BufferEmptyWait();
            bHasData = true;
        }
//        g_pThreadProductConsumerHelper->BufferUnlock();
//        g_pThreadProductConsumerHelper->BufferLock();
        for (int i = 0; i < m_nThreadCnt; ++i) {
            FillDataByPts(cur_clock, i, pData, strTexture[i], width[i], height[i], bPreFull);
        }
//        g_pThreadProductConsumerHelper->BufferUnlock();
        
        if (bPreFull) {
#ifdef DECODE_LOG
            printf("[CBaseVideoPlay::SendData] info: queue not full signal.\n");
#endif
//            g_pThreadProductConsumerHelper->BufferNotFullSignal();
        }
        
//        g_pThreadProductConsumerHelper->BufferUnlock();
        
        bool bRealData = false;
//        int nCnt = 0;
        for (int i = 0; i < m_nThreadCnt; ++i) {
            if (NULL != pData[i]) {
                bRealData = true;
//                ++nCnt;
                break;
            }
        }
        
        if (!bRealData) {
            return false;
        }
//        printf("***************** nCnt[%d], bRealData[%d], bHasData[%d] ****************\n", nCnt, bRealData, bHasData);
        
        if (bHasData) {
//            long long pre_t, cur_t;
//            static long long diff_t = getCurrentTime();
//            pre_t = getCurrentTime();
            (*m_pFuncVideoCB)(pData, width, height, strTexture);
//            cur_t = getCurrentTime();
//            printf("[-=-=---=-=-=-=-==] use time is %lld ms. ddd[%lld]\n", cur_t - pre_t, pre_t - diff_t);
//            diff_t = pre_t;
            for (int i=0; i<m_nThreadCnt; ++i) {
                if (NULL != pData[i]) {
                    //free(pData[i]);
                }
            }
            
            return true;
        }
    }
    
    return false;
}

static int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index, int idx)
{
    int ret, got_frame;
    AVPacket enc_pkt;
    
    if (NULL == fmt_ctx->streams || NULL == *(fmt_ctx->streams)
        || NULL == fmt_ctx->streams[stream_index]
        || !(fmt_ctx->streams[stream_index]->codec->codec->capabilities & CODEC_CAP_DELAY)) {
        return 0;
    }
    
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        
        ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt, NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0) {
            break;
        }
        if (got_frame) {
            printf("###### Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
            
            enc_pkt.pts = idx;
            enc_pkt.dts = idx++;
            av_packet_rescale_ts(&enc_pkt, fmt_ctx->streams[stream_index]->codec->time_base, fmt_ctx->streams[stream_index]->time_base);
            ret = av_write_frame(fmt_ctx, &enc_pkt);
            if (ret < 0) {
                printf("[flush_encoder] warning: av_write_frame failed.\n");
                break;
            }
        }
        
    } // end while(1)
    
    return ret;
}

CBaseVideoPlay::CompressSharedData_s *GetSharedMemory()
{
    int shmid = open(kszCompressionSharedFilePath, O_CREAT | O_RDWR, 0666);
    if (-1 == shmid) {
        printf("[CBaseVideoPlay::CompressRGB] error: shm_open failed.\n");
        exit(1);
    }
    struct stat buf_stat;
    if (-1 == fstat(shmid, &buf_stat)) {
        printf("[CBaseVideoPlay::CompressRGB] error: fstat fstat.\n");
        exit(1);
    }
    printf("[CBaseVideoPlay::CompressRGB] info: size=%lld mode=%o.\n", buf_stat.st_size, buf_stat.st_mode & 0777);
    CBaseVideoPlay::CompressSharedData_s *pSharedData = (CBaseVideoPlay::CompressSharedData_s*)mmap(NULL, buf_stat.st_size, PROT_WRITE, MAP_SHARED, shmid, 0);
    if (MAP_FAILED == pSharedData) {
        printf("[CBaseVideoPlay::CompressRGB] error: mmap failed.\n");
        exit(1);
    }
    
    g_pThreadProductConsumerHelper->CompressBufferLock();
    // if queue is empty, waiting
    if (pSharedData->head == pSharedData->tail) {
#ifdef ENCODE_QUEUE_LOG
        printf("[CBaseVideoPlay::CompressRGB] info: data queue is empty.\n");
#endif
        g_pThreadProductConsumerHelper->CompressBufferEmptyWait();
    }
    g_pThreadProductConsumerHelper->CompressBufferUnlock();
    
    return pSharedData;
}

void CBaseVideoPlay::CompressRGB()
{
    bool is_error = false;
    
    do {
        // get shared memory object
        CompressSharedData_s *pSharedData = GetSharedMemory();
        if (NULL == pSharedData) {
            printf("[] error: Get shared memory failed.\n");
            exit(1);
        }
        
        char out_file[g_default_file_path_length];
//        sprintf(out_file, "./video/screenst%d%s", pSharedData->idx, ".h264");
        sprintf(out_file, "./video/screenst%d%s", pSharedData->idx, ".mp4");
        
        // 用作之后写入视频帧并编码成 h264，贯穿整个工程当中
        AVFormatContext* pFormatCtx;
        pFormatCtx = avformat_alloc_context();
        if (NULL == pFormatCtx) {
            printf("[CBaseVideoPlay::CompressRGB] error: avformat_alloc_context failed.\n");
            is_error = true;
            break;
        }
        
        // 通过这个函数可以获取输出文件的编码格式, 那么这里我们的 fmt 为 h264 格式(AVOutputFormat *)
        AVOutputFormat* fmt;
        if (avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file) < 0) {
            printf("[CBaseVideoPlay::CompressRGB] error: avformat_alloc_output_context2 failed.\n");
            is_error = true;
            break;
        }
        fmt = pFormatCtx->oformat;
        
        // 打开文件的缓冲区输入输出，flags 标识为  AVIO_FLAG_READ_WRITE ，可读写
        if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
            printf("[CBaseVideoPlay::CompressRGB] error: avio_open failed.\n");
            is_error = true;
            break;
        }
        
        AVStream *video_st;
        // 通过媒体文件控制者获取输出文件的流媒体数据，这里 AVCodec * 写 0 ， 默认会为我们计算出合适的编码格式
        video_st = avformat_new_stream(pFormatCtx, 0);
        if (NULL == video_st) {
            printf("[CBaseVideoPlay::CompressRGB] error: avformat_new_stream failed.\n");
            is_error = true;
            break;
        }
        // 设置 25 帧每秒 ，也就是 fps 为 25
        video_st->time_base = (AVRational){1, 25000};
        video_st->r_frame_rate = (AVRational){25, 1};
        video_st->avg_frame_rate = (AVRational){25, 1};
        
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
        pCodecCtx->width = pSharedData->width;
        pCodecCtx->height = pSharedData->height;
        
        // 设置比特率，每秒传输多少比特数 bit，比特率越高，传送速度越快，也可以称作码率，
        // 视频中的比特是指由模拟信号转换为数字信号后，单位时间内的二进制数据量。
        pCodecCtx->bit_rate = 400000;
        // 设置图像组层的大小。
        // 图像组层是在 MPEG 编码器中存在的概念，图像组包 若干幅图像, 组头包 起始码、GOP 标志等,如视频磁带记录器时间、控制码、B 帧处理码等;
        pCodecCtx->gop_size = 250;
        // 设置 25 帧每秒 ，也就是 fps 为 25
        pCodecCtx->time_base = (AVRational){1, 50};
        pCodecCtx->framerate = (AVRational){25, 1};

        //设置 H264 中相关的参数
        pCodecCtx->qmin = 10;
        pCodecCtx->qmax = 25;
        
//        ost->st->id = oc->nb_streams-1;
        if (pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
            pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
        
        // 设置 B 帧最大的数量，B帧为视频图片空间的前后预测帧， B 帧相对于 I、P 帧来说，压缩率比较大，也就是说相同码率的情况下，
        // 越多 B 帧的视频，越清晰，现在很多打视频网站的高清视频，就是采用多编码 B 帧去提高清晰度，
        // 但同时对于编解码的复杂度比较高，比较消耗性能与时间
        pCodecCtx->max_b_frames = 3;
        // 可选设置
        AVDictionary* param = 0;
        if (AV_CODEC_ID_H264 == pCodecCtx->codec_id) {
            // 通过--preset的参数调节编码速度和质量的平衡。
            av_dict_set(&param, "preset", "slow", 0);
            // 通过--tune的参数值指定片子的类型，是和视觉优化的参数，或有特别的情况。
            // zerolatency: 零延迟，用在需要非常低的延迟的情况下，比如电视电话会议的编码
            av_dict_set(&param, "tune", "zerolatency", 0);
        }
        
        
//        avcodec_parameters_from_context(video_st->codecpar, pCodecCtx);
        // 通过 codec_id 找到对应的编码器
        AVCodec* pCodec;
        pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
        if (NULL == pCodec) {
            printf("[CBaseVideoPlay::CompressRGB] error: avcodec_find_encoder failed.\n");
            is_error = true;
            break;
        }
        int ret;
        // 打开编码器，并设置参数 param
        if ((ret = avcodec_open2(pCodecCtx, pCodec, &param)) < 0) {
            printf("[CBaseVideoPlay::CompressRGB] error: avcodec_open2 failed.[%d]\n", ret);
            is_error = true;
            break;
        }
        // 数据信息
        av_dump_format(pFormatCtx, 0, out_file, 1);
        
        // 目标frame
        AVFrame* pFrame;
        pFrame = av_frame_alloc();
        if (NULL == pFrame) {
            printf("[CBaseVideoPlay::CompressRGB] error: av_frame_alloc failed.\n");
            is_error = true;
            break;
        }
        
        // compress and record data
        if (pSharedData->is_first) {
            // 编写 h264 封装格式的文件头部，基本上每种编码都有着自己的格式的头部，想看具体实现的同学可以看看 h264 的具体实现
            avformat_write_header(pFormatCtx, NULL);
        }
        
        // ## 目标frame
        // 通过像素格式(这里为 YUV)获取图片的真实大小，例如将 480 * 272 转换成 int 类型
        int nPicSize = av_image_get_buffer_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
        // 将 picture_size 转换成字节数据，byte
        unsigned char* pPicBuf = (uint8_t*)av_mallocz(nPicSize);
        // 设置原始数据 AVFrame 的每一个frame 的图片大小，AVFrame 这里存储着 YUV 非压缩数据
        
        ret = av_image_fill_arrays(pFrame->data, pFrame->linesize, pPicBuf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Fill yuv data error!\n");
            exit(-2);
        }
        
        // 源frame
        AVFrame *pSrcFrame = av_frame_alloc();
        if (NULL == pSrcFrame) {
            printf("[CBaseVideoPlay::CompressRGB] error: av_frame_alloc failed.\n");
            is_error = true;
            break;
        }
        // 通过像素格式(这里为 YUV)获取图片的真实大小，例如将 480 * 272 转换成 int 类型
        int nRGBSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
        // 将 picture_size 转换成字节数据，byte
        unsigned char* pRGBBuf = (uint8_t*)av_malloc(nRGBSize);
        if (NULL == pRGBBuf) {
            av_log(NULL, AV_LOG_ERROR, "Malloc rgb data error!\n");
            exit(-1);
        }
        //
        SwsContext* pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
        if (NULL == pSwsCtx) {
            printf("[CBaseVideoPlay::CompressRGB] error: sws_getContext.\n");
            is_error = true;
            break;
        }
        
        AVPacket packet;
        if (0 != av_new_packet(&packet, nPicSize)) {
            printf("[CBaseVideoPlay::CompressRGB] error: av_new_packet failed.\n");
            is_error = true;
            break;
        }
        int head;
        int width, height;
        int idx = 0;
        bool is_end;
        AVRational tmp_time_base = (AVRational){1, 1000};
        
        static int icnt = 0;
        static long long st = getCurrentTime();
        
        while (1) {
            g_pThreadProductConsumerHelper->CompressBufferLock();
            head = pSharedData->head;
            // if queue is empty, waiting
            if (head == pSharedData->tail) {
#ifdef ENCODE_QUEUE_LOG
                printf("[CBaseVideoPlay::CompressRGB] info: data queue is empty.\n");
#endif
                g_pThreadProductConsumerHelper->CompressBufferEmptyWait();
            }
            
            width = pSharedData->width;
            height = pSharedData->height;
            is_end = pSharedData->is_end;
            
            // deal with data
            memcpy(pRGBBuf, pSharedData->img_data[head], nRGBSize);
            // 设置原始数据 AVFrame 的每一个frame 的图片大小，AVFrame 这里存储着 YUV 非压缩数据
            ret = av_image_fill_arrays(pSrcFrame->data, pSrcFrame->linesize, pRGBBuf, AV_PIX_FMT_RGB24, width, height, 1);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Fill image error!\n");
                exit(-1);
            }
            
            // 翻转RGB图像
            pSrcFrame->data[0]  += pSrcFrame->linesize[0] * (height - 1);
            pSrcFrame->linesize[0] *= -1;
            pSrcFrame->data[1]  += pSrcFrame->linesize[1] * (height / 2 - 1);
            pSrcFrame->linesize[1] *= -1;
            pSrcFrame->data[2]  += pSrcFrame->linesize[2] * (height / 2 - 1);
            pSrcFrame->linesize[2] *= -1;
            pSrcFrame->format = AV_PIX_FMT_RGB24;
            sws_scale(pSwsCtx, pSrcFrame->data, pSrcFrame->linesize, 0, height, pFrame->data, pFrame->linesize);
            pFrame->format = AV_PIX_FMT_YUV420P;
            pFrame->width = width;
            pFrame->height = height;
            // PTS
            // 设置这一帧的显示时间
            pFrame->pts = av_rescale_q(pSharedData->pts, tmp_time_base, pCodecCtx->time_base);
            int got_pic = 0;
            // 利用编码器进行编码，将 pFrame 编码后的数据传入 pkt 中
            if (avcodec_encode_video2(pCodecCtx, &packet, pFrame, &got_pic) < 0) {
                printf("[CBaseVideoPlay::CompressRGB] error: avcodec_encode_video2 failed.\n");
                is_error = true;
                g_pThreadProductConsumerHelper->CompressBufferUnlock();
                break;
            }
            if (got_pic != 1) {
                printf("[CBaseVideoPlay::CompressRGB] warning: got_pic[%d] != 1.\n", got_pic);
                g_pThreadProductConsumerHelper->CompressBufferUnlock();
                continue;
            }
            
            packet.stream_index = video_st->index;
            av_packet_rescale_ts(&packet, pCodecCtx->time_base, video_st->time_base);
            printf("-=- packet pts = %lld, dts = %lld, duration = %lld, pSharedData->pts[%lld], frame pts[%lld] -=-=-=-=-=-=-=-=-\n", packet.pts, packet.dts, packet.duration, pSharedData->pts, pFrame->pts);
            ++icnt;
            
            ret = av_interleaved_write_frame(pFormatCtx, &packet);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Write packet error!\n");
//                exit(1);
            }
        
            // free image data
            free(pSharedData->img_data[head]);
            pSharedData->img_data[head] = NULL;
            
            av_packet_unref(&packet);
            ++idx;
            
            ++pSharedData->head;
            if (pSharedData->head >= CBaseVideoPlay::COMPRESS_SHARED_DATA_QUEUE_CAP) {
                pSharedData->head = 0;
            }
            // if queue is full, sigal now is not
            if (pSharedData->tail + 1 == head
                || (head == 0 && pSharedData->tail + 1 == CBaseVideoPlay::COMPRESS_SHARED_DATA_QUEUE_CAP)) {
#ifdef ENCODE_QUEUE_LOG
                printf("[CBaseVideoPlay::CompressRGB] info: data queue is not full now.\n");
#endif
                g_pThreadProductConsumerHelper->CompressBufferNotFullSignal();
            }
            g_pThreadProductConsumerHelper->CompressBufferUnlock();
            
            // if is end
            if (is_end) {
                break;
            }
        }
        
        if (is_error) {
            break;
        }
        
//        if (flush_encoder(pFormatCtx, 0, idx) < 0) {
//            printf("[CBaseVideoPlay::CompressRGB] error: flush_encoder failed.\n");
//            is_error = true;
//            break;
//        }
        
        // 写入数据流尾部到输出文件当中，并释放文件的私有数据
        av_write_trailer(pFormatCtx);
        printf("-=-=-=-=-=-=-=-+#_+#_+_+#_+#_+# record cnt [%d] use time[%lld] idx[%d] -=-=-=-=\n", icnt, getCurrentTime()-st, idx);
        
        if (NULL != video_st) {
            // 关闭编码器
            avcodec_close(video_st->codec);
            // 释放 AVFrame
            av_frame_free(&pFrame);
            av_frame_free(&pSrcFrame);
            // 释放图片 buf，就是 free() 函数，硬要改名字，当然这是跟适应编译环境有关系的
            av_free(pPicBuf);
        }
        
        if (NULL != pRGBBuf) {
            free(pRGBBuf);
            pRGBBuf = NULL;
        }
        
        sws_freeContext(pSwsCtx);
        // 关闭输入数据的缓存
        avio_close(pFormatCtx->pb);
        // 释放 AVFromatContext 结构体
        avformat_free_context(pFormatCtx);
    } while(0);
    
    if (is_error) {
        printf("^^^^^^$$$$$$$$#####@@@##@#@#@##@#@#@##@#@#@#@$@@#@$#$##$@$#@$#@$#$#@$@$#@$##$@$#@\n\n\n");
        exit(1);
    }
}
