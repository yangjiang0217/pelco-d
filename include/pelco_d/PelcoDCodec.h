/**
 * @file    PelcoDCodec.h
 * @version 1.0.0
 * @date    2026-09-17
 * @author  yangjiang
 * @brief   Pelco-D 协议组帧拆帧类
 * @details 提供 Pelco-D 协议帧的组帧(BuildFrame)、拆帧(PopFrame/PopFrames)与常用操作便捷接口；
 *          拆帧从 std::deque<uint8_t> 接收缓冲区解析完整帧，解析成功后从缓冲区移除，
 *          混入的非法数据会被安全跳过，不足一帧的尾部数据保留等待后续数据。
 */

#pragma once

#include <cstdint>
#include <deque>
#include <vector>

#include "pelco_d/PelcoDPayload.h"

// Pelco-D 命令 1 位定义（m_byCmd1，可按位组合）
#define DEF_PELCO_D_CMD1_SENSE          0x80   // 感测位（与相机关闭位配合：SENSE=1 且关闭位=0 时开启相机）
#define DEF_PELCO_D_CMD1_AUTO_SCAN      0x10   // 自动/手动扫描
#define DEF_PELCO_D_CMD1_CAMERA_OFF     0x08   // 相机关闭（1 关闭，0 开启）
#define DEF_PELCO_D_CMD1_IRIS_CLOSE     0x04   // 关光圈
#define DEF_PELCO_D_CMD1_IRIS_OPEN      0x02   // 开光圈
#define DEF_PELCO_D_CMD1_FOCUS_NEAR     0x01   // 近焦

// Pelco-D 命令 2 位定义（m_byCmd2，可按位组合）
#define DEF_PELCO_D_CMD2_FOCUS_FAR      0x80   // 远焦
#define DEF_PELCO_D_CMD2_ZOOM_WIDE      0x40   // 缩小
#define DEF_PELCO_D_CMD2_ZOOM_TELE      0x20   // 放大
#define DEF_PELCO_D_CMD2_TILT_DOWN      0x10   // 下移
#define DEF_PELCO_D_CMD2_TILT_UP        0x08   // 上移
#define DEF_PELCO_D_CMD2_PAN_LEFT       0x04   // 左移
#define DEF_PELCO_D_CMD2_PAN_RIGHT      0x02   // 右移

// 扩展命令（m_byCmd1 = 0x00，直接赋给 m_byCmd2）
#define DEF_PELCO_D_CMD_PRESET_SET       0x03   // 设置预置位（预置位号放 m_byData2）
#define DEF_PELCO_D_CMD_PRESET_CLEAR     0x05   // 清除预置位（预置位号放 m_byData2）
#define DEF_PELCO_D_CMD_PRESET_CALL      0x07   // 调用预置位（预置位号放 m_byData2）
#define DEF_PELCO_D_CMD_AUX_SET          0x09   // 打开辅助开关（辅助号 1~8 放 m_byData2）
#define DEF_PELCO_D_CMD_AUX_CLEAR        0x0B   // 关闭辅助开关（辅助号 1~8 放 m_byData2）
#define DEF_PELCO_D_CMD_REMOTE_RESET     0x0F   // 远程复位
#define DEF_PELCO_D_CMD_STOP_PATTERN     0x21   // 停止巡迹
#define DEF_PELCO_D_CMD_RUN_PATTERN      0x23   // 运行巡迹（巡迹号放 m_byData2）
#define DEF_PELCO_D_CMD_SET_ZOOM_SPEED   0x25   // 设置变焦速度（0~3 放 m_byData2）
#define DEF_PELCO_D_CMD_SET_FOCUS_SPEED  0x27   // 设置聚焦速度（0~3 放 m_byData2）
#define DEF_PELCO_D_CMD_RESET_DEFAULTS   0x29   // 相机恢复默认设置
#define DEF_PELCO_D_CMD_SET_ZERO         0x49   // 设置零位
#define DEF_PELCO_D_CMD_SET_PAN          0x4B   // 设置水平位置（m_byData1=高字节，m_byData2=低字节）
#define DEF_PELCO_D_CMD_SET_TILT         0x4D   // 设置垂直位置（m_byData1=高字节，m_byData2=低字节）
#define DEF_PELCO_D_CMD_SET_ZOOM         0x4F   // 设置变焦位置（m_byData1=高字节，m_byData2=低字节）
#define DEF_PELCO_D_CMD_QUERY_PAN        0x51   // 查询水平位置
#define DEF_PELCO_D_CMD_QUERY_TILT       0x53   // 查询垂直位置
#define DEF_PELCO_D_CMD_QUERY_ZOOM       0x55   // 查询变焦位置
#define DEF_PELCO_D_CMD_QUERY_VERSION    0x73   // 查询版本信息

namespace pelco_d
{

// Pelco-D 组帧拆帧工具类
class PELCO_D_API CPelcoDCodec
{
public:
    /**
     * @brief 组帧：将帧载荷组装为完整 Pelco-D 帧
     * @param[in]  refPayload    帧载荷
     * @param[out] szFrame   完整帧缓冲区，长度须不小于 DEF_PELCO_D_FRAME_LEN
     * @return true 成功
     */
    static bool BuildFrame(const CPelcoDPayload& refPayload, std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 命令组帧：按地址、命令、数据组装标准帧（7 字节）
     * @param[in]  byAddr     设备地址
     * @param[in]  byCmd1     命令字节 1
     * @param[in]  byCmd2     命令字节 2
     * @param[in]  byData1    数据 1
     * @param[in]  byData2    数据 2
     * @param[out] refVecFrame 完整帧（std::vector<uint8_t> 出参）
     * @return true 成功
     */
    static bool BuildCommandFrame(uint8_t byAddr, uint8_t byCmd1, uint8_t byCmd2,
                                  uint8_t byData1, uint8_t byData2,
                                  std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 云台移动帧（速度 0x00~0x3F）
     * @param[in]  byAddr      设备地址
     * @param[in]  byPanSpeed  水平速度（左/右移，0x00~0x3F）
     * @param[in]  byTiltSpeed 垂直速度（上/下移，0x00~0x3F）
     * @param[out] szFrame   完整帧缓冲区，长度须不小于 DEF_PELCO_D_FRAME_LEN
     * @return true 成功
     */
    static bool BuildMoveUpFrame(uint8_t byAddr, uint8_t byTiltSpeed, std::vector<uint8_t>& refVecFrame);
    static bool BuildMoveDownFrame(uint8_t byAddr, uint8_t byTiltSpeed, std::vector<uint8_t>& refVecFrame);
    static bool BuildMoveLeftFrame(uint8_t byAddr, uint8_t byPanSpeed, std::vector<uint8_t>& refVecFrame);
    static bool BuildMoveRightFrame(uint8_t byAddr, uint8_t byPanSpeed, std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 停止云台移动帧
     * @param[in]  byAddr     设备地址
     * @param[out] refVecFrame 完整帧（std::vector<uint8_t> 出参）
     * @return true 成功
     */
    static bool BuildStopFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 镜头控制帧（变焦/聚焦/光圈，无速度参数）
     * @param[in]  byAddr     设备地址
     * @param[out] refVecFrame 完整帧（std::vector<uint8_t> 出参）
     * @return true 成功
     */
    static bool BuildZoomInFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);
    static bool BuildZoomOutFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);
    static bool BuildFocusFarFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);
    static bool BuildFocusNearFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);
    static bool BuildIrisOpenFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);
    static bool BuildIrisCloseFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 预置位控制帧
     * @param[in]  byAddr     设备地址
     * @param[in]  byPresetId 预置位号（1~255，0 无效）
     * @param[out] refVecFrame 完整帧（std::vector<uint8_t> 出参）
     * @return true 成功
     */
    static bool BuildSetPresetFrame(uint8_t byAddr, uint8_t byPresetId, std::vector<uint8_t>& refVecFrame);
    static bool BuildClearPresetFrame(uint8_t byAddr, uint8_t byPresetId, std::vector<uint8_t>& refVecFrame);
    static bool BuildCallPresetFrame(uint8_t byAddr, uint8_t byPresetId, std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 辅助开关控制帧
     * @param[in]  byAddr    设备地址
     * @param[in]  byAuxId   辅助号（1~8）
     * @param[out] szFrame 完整帧缓冲区，长度须不小于 DEF_PELCO_D_FRAME_LEN
     * @return true 成功
     */
    static bool BuildSetAuxFrame(uint8_t byAddr, uint8_t byAuxId, std::vector<uint8_t>& refVecFrame);
    static bool BuildClearAuxFrame(uint8_t byAddr, uint8_t byAuxId, std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 远程复位帧
     * @param[in]  byAddr     设备地址
     * @param[out] refVecFrame 完整帧（std::vector<uint8_t> 出参）
     * @return true 成功
     */
    static bool BuildRemoteResetFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 位置查询帧（查询水平/垂直/变焦位置）
     * @param[in]  byAddr     设备地址
     * @param[out] refVecFrame 完整帧（std::vector<uint8_t> 出参）
     * @return true 成功
     */
    static bool BuildQueryPanFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);
    static bool BuildQueryTiltFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);
    static bool BuildQueryZoomFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);

    /**
     * @brief 拆帧：从接收缓冲区解析一帧完整数据
     * @param[in,out] refDequeBuffer 接收缓冲区，解析成功的帧会从缓冲区移除
     * @param[out]    refPayload       解析出的帧载荷
     * @return true 解析成功；false 无完整帧（缓冲区为空或剩余数据不足一帧）
     * @note  缓冲区中混入的非法数据（非同步字节、校验失败）会被安全跳过；
     *        数据不足一帧时保留缓冲区内容，等待更多数据后再次调用
     */
    static bool PopFrame(std::deque<uint8_t>& refDequeBuffer, CPelcoDPayload& refPayload);

    /**
     * @brief 拆帧：从接收缓冲区解析出全部完整帧
     * @param[in,out] refDequeBuffer 接收缓冲区，解析成功的帧会从缓冲区移除
     * @param[out]    refVecFrames   解析出的全部帧载荷
     * @return 解析出的帧数量
     */
    static uint32_t PopFrames(std::deque<uint8_t>& refDequeBuffer, std::vector<CPelcoDPayload>& refVecFrames);

protected:
    /**
     * @brief 计算 Pelco-D 校验和（各字节累加，取低 8 位）
     * @param[in] pData 数据指针
     * @param[in] dwLen   数据长度
     * @return 校验和
     */
    static uint8_t CalcChecksum(const uint8_t* pData, uint32_t dwLen);

    /**
     * @brief 通用拆帧核心：同步扫描、帧长判定、校验和校验、弹出已解析帧
     * @param[in,out] refDequeBuffer 接收缓冲区，解析成功的帧会从缓冲区移除
     * @param[out]    refPayload     帧载荷（标准/扩展载荷均可）
     * @param[in]     pfnGetFrameLen 帧长判定函数（返回 0 表示数据不足无法判定）
     * @return true 解析成功；false 无完整帧（缓冲区为空或剩余数据不足一帧）
     * @note  缓冲区中混入的非法数据（非同步字节、校验失败）会被安全跳过；
     *        数据不足一帧时保留缓冲区内容，等待更多数据后再次调用
     */
    template<typename TPayload>
    static bool PopFrameInternal(std::deque<uint8_t>& refDequeBuffer, TPayload& refPayload,
                                 uint32_t (*pfnGetFrameLen)(const std::deque<uint8_t>& refDequeBuffer))
    {
        while (refDequeBuffer.empty() == false)
        {
            // 跳过非同步字节（非法数据）
            if (refDequeBuffer.front() != DEF_PELCO_D_SYNC_BYTE)
            {
                refDequeBuffer.pop_front();
                continue;
            }

            // 判定帧长；返回 0 表示数据不足无法判定
            uint32_t dwFrameLen = pfnGetFrameLen(refDequeBuffer);
            if (dwFrameLen == 0)
            {
                return false;
            }

            // 数据不足一帧，保留缓冲区等待更多数据
            if (refDequeBuffer.size() < dwFrameLen)
            {
                return false;
            }

            // 计算校验和（地址 + 命令 2 字节 + 数据，不含同步字节与校验和）
            uint8_t byChecksum = 0;
            for (uint32_t dwIdx = 1; dwIdx < dwFrameLen - 1; ++dwIdx)
            {
                byChecksum += refDequeBuffer[dwIdx];
            }

            // 校验失败：当前同步字节为非法数据，移除后继续查找
            if (byChecksum != refDequeBuffer[dwFrameLen - 1])
            {
                refDequeBuffer.pop_front();
                continue;
            }

            // 校验通过，取出帧载荷（地址、命令与数据 1/2）
            refPayload.m_byAddr = refDequeBuffer[1];
            refPayload.m_byCmd1 = refDequeBuffer[2];
            refPayload.m_byCmd2 = refDequeBuffer[3];
            refPayload.m_byData1 = refDequeBuffer[4];
            refPayload.m_byData2 = refDequeBuffer[5];

            // 9 字节帧（扩展位置帧 0xF5/0xF9）含数据 3/4，标准帧无此字段
            if (dwFrameLen > DEF_PELCO_D_FRAME_LEN)
            {
                FillData34(refPayload, refDequeBuffer[6], refDequeBuffer[7]);
            }
            else
            {
                FillData34(refPayload, 0, 0);
            }

            // 从缓冲区移除已解析的完整帧
            for (uint32_t dwIdx = 0; dwIdx < dwFrameLen; ++dwIdx)
            {
                refDequeBuffer.pop_front();
            }

            return true;
        }

        return false;
    }

    /**
     * @brief 帧数据 3/4 填充：标准载荷无该字段，默认空操作；扩展载荷显式特化
     * @param[out] refPayload 帧载荷
     * @param[in]  byData3    数据 3
     * @param[in]  byData4    数据 4
     */
    template<typename TPayload>
    static void FillData34(TPayload& /* refPayload */, uint8_t /* byData3 */, uint8_t /* byData4 */) {}

    /**
     * @brief 标准帧长判定：恒为 DEF_PELCO_D_FRAME_LEN（7 字节）
     * @param[in] refDequeBuffer 接收缓冲区
     * @return 帧长
     */
    static uint32_t GetStandardFrameLen(const std::deque<uint8_t>& /* refDequeBuffer */);

    /**
     * @brief 通用批量拆帧：循环调用 PopFrameInternal
     * @param[in,out] refDequeBuffer 接收缓冲区，解析成功的帧会从缓冲区移除
     * @param[out]    refVecFrames   帧载荷容器
     * @param[in]     pfnGetFrameLen 帧长判定函数
     * @return 解析出的帧数量
     */
    template<typename TPayload>
    static uint32_t PopFramesT(std::deque<uint8_t>& refDequeBuffer, std::vector<TPayload>& refVecFrames,
                               uint32_t (*pfnGetFrameLen)(const std::deque<uint8_t>& refDequeBuffer))
    {
        uint32_t dwCount = 0;
        TPayload payload;

        while (PopFrameInternal(refDequeBuffer, payload, pfnGetFrameLen) == true)
        {
            refVecFrames.push_back(payload);
            ++dwCount;
        }

        return dwCount;
    }
};

}  // namespace pelco_d
