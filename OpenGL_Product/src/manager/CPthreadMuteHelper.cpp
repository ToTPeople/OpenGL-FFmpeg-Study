//
//  CPthreadMuteHelper.cpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/2/8.
//  Copyright © 2018年 lifs. All rights reserved.
//

#include "CPthreadMuteHelper.hpp"

CPthreadMuteHelper* CPthreadMuteHelper::m_pInstance = NULL;

CPthreadMuteHelper* CPthreadMuteHelper::GetInstance()
{
    if (NULL == m_pInstance) {
        m_pInstance = new CPthreadMuteHelper();
    }
    
    return m_pInstance;
}

void CPthreadMuteHelper::LockMutex()
{
    pthread_mutex_lock(&m_mutexLock);
}

void CPthreadMuteHelper::UnlockMutex()
{
    pthread_mutex_unlock(&m_mutexLock);
}

CPthreadMuteHelper::CPthreadMuteHelper()
{
    pthread_mutex_init(&m_mutexLock, NULL);
}
