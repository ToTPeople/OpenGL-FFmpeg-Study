//
//  ffmpeg2pic.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/29.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "ffmpeg2pic.hpp"

#include <string.h>
#include <stdlib.h>

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



//保存BMP文件的函数
void SaveAsBMP (AVFrame *pFrameRGB, int width, int height, int index, int bpp)
{
    char *filename = new char[255];
    //文件存放路径，根据自己的修改
    sprintf(filename, "./bmp/%s_%d.bmp", "filename", index);
    stbi_write_bmp(filename, width, height, 3, pFrameRGB->data[0]);
}

int FFmpeg2pic()
{
    unsigned int i = 0, videoStream = -1;
    AVCodecContext *pCodecCtx;
    AVFormatContext *pFormatCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameRGB;
    struct SwsContext *pSwsCtx;
    const char *filename = "./100.mp4";
    AVPacket packet;
    int frameFinished;
    int PictureSize;
    uint8_t *buf;
    
    //注册编解码器
    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    //打开视频文件
    if (avformat_open_input(&pFormatCtx, filename, NULL,NULL) != 0 )
    {
        printf ("av open input file failed!\n");
        exit (1);
    }
    //获取流信息
    if ( avformat_find_stream_info(pFormatCtx,NULL) < 0 )
    {
        printf ("av find stream info failed!\n");
        exit (1);
    }
    //获取视频流
    for ( i=0; i<pFormatCtx->nb_streams; i++ )
    {
        if ( pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
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
    pCodec = avcodec_find_decoder (pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        printf ("avcode find decoder failed!\n");
        exit (1);
    }
    //打开解码器
    if ( avcodec_open2(pCodecCtx, pCodec,NULL)<0 )
    {
        printf ("avcode open failed!\n");
        exit (1);
    }
    //为每帧图像分配内存
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    if ( (pFrame==NULL)||(pFrameRGB==NULL) )
    {
        printf("avcodec alloc frame failed!\n");
        exit (1);
        
    }
    //获得帧图大小
    enum AVPixelFormat nFormat = AV_PIX_FMT_RGB24;//AV_PIX_FMT_BGR24; // ## 设置不同，形成的图片RGB不一样，显示可能会异常
    //PictureSize = avpicture_get_size(nFormat, pCodecCtx->width, pCodecCtx->height);
    PictureSize = av_image_get_buffer_size(nFormat, pCodecCtx->width, pCodecCtx->height, 1);
    buf = (uint8_t*)av_malloc(PictureSize);
    if ( buf == NULL )
    {
        printf( "av malloc failed!\n");
        exit(1);
    }
    //avpicture_fill ( (AVPicture *)pFrameRGB, buf, nFormat, pCodecCtx->width,  pCodecCtx->height);
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buf, nFormat, pCodecCtx->width, pCodecCtx->height, 1);
    //设置图像转换上下文
    pSwsCtx = sws_getContext (pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, nFormat, SWS_BICUBIC, NULL, NULL, NULL);
    i = 0;
    while(av_read_frame(pFormatCtx, &packet) >= 0)
    {
        if(packet.stream_index==videoStream)
        {
            //真正解码
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,&packet);
            if(frameFinished)
            {
                //反转图像 ，否则生成的图像是上下调到的
                /*
                pFrame->data[0] += pFrame->linesize[0] * (pCodecCtx->height - 1);
                pFrame->linesize[0] *= -1;
                pFrame->data[1] += pFrame->linesize[1] * (pCodecCtx->height / 2 - 1);
                pFrame->linesize[1] *= -1;
                pFrame->data[2] += pFrame->linesize[2] * (pCodecCtx->height / 2 - 1);
                pFrame->linesize[2] *= -1;
                */
                //转换图像格式，将解压出来的YUV420P的图像转换为BRG24的图像
                sws_scale (pSwsCtx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                //保存为bmp图
                SaveAsBMP (pFrameRGB, pCodecCtx->width, pCodecCtx->height,i, 24);
                i++;
            }
            av_free_packet(&packet);
        }
    }
    
    sws_freeContext (pSwsCtx);
    av_free (pFrame);
    av_free (pFrameRGB);
    avcodec_close (pCodecCtx);
    avformat_close_input (&pFormatCtx);
    
    return 0;
}
