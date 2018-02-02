//
//  common_define.h
//  OpenGL_Product
//
//  Created by meitu on 2018/1/14.
//  Copyright © 2018年 meitu. All rights reserved.
//

#ifndef common_define_h
#define common_define_h

#include <glm/glm.hpp>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 测试宏
#define SQUARE_TEST                           // 1: 正方形测试  0:立方体测试
//#define SCALE_OPERATOR                        // 1: 缩放操作 0: 拖拽操作

#define TEST_2D_TRANSLATION                     // 世界坐标系

//#define TEST_FBO                              // FBO测试
//#define TEST_BLEND                              // 颜色混合测试
//#define TEST_STENCIL                            // 模板测试（边框测试）

#define VIDEO_RUN_TEST                          // 视频测试
#define VIDEO_ASYNCHRONOUS_TEST                 // 视频异步播放测试

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MIN_Z_DEPTH                  (-1.1f);   // 设置一个z轴最小值，因为目前这个屏幕显示-1,1范围
#define MAX_Z_DEPTH                  (1.1f);    // 设置一个z轴最大值，因为目前这个屏幕显示-1,1范围

const int g_nDefaultMaxLineLength = 128;

const char* const kszOutlineFragShader = "./outline_frag_shader";       // 边框
// tmp
const char* const kszScreenVtxShader = "./screen_vertex_shader";
const char* const kszScreenFragShader = "./screen_frag_shader";

// Render type
enum RendererType
{
    RENDERER_TYPE_BASE = 0x0,
    RENDERER_TYPE_SQUARE,               // 方形渲染
    RENDERER_TYPE_CUBE,                 // 立方渲染
    RENDERER_TYPE_SCREEN,               // 2D全屏
};

// Shape type
enum ShapeType
{
    SHAPE_TYPE_BASE = 0x0,
    SHAPE_TYPE_TRIANGLE,
    SHAPE_TYPE_SCREEN,
};

enum OperateType
{
    OP_TYPE_DRAG = 0x0,         // 拖拽
    OP_TYPE_SCALE,              // 缩放
};

enum ArrowKeyType
{
    ARROW_KEY_TYPE_BASE = 0x10,
    ARROW_KEY_TYPE_LEFT,
    ARROW_KEY_TYPE_RIGHT,
    ARROW_KEY_TYPE_UP,
    ARROW_KEY_TYPE_DOWN,
    ARROW_KEY_TYPE_MAX,
};

enum LoadTextureType
{
    LOAD_TEXTURE_TYPE_BMP = 0x0,        // 加载.BMP图片
    LOAD_TEXTURE_TYPE_DDS,              // 加载.DDS图片
};

enum VideoOpType
{
    VIDEO_OP_TYPE_SAVE_BMP = 0x10,      // 解析并保存成bmp图片
    VIDEO_OP_TYPE_PLAY,                 // 解析并播放
    VIDEO_OP_TYPE_MAX,
};

// CPU->GPU数据整合
struct PackedVertex
{
    glm::vec3 position;
    glm::vec2 uv;
    bool operator<(const PackedVertex that) const
    {
        return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
    };
};

// 数据路径
const char* const kszScreenDataPath = "./screen_data";
const char* const kszSquareDataPath = "./square_data";
const char* const kszCubeDataPath = "./cube_data";
// 着色器路径
const char* const kszVertexShader = "./vertex_shader";
const char* const kszFragementShader = "./frag_shader";
const char* const kszSquareVertexShader = "./square_vertex_shader";
const char* const kszSquareFragementShader = "./square_frag_shader";
// 纹理图片路径
#ifndef TEST_BLEND
const char* const kszSquareImagePath = "./441H.jpg";//"./uvtemplate.bmp";
#else
const char* const kszSquareImagePath = "window.png";//"grass.png";//"./441H.jpg";//"./uvtemplate.bmp";
#endif
const char* const kszCubeImagePath = "./uvmap.DDS";

const char* const kszVideoPlayImagePath = "./pic/black.jpg";

#endif /* common_define_h */
