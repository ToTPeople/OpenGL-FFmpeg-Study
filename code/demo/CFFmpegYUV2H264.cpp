//
//  CFFmpegYUV2H264.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/31.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CFFmpegYUV2H264.hpp"
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

CFFmpegYUV2H264* CFFmpegYUV2H264::m_pInstance = NULL;

CFFmpegYUV2H264* CFFmpegYUV2H264::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CFFmpegYUV2H264();
    }
    
    return m_pInstance;
}

void CFFmpegYUV2H264::SetYUVFilePath(const std::string& strYUVFilePath)
{
    m_strYUVFile = strYUVFilePath;
}

void CFFmpegYUV2H264::Run()
{
    if (m_strYUVFile.empty())
    {
        printf("[CFFmpegYUV2H264::Run] error: YUV file is empty.\n");
        exit(1);
    }
    
    FILE* in_file = fopen(m_strYUVFile.c_str(), "rb");
    const char* out_file = "./video/trans.h264";
    
    // 注册 ffmpeg 中的所有的封装、解封装 和 协议等，当然，你也可用以下两个函数代替
    av_register_all();
    
    // 用作之后写入视频帧并编码成 h264，贯穿整个工程当中
    AVFormatContext* pFormatCtx;
    pFormatCtx = avformat_alloc_context();
    if (NULL == pFormatCtx)
    {
        printf("[CFFmpegYUV2H264::Run] error: avformat_alloc_context failed.\n");
        fclose(in_file);
        exit(1);
    }
    
    // 通过这个函数可以获取输出文件的编码格式, 那么这里我们的 fmt 为 h264 格式(AVOutputFormat *)
    AVOutputFormat* fmt;
#if 0
    fmt = av_guess_format(NULL, out_file, NULL);
    pFormatCtx->oformat = fmt;
#else
    avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
    fmt = pFormatCtx->oformat;
#endif
    
    // 打开文件的缓冲区输入输出，flags 标识为  AVIO_FLAG_READ_WRITE ，可读写
    if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0)
    {
        printf("[CFFmpegYUV2H264::Run] error: avio_open failed.\n");
        fclose(in_file);
        exit(1);
    }
    
    AVStream *video_st;
    // 通过媒体文件控制者获取输出文件的流媒体数据，这里 AVCodec * 写 0 ， 默认会为我们计算出合适的编码格式
    video_st = avformat_new_stream(pFormatCtx, 0);
    if (NULL == video_st)
    {
        printf("[CFFmpegYUV2H264::Run] error: avformat_new_stream failed.\n");
        fclose(in_file);
        exit(1);
    }
    // 设置 25 帧每秒 ，也就是 fps 为 25
    //video_st->time_base.num = 1;
    //video_st->time_base.den = 25;
    
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
    pCodecCtx->width = 480;
    pCodecCtx->height = 272;
    
    // 设置比特率，每秒传输多少比特数 bit，比特率越高，传送速度越快，也可以称作码率，
    // 视频中的比特是指由模拟信号转换为数字信号后，单位时间内的二进制数据量。
    pCodecCtx->bit_rate = 400000;
    // 设置图像组层的大小。
    // 图像组层是在 MPEG 编码器中存在的概念，图像组包 若干幅图像, 组头包 起始码、GOP 标志等,如视频磁带记录器时间、控制码、B 帧处理码等;
    pCodecCtx->gop_size = 250;
    // 设置 25 帧每秒 ，也就是 fps 为 25
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    //设置 H264 中相关的参数
#if 0
    pCodecCtx->me_range = 16;
    pCodecCtx->max_qdiff = 4;
    pCodecCtx->qcompress = 0.6;
#endif
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
        av_dict_set(&param, "preset", "slow", 0);
        // 通过--tune的参数值指定片子的类型，是和视觉优化的参数，或有特别的情况。
        // zerolatency: 零延迟，用在需要非常低的延迟的情况下，比如电视电话会议的编码
        av_dict_set(&param, "tune", "zerolatency", 0);
    }
    //H.265
    if (AV_CODEC_ID_H265 == pCodecCtx->codec_id)
    {
        av_dict_set(&param, "preset", "ultrafast", 0);
        av_dict_set(&param, "tune", "zero-latency", 0);
    }
    
    // 数据信息
    av_dump_format(pFormatCtx, 0, out_file, 1);
    
    // 通过 codec_id 找到对应的编码器
    AVCodec* pCodec;
    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (NULL == pCodec)
    {
        printf("[CFFmpegYUV2H264::Run] error: avcodec_find_encoder failed.\n");
        fclose(in_file);
        exit(1);
    }
    // 打开编码器，并设置参数 param
    if (avcodec_open2(pCodecCtx, pCodec, &param) < 0)
    {
        printf("[CFFmpegYUV2H264::Run] error: avcodec_open2 failed.\n");
        fclose(in_file);
        exit(1);
    }
    
    AVFrame* pFrame;
    pFrame = av_frame_alloc();
    if (NULL == pFrame)
    {
        printf("[CFFmpegYUV2H264::Run] error: av_frame_alloc failed.\n");
        fclose(in_file);
        exit(1);
    }
    // 通过像素格式(这里为 YUV)获取图片的真实大小，例如将 480 * 272 转换成 int 类型
    int nPicSize = av_image_get_buffer_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
    // 将 picture_size 转换成字节数据，byte
    unsigned char* pPicBuf = (uint8_t*)av_malloc(nPicSize);
    // 设置原始数据 AVFrame 的每一个frame 的图片大小，AVFrame 这里存储着 YUV 非压缩数据
    av_image_fill_arrays(pFrame->data, pFrame->linesize, pPicBuf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
    
    
    // 编写 h264 封装格式的文件头部，基本上每种编码都有着自己的格式的头部，想看具体实现的同学可以看看 h264 的具体实现
    avformat_write_header(pFormatCtx, NULL);
    
    AVPacket packet;
    if (0 != av_new_packet(&packet, nPicSize))
    {
        printf("[CFFmpegYUV2H264::Run] error: av_new_packet failed.\n");
        fclose(in_file);
        exit(1);
    }
    
    // 设置 yuv 数据中 y 图的宽高
    int y_size = pCodecCtx->width * pCodecCtx->height;
    int framenum = 100;
    int framecnt = 0;
    for (int i = 0; i < framenum; ++i)
    {
        if (fread(pPicBuf, 1, y_size*3/2, in_file) <= 0)
        {
            printf("[CFFmpegYUV2H264::Run] error: fread failed.\n");
            fclose(in_file);
            exit(1);
        }
        else if (feof(in_file))
        {
            break;
        }
        
        pFrame->data[0] = pPicBuf;              // Y
        pFrame->data[1] = pPicBuf + y_size;     // U
        pFrame->data[2] = pPicBuf + y_size*5/4; // V
        
        // PTS
        // 设置这一帧的显示时间
        pFrame->pts = i * (video_st->time_base.den) / ((video_st->time_base.num) * 25);
        int got_pic = 0;
#if 1
        // 利用编码器进行编码，将 pFrame 编码后的数据传入 pkt 中
        if (avcodec_encode_video2(pCodecCtx, &packet, pFrame, &got_pic) < 0)
        {
            printf("[CFFmpegYUV2H264::Run] error: avcodec_encode_video2 failed.\n");
            fclose(in_file);
            exit(1);
        }
        if (got_pic != 1)
        {
            continue;
        }
#else
        // avcodec_send_frame()/avcodec_receive_packet()
        //got_pic = avcodec_receive_packet(pCodecCtx, &packet);
        got_pic = avcodec_send_frame(pCodecCtx, pFrame);
        got_pic += avcodec_receive_packet(pCodecCtx, &packet);
        if (0 != got_pic)
        {
            printf("[CFFmpegYUV2H264::Run] error: avcodec_send_frame or avcodec_receive_packet failed.\n");
            fclose(in_file);
            exit(1);
        }
#endif
        
        // 编码成功后写入 AVPacket 到 输入输出数据操作着 pFormatCtx 中，当然，记得释放内存
        printf("------- Succeed to encode frame: %5d\tsize:%5d\n", framecnt, packet.size);
        ++framecnt;
        
        packet.stream_index = video_st->index;
        av_write_frame(pFormatCtx, &packet);
        
        av_packet_unref(&packet);
    }
    
    if (flush_encoder(pFormatCtx, 0) < 0)
    {
        printf("[CFFmpegYUV2H264::Run] error: flush_encoder failed.\n");
        fclose(in_file);
        exit(1);
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
    
    fclose(in_file);
}

CFFmpegYUV2H264::CFFmpegYUV2H264()
: m_strYUVFile("")
, m_bInit(false)
, idx(0)
{
}

CFFmpegYUV2H264::~CFFmpegYUV2H264()
{
}

int CFFmpegYUV2H264::flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index)
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

void CFFmpegYUV2H264::Record(uint8_t* pImgData, int nWidth, int nHeight, bool bEnd)
{
    const char* out_file = "./video/screenshots.h264";
    
    Init(out_file, pImgData, nWidth, nHeight);
    if (!m_bInit)
    {
        printf("[CFFmpegYUV2H264::Record()] init failed\n");
        return;
    }
    
    if (bEnd)
    {
        End();
        return;
    }
    
    AVFrame *pSrcFrame = av_frame_alloc();
    if (NULL == pSrcFrame)
    {
        printf("[CFFmpegYUV2H264::Record] error: av_frame_alloc failed.\n");
        exit(1);
    }
    
    // 通过像素格式(这里为 YUV)获取图片的真实大小，例如将 480 * 272 转换成 int 类型
    int nRGBSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
    // 将 picture_size 转换成字节数据，byte
    unsigned char* pRGBBuf = (uint8_t*)av_malloc(nRGBSize);
    pRGBBuf = pImgData;
    // 设置原始数据 AVFrame 的每一个frame 的图片大小，AVFrame 这里存储着 YUV 非压缩数据
    av_image_fill_arrays(pSrcFrame->data, pSrcFrame->linesize, pRGBBuf, AV_PIX_FMT_RGB24, nWidth, nHeight, 1);
    
    SwsContext* pSwsCtx = sws_getContext(nWidth, nHeight, AV_PIX_FMT_RGB24, nWidth, nHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    
    //pSrcFrame
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
        pFrame->pts = idx * (video_st->time_base.den) / ((video_st->time_base.num) * 25);
        //pFrame->pts = idx * (pCodecCtx->time_base.den) / (pCodecCtx->time_base.num * 25);
        int got_pic = 0;

        // 利用编码器进行编码，将 pFrame 编码后的数据传入 pkt 中
        if (avcodec_encode_video2(pCodecCtx, &packet, pFrame, &got_pic) < 0)
        {
            printf("[CFFmpegYUV2H264::Record] error: avcodec_encode_video2 failed.\n");
            exit(1);
        }
        if (got_pic != 1)
        {
            return;
        }
        
        // 编码成功后写入 AVPacket 到 输入输出数据操作着 pFormatCtx 中，当然，记得释放内存
        printf("------- Succeed to encode frame: %5d\tsize:%5d\n", idx, packet.size);
        ++idx;
        
        packet.stream_index = video_st->index;
        av_write_frame(pFormatCtx, &packet);
        
        av_packet_unref(&packet);
    }
    
    
}

void CFFmpegYUV2H264::End()
{
    if (flush_encoder(pFormatCtx, 0) < 0)
    {
        printf("[CFFmpegYUV2H264::End] error: flush_encoder failed.\n");
        exit(1);
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
    
    m_bInit = false;
}

void CFFmpegYUV2H264::Init(const char* out_file, uint8_t* pImgData, int nWidth, int nHeight)
{
    if (out_file == NULL)
    {
        return;
    }
    
    if (!m_bInit)
    {
        m_bInit = true;
        // 注册 ffmpeg 中的所有的封装、解封装 和 协议等，当然，你也可用以下两个函数代替
        av_register_all();
        
        // 用作之后写入视频帧并编码成 h264，贯穿整个工程当中
        pFormatCtx = avformat_alloc_context();
        if (NULL == pFormatCtx)
        {
            printf("[CFFmpegYUV2H264::Init] error: avformat_alloc_context failed.\n");
            exit(1);
        }
        
        // 通过这个函数可以获取输出文件的编码格式, 那么这里我们的 fmt 为 h264 格式(AVOutputFormat *)
#if 0
        fmt = av_guess_format(NULL, out_file, NULL);
        pFormatCtx->oformat = fmt;
#else
        avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
        fmt = pFormatCtx->oformat;
#endif
        
        // 打开文件的缓冲区输入输出，flags 标识为  AVIO_FLAG_READ_WRITE ，可读写
        if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0)
        {
            printf("[CFFmpegYUV2H264::Init] error: avio_open failed.\n");
            exit(1);
        }
        
        // 通过媒体文件控制者获取输出文件的流媒体数据，这里 AVCodec * 写 0 ， 默认会为我们计算出合适的编码格式
        video_st = avformat_new_stream(pFormatCtx, 0);
        if (NULL == video_st)
        {
            printf("[CFFmpegYUV2H264::Init] error: avformat_new_stream failed.\n");
            exit(1);
        }
        
        video_st->time_base.den = 25;
        video_st->time_base.num = 1;
        
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
        pCodecCtx->bit_rate = 400000;
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
        param = 0;
        if (AV_CODEC_ID_H264 == pCodecCtx->codec_id)
        {
            // 通过--preset的参数调节编码速度和质量的平衡。
            av_dict_set(&param, "preset", "slow", 0);
            // 通过--tune的参数值指定片子的类型，是和视觉优化的参数，或有特别的情况。
            // zerolatency: 零延迟，用在需要非常低的延迟的情况下，比如电视电话会议的编码
            av_dict_set(&param, "tune", "zerolatency", 0);
        }
        //H.265
        if (AV_CODEC_ID_H265 == pCodecCtx->codec_id)
        {
            av_dict_set(&param, "preset", "ultrafast", 0);
            av_dict_set(&param, "tune", "zero-latency", 0);
        }
        
        // 数据信息
        printf("==========================================\n");
        av_dump_format(pFormatCtx, 0, out_file, 1);
        printf("==========================================\n");
        
        // 通过 codec_id 找到对应的编码器
        pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
        if (NULL == pCodec)
        {
            printf("[CFFmpegYUV2H264::Init] error: avcodec_find_encoder failed.\n");
            exit(1);
        }
        // 打开编码器，并设置参数 param
        if (avcodec_open2(pCodecCtx, pCodec, &param) < 0)
        {
            printf("[CFFmpegYUV2H264::Init] error: avcodec_open2 failed.\n");
            exit(1);
        }
        
        // 目标frame
        int nPicSize = 0;
        {
            pFrame = av_frame_alloc();
            if (NULL == pFrame)
            {
                printf("[CFFmpegYUV2H264::Init] error: av_frame_alloc failed.\n");
                exit(1);
            }
            
            // 通过像素格式(这里为 YUV)获取图片的真实大小，例如将 480 * 272 转换成 int 类型
            nPicSize = av_image_get_buffer_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
            // 将 picture_size 转换成字节数据，byte
            pPicBuf = (uint8_t*)av_malloc(nPicSize);
            // 设置原始数据 AVFrame 的每一个frame 的图片大小，AVFrame 这里存储着 YUV 非压缩数据
            av_image_fill_arrays(pFrame->data, pFrame->linesize, pPicBuf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, 1);
        }
        // 源frame
        /*{
            pSrcFrame = av_frame_alloc();
            if (NULL == pSrcFrame)
            {
                printf("[CFFmpegYUV2H264::Record] error: av_frame_alloc failed.\n");
                exit(1);
            }
            
            // 通过像素格式(这里为 YUV)获取图片的真实大小，例如将 480 * 272 转换成 int 类型
            int nRGBSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
            // 将 picture_size 转换成字节数据，byte
            pRGBBuf = (uint8_t*)av_malloc(nRGBSize);
            pRGBBuf = pImgData;
            // 设置原始数据 AVFrame 的每一个frame 的图片大小，AVFrame 这里存储着 YUV 非压缩数据
            av_image_fill_arrays(pSrcFrame->data, pSrcFrame->linesize, pRGBBuf, AV_PIX_FMT_RGB24, nWidth, nHeight, 1);
            
            pSwsCtx = sws_getContext(nWidth, nHeight, AV_PIX_FMT_RGB24, nWidth, nHeight, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
        }*/
        
        
        // 编写 h264 封装格式的文件头部，基本上每种编码都有着自己的格式的头部，想看具体实现的同学可以看看 h264 的具体实现
        avformat_write_header(pFormatCtx, NULL);
        
        if (0 != av_new_packet(&packet, nPicSize))
        {
            printf("[CFFmpegYUV2H264::Init] error: av_new_packet failed.\n");
            exit(1);
        }
    }
}
