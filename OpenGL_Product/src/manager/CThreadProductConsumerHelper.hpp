//
//  CThreadProductConsumerHelper.hpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/2/8.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CThreadProductConsumerHelper_hpp
#define CThreadProductConsumerHelper_hpp

#include <stdio.h>
#include <pthread.h>

class CThreadProductConsumerHelper
{
public:
    static CThreadProductConsumerHelper* GetInstance();
    
public:
    // 视频解析
    // 生产者
    void BufferFullWait();
    void BufferNotEmptySignal();
    // 消费者
    void BufferEmptyWait();
    void BufferNotFullSignal();
    // 互斥锁
    void BufferLock();
    void BufferUnlock();
    
    // 数据压缩
    // 生产者
    void CompressBufferFullWait();
    void CompressBufferNotEmptySignal();
    // 消费者
    void CompressBufferEmptyWait();
    void CompressBufferNotFullSignal();
    // 互斥锁
    void CompressBufferLock();
    void CompressBufferUnlock();
    
private:
    CThreadProductConsumerHelper();
    
private:
    static CThreadProductConsumerHelper*   m_pInstance;
    // 视频解析
    pthread_mutex_t                 m_buffer_mutex;
    pthread_cond_t                  m_buffer_not_full_cond;
    pthread_cond_t                  m_buffer_not_empty_cond;
    // 数据压缩
    pthread_mutex_t                 m_compress_buffer_mutex;
    pthread_cond_t                  m_compress_buffer_not_full_cond;
    pthread_cond_t                  m_compress_buffer_not_empty_cond;
};

#define g_pThreadProductConsumerHelper (CThreadProductConsumerHelper::GetInstance())

#endif /* CThreadProductConsumerHelper_hpp */
