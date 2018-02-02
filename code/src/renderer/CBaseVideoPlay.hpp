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
#include <mutex>
#include <thread>

struct AVFrame;

// 回调函数
// 视频解析后，传递frame数据回调函数
typedef void (*FuncVideoCB)(const void *pData, int width, int height, bool bFirstCB, const std::string& strTexture);

class CBaseVideoPlay
{
public:
    enum
    {
        ARRAR_MAX = 3003,
        THREAD_MAX_COUNT = 5,
    };
    struct SharedData_s
    {
        void* pData;
        int nWidth;
        int nHeight;
        bool bFirstCB;
        std::string strTexture;
    };
    
public:
    CBaseVideoPlay(int eOpType, const std::string& strVideoPath);
    ~CBaseVideoPlay();
    
    static void Play(const char* filename, const std::string& strTexture);
    virtual void Run(const std::string& strTexture);
    // 保存为bmp操作时，设置保存路径
    void SetSavePath(const std::string& strSavePath);
    // 播放时，设置回调函数
    void SetVideoCbFunc(FuncVideoCB pFuncVideoCB);
    
    // 设置公共队列数据
    void SetSharedData(void *pData, int width, int height, bool bFirstCB, int nSize, const std::string& strTexture);
    // 更新减少子线程数
    void DecreaceThreadCount();
    
protected:
    // 检测数据合法性
    bool CheckValid();
    // 保存BMP文件的函数
    void SaveAsBMP(AVFrame *pFrameRGB, int width, int height, int index);
    
    // 发送队列数据，更新绘制
    bool SendData();
    
private:
    int             m_eOpType;
    std::string     m_strVideoPath;     // 播放的文件路径名称
    std::string     m_strSavePath;      // 保存图片时：保存的路径；
    FuncVideoCB     m_pFuncVideoCB;     // 回调函数
    bool            m_bInit;            // 视频解码是否初始化
    std::mutex      m_mutex;            // 互斥锁
    int             m_nPre;             // 队列头
    int             m_nBack;            // 队列尾
    SharedData_s    m_sSharedData[ARRAR_MAX];   // 公共数据队列
    int             m_nThreadCnt;       // 运行中子进程个数
};

#endif /* CBaseVideoPlay_hpp */
