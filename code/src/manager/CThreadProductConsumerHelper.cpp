//
//  CThreadProductConsumerHelper.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/2/8.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CThreadProductConsumerHelper.hpp"

CThreadProductConsumerHelper* CThreadProductConsumerHelper::m_pInstance = NULL;

CThreadProductConsumerHelper* CThreadProductConsumerHelper::GetInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CThreadProductConsumerHelper();
    }
    
    return m_pInstance;
}

void CThreadProductConsumerHelper::BufferFullWait()
{
    pthread_cond_wait(&m_buffer_not_full_cond, &m_buffer_mutex);
}

void CThreadProductConsumerHelper::BufferNotEmptySignal()
{
    //pthread_cond_signal(&m_buffer_not_empty_cond);
    pthread_cond_broadcast(&m_buffer_not_empty_cond);
}

void CThreadProductConsumerHelper::BufferEmptyWait()
{
    pthread_cond_wait(&m_buffer_not_empty_cond, &m_buffer_mutex);
}

void CThreadProductConsumerHelper::BufferNotFullSignal()
{
    //pthread_cond_signal(&m_buffer_not_full_cond);
    pthread_cond_broadcast(&m_buffer_not_full_cond);
}

void CThreadProductConsumerHelper::BufferLock()
{
    pthread_mutex_lock(&m_buffer_mutex);
}

void CThreadProductConsumerHelper::BufferUnlock()
{
    pthread_mutex_unlock(&m_buffer_mutex);
}

CThreadProductConsumerHelper::CThreadProductConsumerHelper()
{
    pthread_mutex_init(&m_buffer_mutex, NULL);
    pthread_cond_init(&m_buffer_not_full_cond, NULL);
    pthread_cond_init(&m_buffer_not_empty_cond, NULL);
}
