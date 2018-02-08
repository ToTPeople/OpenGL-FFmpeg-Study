//
//  CFFmpegYUV2H264.hpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/31.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CFFmpegYUV2H264_hpp
#define CFFmpegYUV2H264_hpp

#include <stdio.h>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
};
#endif

struct AVFormatContext;
//
struct AVOutputFormat;
struct AVStream;
struct AVCodecContext;
struct AVDictionary;
struct AVCodec;
struct AVFrame;
struct SwsContext;
//

class CFFmpegYUV2H264
{
public:
    static CFFmpegYUV2H264* GetInstance();
    
public:
    void SetYUVFilePath(const std::string& strYUVFilePath);
    void Run();
    void Record(uint8_t* pImgData, int nWidth, int nHeight, bool bEnd);
    void End();
    
private:
    CFFmpegYUV2H264();
    ~CFFmpegYUV2H264();
    
    int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index);
    
    // tmp
    void Init(const char* out_file, uint8_t* pImgData, int nWidth, int nHeight);
    // tmp
    
private:
    static CFFmpegYUV2H264*     m_pInstance;
    
    std::string                 m_strYUVFile;
    
    // tmp
    bool                m_bInit;
    AVFormatContext* pFormatCtx;
    AVOutputFormat* fmt;
    AVStream *video_st;
    AVCodecContext* pCodecCtx;
    AVDictionary* param;
    AVCodec* pCodec;
    AVFrame* pFrame;
    AVFrame *pSrcFrame;
    SwsContext* pSwsCtx;
    unsigned char* pPicBuf;
    unsigned char* pRGBBuf;
    AVPacket packet;
    int idx;
    // tmp
};

#define g_pFFmpegYUV2H264Instance (CFFmpegYUV2H264::GetInstance())

#endif /* CFFmpegYUV2H264_hpp */
