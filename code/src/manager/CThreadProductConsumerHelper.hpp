//
//  CThreadProductConsumerHelper.hpp
//  OpenGL_Product
//
//  Created by meitu on 2018/2/8.
//  Copyright © 2018年 meitu. All rights reserved.
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
    // 生产者
    void BufferFullWait();
    void BufferNotEmptySignal();
    // 消费者
    void BufferEmptyWait();
    void BufferNotFullSignal();
    // 互斥锁
    void BufferLock();
    void BufferUnlock();
    
private:
    CThreadProductConsumerHelper();
    
private:
    static CThreadProductConsumerHelper*   m_pInstance;
    pthread_mutex_t                 m_buffer_mutex;
    pthread_cond_t                  m_buffer_not_full_cond;
    pthread_cond_t                  m_buffer_not_empty_cond;
};

#define g_pThreadProductConsumerHelper (CThreadProductConsumerHelper::GetInstance())

#endif /* CThreadProductConsumerHelper_hpp */
