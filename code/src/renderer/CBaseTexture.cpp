//
//  CBaseTexture.cpp
//  OpenGL_Product
//
//  Created by meitu on 2018/1/13.
//  Copyright © 2018年 meitu. All rights reserved.
//

#include "CBaseTexture.hpp"
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace
{
#define MIPMAP_TEXTURE_TEST
}

CBaseTexture::CBaseTexture()
: m_strImagePath("")
, m_uHandleID(0)
, m_uTarget(GL_TEXTURE_2D)
{
}

CBaseTexture::~CBaseTexture()
{
    printf("[CBaseTexture::~CBaseTexture()] handleID[%d] \n", m_uHandleID);
    if (0 != m_uHandleID)
    {
        glDeleteTextures(1, &m_uHandleID);
        m_uHandleID = 0;
    }
}

void CBaseTexture::SetImagePath(const std::string& strImagePath)
{
    m_strImagePath = strImagePath;
}

void CBaseTexture::SetTarget(int eTarget)
{
    m_uTarget = eTarget;
}

void CBaseTexture::LoadTexture(int nLoadType)
{
    switch (nLoadType) {
        case LOAD_TEXTURE_TYPE_BMP:
            LoadBMPCustom(m_strImagePath.c_str());
            break;
        case LOAD_TEXTURE_TYPE_DDS:
            LoadDDS(m_strImagePath.c_str());
            break;
            
        default:
            break;
    }
}

void CBaseTexture::UpdateTexture(const std::string &strPath)
{
    if (strPath.empty())
    {
        printf("[CBaseTexture::UpdateTexture] error: image path is empty.\n");
        return;
    }
    
    //unsigned int width, height;
    int width, height;
    int nChannel;
    // Actual RGB data
    unsigned char * data;
    GLenum format = GL_RGB;
    
    data = stbi_load(strPath.c_str(), &width, &height, &nChannel, 0);
    if (NULL == data)
    {
        printf("[CBaseTexture::UpdateTexture] -=-=-=-=-=-=-=-= loadfile failed \n");
        return;
    }
    
    if (nChannel == 1)
        format = GL_RED;
    else if (nChannel == 3)
        format = GL_RGB;
    else if (nChannel == 4)
        format = GL_RGBA;
    
    glBindTexture(GL_TEXTURE_2D, m_uHandleID);
    
    // Give the image to OpenGL
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
    
    // Generate mipmaps, by the way.
    glGenerateMipmap(GL_TEXTURE_2D);
    
    STBI_FREE(data);
}

void CBaseTexture::UpdateTexture(const void *pData, int width, int height, bool bReGenerate /* = false */)
{
    if (NULL == pData || width <=0 || height <= 0)
    {
        printf("[CBaseTexture::UpdateTexture] warning: parametre is invalid. pData[%p], width[%d], height[%d]\n", pData, width, height);
        return;
    }
    
    glBindTexture(GL_TEXTURE_2D, m_uHandleID);
    
    if (bReGenerate)
    {
        glTexImage2D(m_uTarget, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);
    }
    else
    {
        // update the image to OpenGL
        glTexSubImage2D(m_uTarget, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pData);
    }
#ifdef MIPMAP_TEXTURE_TEST
    glGenerateMipmap(m_uTarget);
#endif
}

unsigned int CBaseTexture::GetHandleID()
{
    return m_uHandleID;
}

unsigned int CBaseTexture::GetTarget()
{
    return m_uTarget;
}

void CBaseTexture::SetHandleID(unsigned int uHandleID)
{
    m_uHandleID = uHandleID;
}

unsigned int CBaseTexture::LoadBMPCustom(const char * imagepath)
{
    // Data read from the header of the BMP file
    unsigned char header[54]; // Each BMP file begins by a 54-bytes header
    unsigned int dataPos;     // Position in the file where the actual data begins
    //unsigned int width, height;
    int width, height;
    unsigned int imageSize;   // = width*height*3
    int nChannel;
    // Actual RGB data
    unsigned char * data;
    GLenum format = GL_RGB;
//#define AA
#ifdef AA
    // Open the file
    FILE * file = fopen(imagepath,"rb");
    if (!file)
    {
        printf("Image could not be openedn");
        return 0;
    }
    
    // 文件一开始是54字节长的文件头，用于标识”这是不是一个BMP文件”、图像大小、像素位等等。
    if ( fread(header, 1, 54, file)!=54 ){ // If not 54 bytes read : problem
        printf("Not a correct BMP filen");
        return false;
    }
    
    // 'B' 'M'开头检测
    if ( header[0]!='B' || header[1]!='M' ){
        printf("Not a correct BMP filen");
        return 0;
    }
    
    // 读取文件中图像大小、数据位置等信息
    // Read ints from the byte array
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);
    
    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos==0)      dataPos=54; // The BMP header is done that way
    
    // 知道了图像的大小，可以为之分配一些内存，把图像读进去
    
    // Create a buffer
    data = new unsigned char [imageSize];
    
    // Read the actual data from the file into the buffer
    fread(data,1,imageSize,file);
    
    //Everything is in memory now, the file can be closed
    fclose(file);
#else
    data = stbi_load(imagepath, &width, &height, &nChannel, 0);
    if (NULL == data)
    {
        printf("[] -=-=-=-=-=-=-=-= loadfile failed \n");
        return 0;
    }
    
    if (nChannel == 1)
        format = GL_RED;
    else if (nChannel == 3)
        format = GL_RGB;
    else if (nChannel == 4)
        format = GL_RGBA;
#endif
    // 创建纹理和创建顶点缓冲差不多：创建一个纹理、绑定、填充、配置。
    glGenTextures(1, &m_uHandleID);
    glBindTexture(GL_TEXTURE_2D, m_uHandleID);
    m_uTarget = GL_TEXTURE_2D;
    
    // Give the image to OpenGL
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    // (目标纹理，level，数据格式（这边是颜色），图片宽，图片长，border（只能0或1)，数据在内存存储格式，数据类型，数据)
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);   // 设置缓存中图片属性，类似数组缓存的glBufferData()函数
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    fprintf(stdout, "use linear in linear\n");
    
    // Nice trilinear filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    
    // When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // When MINifying the image, use a LINEAR blend of two mipmaps, each filtered LINEARLY too
#ifdef MIPMAP_TEXTURE_TEST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // Generate mipmaps, by the way.
#ifdef MIPMAP_TEXTURE_TEST
    glGenerateMipmap(GL_TEXTURE_2D);
#endif
    
    STBI_FREE(data);
    
    return m_uHandleID;
}


#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

unsigned int CBaseTexture::LoadDDS(const char * imagepath)
{
    unsigned char header[124];
    
    FILE *fp;
    
    /* try to open the file */
    fp = fopen(imagepath, "rb");
    if (fp == NULL)
        return 0;
    
    /* verify the type of file */
    char filecode[4];
    fread(filecode, 1, 4, fp);
    if (strncmp(filecode, "DDS ", 4) != 0) {
        fclose(fp);
        return 0;
    }
    
    /* get the surface desc */
    fread(&header, 124, 1, fp);
    
    unsigned int height      = *(unsigned int*)&(header[8 ]);
    unsigned int width         = *(unsigned int*)&(header[12]);
    unsigned int linearSize     = *(unsigned int*)&(header[16]);
    unsigned int mipMapCount = *(unsigned int*)&(header[24]);
    unsigned int fourCC      = *(unsigned int*)&(header[80]);
    
    unsigned char * buffer;
    unsigned int bufsize;
    /* how big is it going to be including all mipmaps? */
    bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
    buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
    fread(buffer, 1, bufsize, fp);
    /* close the file pointer */
    fclose(fp);
    
    // 处理三种格式：DXT1、DXT3和DXT5。我们得把”fourCC”标识转换成OpenGL能识别的值。
    unsigned int components  = (fourCC == FOURCC_DXT1) ? 3 : 4;
    unsigned int format;
    switch(fourCC)
    {
        case FOURCC_DXT1:
            format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            break;
        case FOURCC_DXT3:
            format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            break;
        case FOURCC_DXT5:
            format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
        default:
            free(buffer);
            return 0;
    }
    
    // 纹理
    glGenTextures(1, &m_uHandleID);
    glBindTexture(GL_TEXTURE_2D, m_uHandleID);
    
    fprintf(stdout, "##### gen textrues is %d  \n", m_uHandleID);
    
    // 逐个填充mipmap
    unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    unsigned int offset = 0;
    
    /* load the mipmaps */
    for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
    {
        unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize;
        // (目标纹理，level，数据格式（这边是颜色），图片宽，图片长，border（只能0或1)，数据类型，数据)
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset);
        
        offset += size;
        width  /= 2;
        height /= 2;
    }
    free(buffer);
    
    return m_uHandleID;
}

