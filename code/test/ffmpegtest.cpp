//
//  ffmpegtest.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/1/29.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "ffmpegtest.hpp"

/**
 * 最简单的FFmpeg Helloworld程序
 * Simplest FFmpeg HelloWorld
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 *
 * 本程序是基于FFmpeg函数的最简单的程序。它可以打印出FFmpeg类库的下列信息：
 * Protocol:  FFmpeg类库支持的协议
 * AVFormat:  FFmpeg类库支持的封装格式
 * AVCodec:   FFmpeg类库支持的编解码器
 * AVFilter:  FFmpeg类库支持的滤镜
 * Configure: FFmpeg类库的配置信息
 *
 * This is the simplest program based on FFmpeg API. It can show following
 * informations about FFmpeg library:
 * Protocol:  Protocols supported by FFmpeg.
 * AVFormat:  Container format supported by FFmpeg.
 * AVCodec:   Encoder/Decoder supported by FFmpeg.
 * AVFilter:  Filters supported by FFmpeg.
 * Configure: configure information of FFmpeg.
 *
 */

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif
#endif

//FIX
struct URLProtocol;

char * urlprotocolinfo();
char * avformatinfo();
char * avcodecinfo();
char * avfilterinfo();
char * configurationinfo();

int FFmpegTest()
{
    char *infostr=NULL;
    infostr=configurationinfo();
    printf("\n<<Configuration>>\n%s",infostr);
    free(infostr);
    
    infostr=urlprotocolinfo();
    printf("\n<<URLProtocol>>\n%s",infostr);
    free(infostr);
    
    infostr=avformatinfo();
    printf("\n<<AVFormat>>\n%s",infostr);
    free(infostr);
    
    infostr=avcodecinfo();
    printf("\n<<AVCodec>>\n%s",infostr);
    free(infostr);
    
    infostr=avfilterinfo();
    printf("\n<<AVFilter>>\n%s",infostr);
    free(infostr);
    
    return 0;
}

/**
 * Protocol Support Information
 */
char * urlprotocolinfo()
{
    
    char *info=(char *)malloc(40000);
    memset(info,0,40000);
    
    av_register_all();
    
    struct URLProtocol *pup = NULL;
    //Input
    struct URLProtocol **p_temp = &pup;
    avio_enum_protocols((void **)p_temp, 0);
    while ((*p_temp) != NULL){
        sprintf(info, "%s[In ][%10s]\n", info, avio_enum_protocols((void **)p_temp, 0));
    }
    pup = NULL;
    //Output
    avio_enum_protocols((void **)p_temp, 1);
    while ((*p_temp) != NULL){
        sprintf(info, "%s[Out][%10s]\n", info, avio_enum_protocols((void **)p_temp, 1));
    }
    
    return info;
}

/**
 * AVFormat Support Information
 */
char * avformatinfo()
{
    
    char *info=(char *)malloc(40000);
    memset(info,0,40000);
    
    av_register_all();
    
    AVInputFormat *if_temp = av_iformat_next(NULL);
    AVOutputFormat *of_temp = av_oformat_next(NULL);
    //Input
    while(if_temp!=NULL){
        sprintf(info, "%s[In ] %10s\n", info, if_temp->name);
        if_temp=if_temp->next;
    }
    //Output
    while (of_temp != NULL){
        sprintf(info, "%s[Out] %10s\n", info, of_temp->name);
        of_temp = of_temp->next;
    }
    return info;
}

/**
 * AVCodec Support Information
 */
char * avcodecinfo()
{
    char *info=(char *)malloc(40000);
    memset(info,0,40000);
    
    av_register_all();
    
    AVCodec *c_temp = av_codec_next(NULL);
    
    while(c_temp!=NULL){
        if (c_temp->decode!=NULL){
            sprintf(info, "%s[Dec]", info);
        }
        else{
            sprintf(info, "%s[Enc]", info);
        }
        switch (c_temp->type){
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s[Video]", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s[Audio]", info);
                break;
            default:
                sprintf(info, "%s[Other]", info);
                break;
        }
        
        sprintf(info, "%s %10s\n", info, c_temp->name);
        
        c_temp=c_temp->next;
    }
    return info;
}

/**
 * AVFilter Support Information
 */
char * avfilterinfo()
{
    char *info=(char *)malloc(40000);
    memset(info,0,40000);
    
    avfilter_register_all();
    
    AVFilter *f_temp = (AVFilter *)avfilter_next(NULL);
    
    while (f_temp != NULL){
        sprintf(info, "%s[%15s]\n", info, f_temp->name);
        f_temp=f_temp->next;
    }
    return info;
}

/**
 * Configuration Information
 */
char * configurationinfo()
{
    char *info=(char *)malloc(40000);
    memset(info,0,40000);
    
    av_register_all();
    
    sprintf(info, "%s\n", avcodec_configuration());
    
    return info;
}

////////////////////////////////code_view() start/////////////////////////////////////
int code_view()
{
    //Parameters
    FILE *src_file =fopen("sintel_480x272_yuv420p.yuv", "rb");
    const int src_w=480,src_h=272;
    AVPixelFormat src_pixfmt=AV_PIX_FMT_YUV420P;
    
    int src_bpp=av_get_bits_per_pixel(av_pix_fmt_desc_get(src_pixfmt));
    
    FILE *dst_file = fopen("sintel_1280x720_rgb24.rgb", "wb");
    const int dst_w=1280,dst_h=720;
    AVPixelFormat dst_pixfmt=AV_PIX_FMT_RGB24;
    int dst_bpp=av_get_bits_per_pixel(av_pix_fmt_desc_get(dst_pixfmt));
    
    //Structures
    uint8_t *src_data[4];
    int src_linesize[4];
    
    uint8_t *dst_data[4];
    int dst_linesize[4];
    
    int rescale_method=SWS_BICUBIC;
    struct SwsContext *img_convert_ctx;
    uint8_t *temp_buffer=(uint8_t *)malloc(src_w*src_h*src_bpp/8);
    
    int frame_idx=0;
    int ret=0;
    ret= av_image_alloc(src_data, src_linesize,src_w, src_h, src_pixfmt, 1);
    if (ret< 0) {
        printf( "Could not allocate source image\n");
        return -1;
    }
    ret = av_image_alloc(dst_data, dst_linesize,dst_w, dst_h, dst_pixfmt, 1);
    if (ret< 0) {
        printf( "Could not allocate destination image\n");
        return -1;
    }
    //-----------------------------
    //Init Method 1
    img_convert_ctx =sws_alloc_context();
    //Show AVOption
    av_opt_show2(img_convert_ctx,stdout,AV_OPT_FLAG_VIDEO_PARAM,0);
    //Set Value
    av_opt_set_int(img_convert_ctx,"sws_flags",SWS_BICUBIC|SWS_PRINT_INFO,0);
    av_opt_set_int(img_convert_ctx,"srcw",src_w,0);
    av_opt_set_int(img_convert_ctx,"srch",src_h,0);
    av_opt_set_int(img_convert_ctx,"src_format",src_pixfmt,0);
    //'0' for MPEG (Y:0-235);'1' for JPEG (Y:0-255)
    av_opt_set_int(img_convert_ctx,"src_range",1,0);
    av_opt_set_int(img_convert_ctx,"dstw",dst_w,0);
    av_opt_set_int(img_convert_ctx,"dsth",dst_h,0);
    av_opt_set_int(img_convert_ctx,"dst_format",dst_pixfmt,0);
    av_opt_set_int(img_convert_ctx,"dst_range",1,0);
    sws_init_context(img_convert_ctx,NULL,NULL);
    
    //Init Method 2
    //img_convert_ctx = sws_getContext(src_w, src_h,src_pixfmt, dst_w, dst_h, dst_pixfmt,
    //    rescale_method, NULL, NULL, NULL);
    //-----------------------------
    /*
     //Colorspace
     ret=sws_setColorspaceDetails(img_convert_ctx,sws_getCoefficients(SWS_CS_ITU601),0,
     sws_getCoefficients(SWS_CS_ITU709),0,
     0, 1 << 16, 1 << 16);
     if (ret==-1) {
     printf( "Colorspace not support.\n");
     return -1;
     }
     */
    while(1)
    {
        if (fread(temp_buffer, 1, src_w*src_h*src_bpp/8, src_file) != src_w*src_h*src_bpp/8){
            break;
        }
        
        switch(src_pixfmt){
            case AV_PIX_FMT_GRAY8:{
                memcpy(src_data[0],temp_buffer,src_w*src_h);
                break;
            }
            case AV_PIX_FMT_YUV420P:{
                memcpy(src_data[0],temp_buffer,src_w*src_h);                    //Y
                memcpy(src_data[1],temp_buffer+src_w*src_h,src_w*src_h/4);      //U
                memcpy(src_data[2],temp_buffer+src_w*src_h*5/4,src_w*src_h/4);  //V
                break;
            }
            case AV_PIX_FMT_YUV422P:{
                memcpy(src_data[0],temp_buffer,src_w*src_h);                    //Y
                memcpy(src_data[1],temp_buffer+src_w*src_h,src_w*src_h/2);      //U
                memcpy(src_data[2],temp_buffer+src_w*src_h*3/2,src_w*src_h/2);  //V
                break;
            }
            case AV_PIX_FMT_YUV444P:{
                memcpy(src_data[0],temp_buffer,src_w*src_h);                    //Y
                memcpy(src_data[1],temp_buffer+src_w*src_h,src_w*src_h);        //U
                memcpy(src_data[2],temp_buffer+src_w*src_h*2,src_w*src_h);      //V
                break;
            }
            case AV_PIX_FMT_YUYV422:{
                memcpy(src_data[0],temp_buffer,src_w*src_h*2);                  //Packed
                break;
            }
            case AV_PIX_FMT_RGB24:{
                memcpy(src_data[0],temp_buffer,src_w*src_h*3);                  //Packed
                break;
            }
            default:{
                printf("Not Support Input Pixel Format.\n");
                break;
            }
        }
        
        sws_scale(img_convert_ctx, src_data, src_linesize, 0, src_h, dst_data, dst_linesize);
        printf("Finish process frame %5d\n",frame_idx);
        frame_idx++;
        
        switch(dst_pixfmt){
            case AV_PIX_FMT_GRAY8:{
                fwrite(dst_data[0],1,dst_w*dst_h,dst_file);
                break;
            }
            case AV_PIX_FMT_YUV420P:{
                fwrite(dst_data[0],1,dst_w*dst_h,dst_file);                 //Y
                fwrite(dst_data[1],1,dst_w*dst_h/4,dst_file);               //U
                fwrite(dst_data[2],1,dst_w*dst_h/4,dst_file);               //V
                break;
            }
            case AV_PIX_FMT_YUV422P:{
                fwrite(dst_data[0],1,dst_w*dst_h,dst_file);                    //Y
                fwrite(dst_data[1],1,dst_w*dst_h/2,dst_file);                //U
                fwrite(dst_data[2],1,dst_w*dst_h/2,dst_file);                //V
                break;
            }
            case AV_PIX_FMT_YUV444P:{
                fwrite(dst_data[0],1,dst_w*dst_h,dst_file);                 //Y
                fwrite(dst_data[1],1,dst_w*dst_h,dst_file);                 //U
                fwrite(dst_data[2],1,dst_w*dst_h,dst_file);                 //V
                break;
            }
            case AV_PIX_FMT_YUYV422:{
                fwrite(dst_data[0],1,dst_w*dst_h*2,dst_file);               //Packed
                break;
            }
            case AV_PIX_FMT_RGB24:{
                fwrite(dst_data[0],1,dst_w*dst_h*3,dst_file);               //Packed
                break;
            }
            default:{
                printf("Not Support Output Pixel Format.\n");
                break;
            }
        }
    }
    
    sws_freeContext(img_convert_ctx);
    
    free(temp_buffer);
    fclose(dst_file);
    av_freep(&src_data[0]);
    av_freep(&dst_data[0]);
    
    return 0;
}
////////////////////////////////code_view() end/////////////////////////////////////
