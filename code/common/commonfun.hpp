//
//  commonfun.hpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/12.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef commonfun_hpp
#define commonfun_hpp

#include <stdio.h>
#include <algorithm>
#include <glm/glm.hpp>


class CGLRender;

bool LoadDataByFile(const char* path, std::vector<glm::vec3> & out_vertices
             , std::vector<glm::vec2> & out_uvs
             , std::vector<glm::vec3> & out_normals, bool bDDS);

void CopyVertex(std::vector<glm::vec3> &to, std::vector<glm::vec3> &from);
void CopyUV(std::vector<glm::vec2> &to, std::vector<glm::vec2> &from);

int TransGLFWKey2Normal(int eGLFWKeyType);

#endif /* commonfun_hpp */
