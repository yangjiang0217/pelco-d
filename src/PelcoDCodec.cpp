/**
 * @file    PelcoDCodec.cpp
 * @version 1.0.0
 * @date    2026-09-17
 * @author  yangjiang
 * @brief   Pelco-D 协议组帧拆帧实现
 */

#include "pelco_d/PelcoDCodec.h"

namespace pelco_d
{

bool CPelcoDCodec::BuildCommandFrame(uint8_t byAddr, uint8_t byCmd1, uint8_t byCmd2,
                                     uint8_t byData1, uint8_t byData2,
                                     std::vector<uint8_t>& refVecFrame)
{
    CPelcoDPayload payload;
    payload.m_byAddr = byAddr;
    payload.m_byCmd1 = byCmd1;
    payload.m_byCmd2 = byCmd2;
    payload.m_byData1 = byData1;
    payload.m_byData2 = byData2;
    return BuildFrame(payload, refVecFrame);
}

bool CPelcoDCodec::BuildMoveUpFrame(uint8_t byAddr, uint8_t byTiltSpeed, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD2_TILT_UP, 0x00, byTiltSpeed, refVecFrame);
}

bool CPelcoDCodec::BuildMoveDownFrame(uint8_t byAddr, uint8_t byTiltSpeed, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD2_TILT_DOWN, 0x00, byTiltSpeed, refVecFrame);
}

bool CPelcoDCodec::BuildMoveLeftFrame(uint8_t byAddr, uint8_t byPanSpeed, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD2_PAN_LEFT, byPanSpeed, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildMoveRightFrame(uint8_t byAddr, uint8_t byPanSpeed, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD2_PAN_RIGHT, byPanSpeed, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildStopFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, 0x00, 0x00, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildZoomInFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD2_ZOOM_TELE, 0x00, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildZoomOutFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD2_ZOOM_WIDE, 0x00, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildFocusFarFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD2_FOCUS_FAR, 0x00, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildFocusNearFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, DEF_PELCO_D_CMD1_FOCUS_NEAR, 0x00, 0x00, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildIrisOpenFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, DEF_PELCO_D_CMD1_IRIS_OPEN, 0x00, 0x00, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildIrisCloseFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, DEF_PELCO_D_CMD1_IRIS_CLOSE, 0x00, 0x00, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildSetPresetFrame(uint8_t byAddr, uint8_t byPresetId, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD_PRESET_SET, 0x00, byPresetId, refVecFrame);
}

bool CPelcoDCodec::BuildClearPresetFrame(uint8_t byAddr, uint8_t byPresetId, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD_PRESET_CLEAR, 0x00, byPresetId, refVecFrame);
}

bool CPelcoDCodec::BuildCallPresetFrame(uint8_t byAddr, uint8_t byPresetId, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD_PRESET_CALL, 0x00, byPresetId, refVecFrame);
}

bool CPelcoDCodec::BuildSetAuxFrame(uint8_t byAddr, uint8_t byAuxId, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD_AUX_SET, 0x00, byAuxId, refVecFrame);
}

bool CPelcoDCodec::BuildClearAuxFrame(uint8_t byAddr, uint8_t byAuxId, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD_AUX_CLEAR, 0x00, byAuxId, refVecFrame);
}

bool CPelcoDCodec::BuildRemoteResetFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD_REMOTE_RESET, 0x00, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildQueryPanFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD_QUERY_PAN, 0x00, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildQueryTiltFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD_QUERY_TILT, 0x00, 0x00, refVecFrame);
}

bool CPelcoDCodec::BuildQueryZoomFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame)
{
    return BuildCommandFrame(byAddr, 0x00, DEF_PELCO_D_CMD_QUERY_ZOOM, 0x00, 0x00, refVecFrame);
}

uint8_t CPelcoDCodec::CalcChecksum(const uint8_t* pData, uint32_t dwLen)
{
    uint8_t byChecksum = 0;
    if (pData == nullptr)
    {
        return byChecksum;
    }

    for (uint32_t dwIdx = 0; dwIdx < dwLen; ++dwIdx)
    {
        byChecksum += pData[dwIdx];
    }

    return byChecksum;
}

bool CPelcoDCodec::BuildFrame(const CPelcoDPayload& refPayload, std::vector<uint8_t>& refVecFrame)
{
    refVecFrame.clear();
    refVecFrame.resize(DEF_PELCO_D_FRAME_LEN);

    refVecFrame[0] = DEF_PELCO_D_SYNC_BYTE;
    refVecFrame[1] = refPayload.m_byAddr;
    refVecFrame[2] = refPayload.m_byCmd1;
    refVecFrame[3] = refPayload.m_byCmd2;
    refVecFrame[4] = refPayload.m_byData1;
    refVecFrame[5] = refPayload.m_byData2;
    refVecFrame[6] = CalcChecksum(&refVecFrame[1], DEF_PELCO_D_FRAME_LEN - 2);

    return true;
}

uint32_t CPelcoDCodec::GetStandardFrameLen(const std::deque<uint8_t>& /* refDequeBuffer */)
{
    return DEF_PELCO_D_FRAME_LEN;
}

bool CPelcoDCodec::PopFrame(std::deque<uint8_t>& refDequeBuffer, CPelcoDPayload& refPayload)
{
    return PopFrameInternal(refDequeBuffer, refPayload, GetStandardFrameLen);
}

uint32_t CPelcoDCodec::PopFrames(std::deque<uint8_t>& refDequeBuffer, std::vector<CPelcoDPayload>& refVecFrames)
{
    return PopFramesT(refDequeBuffer, refVecFrames, GetStandardFrameLen);
}

}  // namespace pelco_d
