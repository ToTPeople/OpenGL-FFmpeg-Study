//
//  CShapeDataLoadHelper.hpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/19.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef CShapeDataLoadHelper_hpp
#define CShapeDataLoadHelper_hpp

#include <stdio.h>
#include <cstring>
#include <string>
#include <vector>

class CShapeDataObj;

class CShapeDataLoadHelper
{
public:
    static CShapeDataLoadHelper* GetInstance();
    
public:
    bool LoadDataByFile(const char* path);
    
protected:
    void ClearData();
    
private:
    CShapeDataLoadHelper();
    
private:
    static CShapeDataLoadHelper* m_pInstance;
    
    std::vector<CShapeDataObj*> m_vecShapeDataObj;
};

#define g_pShapeDataLoadHelper (CShapeDataLoadHelper::GetInstance())

#endif /* CShapeDataLoadHelper_hpp */
