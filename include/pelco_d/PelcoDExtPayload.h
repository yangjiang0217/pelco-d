/**
 * @file    PelcoDExtPayload.h
 * @version 1.0.0
 * @date    2026-09-18
 * @author  yangjiang
 * @brief   云台扩展协议帧载荷类
 * @details 扩展帧载荷继承标准帧载荷 CPelcoDPayload，表达"附加协议基于标准 Pelco-D 协议"的
 *          继承关系；标准帧为 7 字节（数据 1/2），附加协议位置帧（0xF5/0xF9）为 9 字节，
 *          数据 3/4（俯仰位置高/低字节）在此类扩展。
 */

#pragma once

#include "pelco_d/PelcoDPayload.h"

namespace pelco_d
{

/**
 * @class  CPelcoDExtPayload
 * @brief  云台扩展协议帧载荷（基于标准帧载荷）
 */
class PELCO_D_API CPelcoDExtPayload : public CPelcoDPayload
{
public:
    // 构造函数：调用基类构造后清零数据 3/4
    CPelcoDExtPayload()
        : m_byData3(0)
        , m_byData4(0)
    {
    }

public:
    uint8_t m_byData3;  // 数据 3（俯仰位置高字节）
    uint8_t m_byData4;  // 数据 4（俯仰位置低字节）
};

}  // namespace pelco_d
