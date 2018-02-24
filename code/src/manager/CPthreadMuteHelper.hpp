//
//  CPthreadMuteHelper.hpp
//  OpenGL_Product
//
//  Created by lifushan on 2018/2/8.
//  Copyright © 2018年 lifs. All rights reserved.
//

#ifndef CPthreadMuteHelper_hpp
#define CPthreadMuteHelper_hpp

#include <stdio.h>
#include <pthread.h>

class CPthreadMuteHelper
{
public:
    static CPthreadMuteHelper* GetInstance();
    
public:
    // 互斥锁：锁定、解锁
    void LockMutex();
    void UnlockMutex();
    
private:
    CPthreadMuteHelper();
    
private:
    static CPthreadMuteHelper* m_pInstance;
    pthread_mutex_t            m_mutexLock;
};

#define g_pPthreadMuteHelper (CPthreadMuteHelper::GetInstance())

#endif /* CPthreadMuteHelper_hpp */
