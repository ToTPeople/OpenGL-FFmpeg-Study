//
//  CThreadProductConsumerHelper.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/2/8.
//  Copyright © 2018年 lifs. All rights reserved.
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

// 压缩数据
void CThreadProductConsumerHelper::CompressBufferFullWait()
{
    pthread_cond_wait(&m_compress_buffer_not_full_cond, &m_compress_buffer_mutex);
}

void CThreadProductConsumerHelper::CompressBufferNotEmptySignal()
{
    pthread_cond_signal(&m_compress_buffer_not_empty_cond);
}

void CThreadProductConsumerHelper::CompressBufferEmptyWait()
{
    pthread_cond_wait(&m_compress_buffer_not_empty_cond, &m_compress_buffer_mutex);
}

void CThreadProductConsumerHelper::CompressBufferNotFullSignal()
{
    pthread_cond_signal(&m_compress_buffer_not_full_cond);
}

void CThreadProductConsumerHelper::CompressBufferLock()
{
    pthread_mutex_lock(&m_compress_buffer_mutex);
}

void CThreadProductConsumerHelper::CompressBufferUnlock()
{
    pthread_mutex_unlock(&m_compress_buffer_mutex);
}
// 压缩数据

CThreadProductConsumerHelper::CThreadProductConsumerHelper()
{
    pthread_mutex_init(&m_buffer_mutex, NULL);
    pthread_cond_init(&m_buffer_not_full_cond, NULL);
    pthread_cond_init(&m_buffer_not_empty_cond, NULL);
    //
    pthread_mutex_init(&m_compress_buffer_mutex, NULL);
    pthread_cond_init(&m_compress_buffer_not_full_cond, NULL);
    pthread_cond_init(&m_compress_buffer_not_empty_cond, NULL);
}
