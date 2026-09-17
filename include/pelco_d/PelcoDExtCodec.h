/**
 * @file    PelcoDExtCodec.h
 * @version 1.0.0
 * @date    2026-09-18
 * @author  yangjiang
 * @brief   云台扩展协议（PTZ Extended Protocol V0.1）组帧拆帧类
 * @details 适配基于 Pelco-D 的云台附加协议：
 *          1. 帧格式为 7 字节（同步+地址+命令 2 字节+数据 2 字节+校验和），
 *             位置类指令（0xF5 回复 / 0xF9 写位置）为 9 字节（数据 4 字节）；
 *          2. 帧长由命令字节 2（COMD2）判定：0xF5/0xF9 为 9 字节，其余 7 字节；
 *          3. 校验和沿用 Pelco-D 算法（地址+命令+数据累加，取低 8 位）。
 */

#pragma once

#include <cstdint>
#include <deque>
#include <vector>

#include "pelco_d/PelcoDCodec.h"
#include "pelco_d/PelcoDExtPayload.h"

// 附加协议帧长（同步+地址+命令 2 字节+数据+校验和）
#define DEF_PELCO_D_EXT_FRAME_LEN_MIN   7   // 最短帧长（数据 2 字节）
#define DEF_PELCO_D_EXT_FRAME_LEN_MAX   9   // 最长帧长（数据 4 字节）

// 附加协议命令（m_byCmd1 = 0x00，命令值赋给 m_byCmd2）
#define DEF_PELCO_D_EXT_CMD_QUERY_POS      0xF7   // 读指令：同时读取水平/俯仰位置（主机 7 字节）
#define DEF_PELCO_D_EXT_CMD_POS_REPLY      0xF5   // 云台回复位置（9 字节：水平高/低、俯仰高/低）
#define DEF_PELCO_D_EXT_CMD_SET_POS        0xF9   // 写指令：向指定位置以设定速度直线运行（9 字节）
#define DEF_PELCO_D_EXT_CMD_PRESET_SPEED   0x85   // 写指令：预置点间速度设定（速度放数据二）
#define DEF_PELCO_D_EXT_CMD_CALIB_ZERO     0x87   // 写指令：水平/俯仰运行至物理零点校零

namespace pelco_d
{

// 云台扩展协议组帧拆帧工具类
class PELCO_D_API CPelcoDExtCodec : public CPelcoDCodec
{
public:

    // 引入基类拆帧接口：标准帧（CPelcoDPayload）与扩展帧（CPelcoDExtPayload）均可解析
    using CPelcoDCodec::PopFrame;
    using CPelcoDCodec::PopFrames;

    /**
     * @brief 命令组帧：按地址、命令、数据组装 7 字节帧
     * @param[in]  byAddr   设备地址
     * @param[in]  byCmd1   命令字节 1
     * @param[in]  byCmd2   命令字节 2
     * @param[in]  byData1  数据一
     * @param[in]  byData2  数据二
     * @param[out] refVecFrame 完整帧（std::vector<uint8_t> 出参）
     * @return true 成功
     */
    static bool BuildCommandFrame(uint8_t byAddr, uint8_t byCmd1, uint8_t byCmd2,
                                  uint8_t byData1, uint8_t byData2,
                                  std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 读位置帧：同时读取当前水平/俯仰位置（COMD2 = 0xF7）
     * @param[in]  byAddr   设备地址
     * @param[out] refVecFrame 完整帧（std::vector<uint8_t> 出参）
     * @return true 成功
     */
    static bool BuildQueryPositionFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 写位置帧：向指定位置以设定速度直线运行（COMD2 = 0xF9）
     * @param[in]  byAddr   设备地址
     * @param[in]  wPanPos  水平位置（0~35999，高字节在前）
     * @param[in]  wTiltPos 俯仰位置（0~9000 或 27000~35999，高字节在前）
     * @param[out] refVecFrame 完整帧（std::vector<uint8_t> 出参）
     * @return true 成功
     */
    static bool BuildSetPositionFrame(uint8_t byAddr, uint16_t wPanPos, uint16_t wTiltPos,
                                      std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 预置点间速度设定帧（COMD2 = 0x85）
     * @param[in]  byAddr   设备地址
     * @param[in]  bySpeed  巡航速度
     * @param[out] refVecFrame 完整帧（std::vector<uint8_t> 出参）
     * @return true 成功
     */
    static bool BuildSetPresetSpeedFrame(uint8_t byAddr, uint8_t bySpeed,
                                         std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 物理校零帧：水平/俯仰运行至物理零点（COMD2 = 0x87）
     * @param[in]  byAddr   设备地址
     * @param[out] refVecFrame 完整帧（std::vector<uint8_t> 出参）
     * @return true 成功
     */
    static bool BuildCalibrateZeroFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 拆帧：从接收缓冲区解析一帧完整数据
     * @param[in,out] refDequeBuffer 接收缓冲区，解析成功的帧会从缓冲区移除
     * @param[out]    refPayload       解析出的帧载荷
     * @return true 解析成功；false 无完整帧（缓冲区为空或剩余数据不足一帧）
     * @note  帧长按 COMD2 判定：0xF5/0xF9 为 9 字节，其余 7 字节；
     *        缓冲区中混入的非法数据（非同步字节、校验失败）会被安全跳过；
     *        数据不足一帧时保留缓冲区内容，等待更多数据后再次调用
     */
    static bool PopFrame(std::deque<uint8_t>& refDequeBuffer, CPelcoDExtPayload& refPayload);

    /**
     * @brief 拆帧：从接收缓冲区解析出全部完整帧
     * @param[in,out] refDequeBuffer 接收缓冲区，解析成功的帧会从缓冲区移除
     * @param[out]    refVecFrames   解析出的全部帧载荷
     * @return 解析出的帧数量
     */
    static uint32_t PopFrames(std::deque<uint8_t>& refDequeBuffer, std::vector<CPelcoDExtPayload>& refVecFrames);

    /**
     * @brief 读取水平位置（回复帧 0xF5：数据一<<8 + 数据二）
     * @param[in] refPayload 帧载荷
     * @return 水平位置（0~35999）
     */
    static uint16_t GetPanPosition(const CPelcoDExtPayload& refPayload);

    /**
     * @brief 读取俯仰位置（回复帧 0xF5：数据三<<8 + 数据四）
     * @param[in] refPayload 帧载荷
     * @return 俯仰位置（0~9000 或 27000~35999）
     */
    static uint16_t GetTiltPosition(const CPelcoDExtPayload& refPayload);

private:
    /**
     * @brief 扩展帧长判定：0xF5/0xF9 为 9 字节，其余 7 字节
     * @param[in] refDequeBuffer 接收缓冲区
     * @return 帧长；数据不足最短帧长（7 字节）时返回 0 表示无法判定
     */
    static uint32_t GetExtFrameLen(const std::deque<uint8_t>& refDequeBuffer);

};

}  // namespace pelco_d
