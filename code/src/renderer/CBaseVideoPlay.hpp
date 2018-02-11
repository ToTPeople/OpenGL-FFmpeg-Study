//
//  CBaseVideoPlay.hpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/30.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CBaseVideoPlay_hpp
#define CBaseVideoPlay_hpp

#include <stdio.h>
#include <string>
#include <thread>
#include <sys/stat.h>

struct AVFrame;
struct AVCodecContext;

// 回调函数
// 视频解析后，传递frame数据回调函数
typedef void (*FuncVideoCB)(void *pData[], int *width, int *height, bool *bFirstCB, const std::string* strTexture);

class CBaseVideoPlay
{
public:
    enum
    {
        THREAD_MAX_COUNT = 5,                   // 子线程个数
        SHARED_DATA_QUEUE_COUNT = 10,           // 视频解析共享内存各个线程数据保存个数
        COMPRESS_SHARED_DATA_QUEUE_CAP = 30,    // 压缩共享内存数据队列保存个数
    };
    // 视频解析共享内存数据结构
    struct SharedData_s
    {
        int nWidth[THREAD_MAX_COUNT];                                   // decode image width
        int nHeight[THREAD_MAX_COUNT];                                  // decode image height
        int pre[THREAD_MAX_COUNT];                                      // the head of queue
        int back[THREAD_MAX_COUNT];                                     // the tail of queue
        bool bFirstCB[THREAD_MAX_COUNT][SHARED_DATA_QUEUE_COUNT];       // is the first decode data
        void* pData[THREAD_MAX_COUNT][SHARED_DATA_QUEUE_COUNT];         // decode data
    };
    // 截屏压缩共享内存数据结构
    struct CompressSharedData_s
    {
        int width;                                                      // image width
        int height;                                                     // image height
        bool is_end;                                                    // is last image to compress
        bool is_first;                                                  // is first image to compress
        int head;                                                       // the header location of queue
        int tail;                                                       // the tail location to queue
        int idx;                                                        // index
        uint8_t *img_data[COMPRESS_SHARED_DATA_QUEUE_CAP];              // the image rgb data
    };
    
public:
    CBaseVideoPlay(int eOpType, const std::string& strVideoPath);
    virtual ~CBaseVideoPlay();
    
    // 解析视频接口
    virtual void Run(const std::string& strTexture);
    // 压缩数据为视频文件
    void Record(uint8_t* pImgData, int nWidth, int nHeight, bool bEnd);
    
    // 保存为bmp操作时，设置保存路径
    void SetSavePath(const std::string& strSavePath);
    // 播放时，设置回调函数
    void SetVideoCbFunc(FuncVideoCB pFuncVideoCB);
    // 已创建线程个数
    int GetCreateThreadCnt();
    
protected:
    // 检测数据合法性
    bool CheckValid();
    // 保存BMP文件的函数
    void SaveAsBMP(AVFrame *pFrameRGB, int width, int height, int index);
    
    // 发送队列数据，更新绘制
    bool SendData();
    
    // 启动子线程解析视频数据
    static void Play(const char* filename, int* cur_thread_cnt, int thread_idx);
    //
    static void CompressRGB();
    
private:
    int             m_eOpType;
    std::string     m_strVideoPath;     // 播放的文件路径名称
    std::string     m_strSavePath;      // 保存图片时：保存的路径；
    FuncVideoCB     m_pFuncVideoCB;     // 回调函数
    bool            m_bInit;            // 视频编、解码是否初始化
    // 子线程解析视频相关 start
    int             m_nThreadCnt;       // 已创建的子线程个数
    int             m_nCurThreadCnt;    // 当前运行中的线程个数
    int             m_shmid;            // 解析共享内存文件描述符
    std::string     m_strTexture[THREAD_MAX_COUNT];     // 线程解析后数据绘制的目标纹理
    struct stat     m_statBuf;          // 文件状态相关信息
    SharedData_s    *m_pSharedData;     // 共享内存映射后起始地址
    // 子线程解析视频相关 end
    // 子线程压缩相关 start
    bool            m_is_recording;
    int             idx;                // 第几段RGB图片进行视频压缩
    int             m_compress_shmid;   // 压缩共享内存文件描述符
    CompressSharedData_s    *m_pCompressSharedData;
    // 子线程压缩相关 end
};

#endif /* CBaseVideoPlay_hpp */
