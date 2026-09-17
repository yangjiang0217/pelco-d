/**
 * @file    PelcoDPayload.h
 * @version 1.0.0
 * @date    2026-09-17
 * @author  yangjiang
 * @brief   Pelco-D 帧载荷类
 * @details 定义 Pelco-D 帧同步字节、帧长度宏与帧载荷类 CPelcoDPayload。
 */

#pragma once

#include <cstdint>

// 动态库导出宏：构建动态库时导出符号（PELCO_D_BUILD_SHARED 由 CMake 定义）；
// 使用方链接动态库可定义 PELCO_D_USE_SHARED 导入符号；静态链接无需定义
#if defined(_WIN32)
#  if defined(PELCO_D_BUILD_SHARED)
#    define PELCO_D_API __declspec(dllexport)
#  elif defined(PELCO_D_USE_SHARED)
#    define PELCO_D_API __declspec(dllimport)
#  else
#    define PELCO_D_API
#  endif
#else
#  define PELCO_D_API
#endif

// 帧同步字节
#define DEF_PELCO_D_SYNC_BYTE   0xFF
// 帧总长度（同步字节 + 地址 + 命令 2 字节 + 数据 2 字节 + 校验和）
#define DEF_PELCO_D_FRAME_LEN   7

namespace pelco_d
{

/**
 * @class  CPelcoDPayload
 * @brief  Pelco-D 帧载荷（不含同步字节与校验和）
 */
class PELCO_D_API CPelcoDPayload
{
public:
    // 构造函数：全部成员清零
    CPelcoDPayload()
        : m_byAddr(0)
        , m_byCmd1(0)
        , m_byCmd2(0)
        , m_byData1(0)
        , m_byData2(0)
    {
    }

public:
    uint8_t m_byAddr;   // 设备地址
    uint8_t m_byCmd1;   // 命令字节 1
    uint8_t m_byCmd2;   // 命令字节 2
    uint8_t m_byData1;  // 数据 1（水平速度，0x00~0x3F）
    uint8_t m_byData2;  // 数据 2（垂直速度，0x00~0x3F）
};

}  // namespace pelco_d
