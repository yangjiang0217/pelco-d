/**
 * @file    PelcoDExtCodec.cpp
 * @version 1.0.0
 * @date    2026-09-18
 * @author  yangjiang
 * @brief   云台扩展协议（PTZ Extended Protocol V0.1）组帧拆帧实现
 */

#include "pelco_d/PelcoDExtCodec.h"

namespace pelco_d
{

bool CPelcoDExtCodec::BuildCommandFrame(uint8_t byAddr, uint8_t byCmd1, uint8_t byCmd2,
                                       uint8_t byData1, uint8_t byData2,
                                       std::vector<uint8_t>& refVecFrame)
{
    refVecFrame.clear();
    refVecFrame.resize(DEF_PELCO_D_EXT_FRAME_LEN_MIN);

    refVecFrame[0] = DEF_PELCO_D_SYNC_BYTE;
    refVecFrame[1] = byAddr;
    refVecFrame[2] = byCmd1;
    refVecFrame[3] = byCmd2;
    refVecFrame[4] = byData1;
    refVecFrame[5] = byData2;
    refVecFrame[6] = CalcChecksum(&refVecFrame[1], DEF_PELCO_D_EXT_FRAME_LEN_MIN - 2);

    return true;
}

bool CPelcoDExtCodec::BuildQueryPositionFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_EXT_CMD_QUERY_POS, 0x00, 0x00, refVecFrame);
}

bool CPelcoDExtCodec::BuildSetPositionFrame(uint8_t byAddr, uint16_t wPanPos, uint16_t wTiltPos,
                                           std::vector<uint8_t>& refVecFrame)
{
    refVecFrame.clear();
    refVecFrame.resize(DEF_PELCO_D_EXT_FRAME_LEN_MAX);

    refVecFrame[0] = DEF_PELCO_D_SYNC_BYTE;
    refVecFrame[1] = byAddr;
    refVecFrame[2] = 0x00;
    refVecFrame[3] = DEF_PELCO_D_EXT_CMD_SET_POS;
    refVecFrame[4] = static_cast<uint8_t>((wPanPos >> 8) & 0xFF);   // 水平位置高字节
    refVecFrame[5] = static_cast<uint8_t>(wPanPos & 0xFF);          // 水平位置低字节
    refVecFrame[6] = static_cast<uint8_t>((wTiltPos >> 8) & 0xFF);  // 俯仰位置高字节
    refVecFrame[7] = static_cast<uint8_t>(wTiltPos & 0xFF);         // 俯仰位置低字节
    refVecFrame[8] = CalcChecksum(&refVecFrame[1], DEF_PELCO_D_EXT_FRAME_LEN_MAX - 2);

    return true;
}

bool CPelcoDExtCodec::BuildSetPresetSpeedFrame(uint8_t byAddr, uint8_t bySpeed,
                                              std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_EXT_CMD_PRESET_SPEED, 0x00, bySpeed, refVecFrame);
}

bool CPelcoDExtCodec::BuildCalibrateZeroFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_EXT_CMD_CALIB_ZERO, 0x00, 0x00, refVecFrame);
}

uint32_t CPelcoDExtCodec::GetExtFrameLen(const std::deque<uint8_t>& refDequeBuffer)
{
    // 数据不足最短帧长（7 字节），无法判定
    if (refDequeBuffer.size() < DEF_PELCO_D_EXT_FRAME_LEN_MIN)
    {
        return 0;
    }

    // 位置类指令（0xF5 回复 / 0xF9 写位置）为 9 字节，其余 7 字节
    uint8_t byCmd2 = refDequeBuffer[3];
    if ((byCmd2 == DEF_PELCO_D_EXT_CMD_POS_REPLY) || (byCmd2 == DEF_PELCO_D_EXT_CMD_SET_POS))
    {
        return DEF_PELCO_D_EXT_FRAME_LEN_MAX;
    }

    return DEF_PELCO_D_EXT_FRAME_LEN_MIN;
}

// 数据 3/4 填充特化：扩展位置帧（0xF5/0xF9）解析后写入俯仰位置高/低字节
template<>
void CPelcoDCodec::FillData34<CPelcoDExtPayload>(CPelcoDExtPayload& refPayload, uint8_t byData3, uint8_t byData4)
{
    refPayload.m_byData3 = byData3;
    refPayload.m_byData4 = byData4;
}

bool CPelcoDExtCodec::PopFrame(std::deque<uint8_t>& refDequeBuffer, CPelcoDExtPayload& refPayload)
{
    return PopFrameInternal(refDequeBuffer, refPayload, GetExtFrameLen);
}

uint32_t CPelcoDExtCodec::PopFrames(std::deque<uint8_t>& refDequeBuffer, std::vector<CPelcoDExtPayload>& refVecFrames)
{
    return PopFramesT(refDequeBuffer, refVecFrames, GetExtFrameLen);
}

uint16_t CPelcoDExtCodec::GetPanPosition(const CPelcoDExtPayload& refPayload)
{
    return (static_cast<uint16_t>(refPayload.m_byData1) << 8) | refPayload.m_byData2;
}

uint16_t CPelcoDExtCodec::GetTiltPosition(const CPelcoDExtPayload& refPayload)
{
    return (static_cast<uint16_t>(refPayload.m_byData3) << 8) | refPayload.m_byData4;
}

}  // namespace pelco_d
