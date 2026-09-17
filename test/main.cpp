/**
 * @file    main.cpp
 * @version 1.0.0
 * @date    2026-09-17
 * @author  yangjiang
 * @brief   Pelco-D 组帧拆帧测试程序
 * @details 覆盖组帧、拆帧、非法数据跳过、校验失败帧跳过、0xFF 数据字节、
 *          多帧解析、不完整帧保留、空缓冲区等场景。
 */

#include "pelco_d/PelcoDCodec.h"
#include "pelco_d/PelcoDExtCodec.h"

// 测试代码直接使用 pelco_d 命名空间中的类型
using namespace pelco_d;

#include <cstdio>
#include <cstring>
#include <deque>
#include <vector>

// 测试统计
static int s_nPassCount = 0;
static int s_nFailCount = 0;

// 向缓冲区写入数据
static void PushBytes(std::deque<uint8_t>& refDequeBuffer, const uint8_t* pData, uint32_t dwLen)
{
    for (uint32_t dwIdx = 0; dwIdx < dwLen; ++dwIdx)
    {
        refDequeBuffer.push_back(pData[dwIdx]);
    }
}

// 检查测试结果并统计
static void CheckResult(bool bResult, const char* pszCaseName)
{
    if (bResult)
    {
        ++s_nPassCount;
        printf("[PASS] %s\n", pszCaseName);
    }
    else
    {
        ++s_nFailCount;
        printf("[FAIL] %s\n", pszCaseName);
    }
}

// 组帧：基本用例与空指针用例
static void TestBuildFrame()
{
    // 帧载荷：地址 1，命令 0x0000，数据 0x00/0x00
    CPelcoDPayload payload;
    payload.m_byAddr = 0x01;
    payload.m_byCmd1 = 0x00;
    payload.m_byCmd2 = 0x00;
    payload.m_byData1 = 0x00;
    payload.m_byData2 = 0x00;

    // 期望帧：FF 01 00 00 00 00 01（校验和 = 0x01+0x00+0x00+0x00+0x00 = 0x01）
    uint8_t szExpect[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDCodec::BuildFrame(payload, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_FRAME_LEN) == 0), "BuildFrame basic");

    // 输出缓冲区为空指针应失败
}

// 组帧：使用命令常量（右转）
static void TestBuildFrameWithCommand()
{
    // 右转：m_byCmd1 = 0x00，m_byCmd2 = 右移(0x02)，水平速度 0x20
    CPelcoDPayload payload;
    payload.m_byAddr = 0x01;
    payload.m_byCmd1 = 0x00;
    payload.m_byCmd2 = DEF_PELCO_D_CMD2_PAN_RIGHT;
    payload.m_byData1 = 0x20;
    payload.m_byData2 = 0x00;

    // 期望帧：FF 01 00 02 20 00 23（校验和 = 0x01+0x00+0x02+0x20+0x00 = 0x23）
    uint8_t szExpect[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x02, 0x20, 0x00, 0x23};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDCodec::BuildFrame(payload, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_FRAME_LEN) == 0), "BuildFrame with command const");
}

// 组帧：命令位组合（上移 + 右移）
static void TestBuildFrameCombineCommand()
{
    // 上+右：m_byCmd2 = 上移(0x08) | 右移(0x02) = 0x0A，水平/垂直速度 0x20
    CPelcoDPayload payload;
    payload.m_byAddr = 0x01;
    payload.m_byCmd1 = 0x00;
    payload.m_byCmd2 = DEF_PELCO_D_CMD2_TILT_UP | DEF_PELCO_D_CMD2_PAN_RIGHT;
    payload.m_byData1 = 0x20;
    payload.m_byData2 = 0x20;

    // 期望帧：FF 01 00 0A 20 20 4B（校验和 = 0x01+0x00+0x0A+0x20+0x20 = 0x4B）
    uint8_t szExpect[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x0A, 0x20, 0x20, 0x4B};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDCodec::BuildFrame(payload, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_FRAME_LEN) == 0), "BuildFrame combined command");
}

// 便捷接口：上移
static void TestBuildMoveUpFrame()
{
    // 上移：addr=1，垂直速度 0x20 → FF 01 00 08 00 20 29（校验和 = 0x01+0x00+0x08+0x00+0x20 = 0x29）
    uint8_t szExpect[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x08, 0x00, 0x20, 0x29};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDCodec::BuildMoveUpFrame(0x01, 0x20, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_FRAME_LEN) == 0), "BuildMoveUpFrame");
}

// 便捷接口：停止
static void TestBuildStopFrame()
{
    // 停止：FF 01 00 00 00 00 01（校验和 = 0x01）
    uint8_t szExpect[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDCodec::BuildStopFrame(0x01, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_FRAME_LEN) == 0), "BuildStopFrame");
}

// 便捷接口：调用预置位
static void TestBuildCallPresetFrame()
{
    // 调用 5 号预置位：FF 01 00 07 00 05 0D（校验和 = 0x01+0x00+0x07+0x00+0x05 = 0x0D）
    uint8_t szExpect[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x07, 0x00, 0x05, 0x0D};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDCodec::BuildCallPresetFrame(0x01, 0x05, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_FRAME_LEN) == 0), "BuildCallPresetFrame");
}

// 便捷接口：查询水平位置（验证命令值 0x51）
static void TestBuildQueryPanFrame()
{
    // 查询水平位置：FF 01 00 51 00 00 52（校验和 = 0x01+0x00+0x51+0x00+0x00 = 0x52）
    uint8_t szExpect[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x51, 0x00, 0x00, 0x52};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDCodec::BuildQueryPanFrame(0x01, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_FRAME_LEN) == 0), "BuildQueryPanFrame");
}

// 便捷接口：打开辅助开关
static void TestBuildSetAuxFrame()
{
    // 打开 1 号辅助：FF 01 00 09 00 01 0B（校验和 = 0x01+0x00+0x09+0x00+0x01 = 0x0B）
    uint8_t szExpect[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x09, 0x00, 0x01, 0x0B};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDCodec::BuildSetAuxFrame(0x01, 0x01, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_FRAME_LEN) == 0), "BuildSetAuxFrame");
}

// 拆帧：单帧往返
static void TestPopFrameBasic()
{
    std::deque<uint8_t> dequeBuffer;
    uint8_t szFrame[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
    PushBytes(dequeBuffer, szFrame, DEF_PELCO_D_FRAME_LEN);

    CPelcoDPayload payload;
    bool bRet = CPelcoDCodec::PopFrame(dequeBuffer, payload);
    bool bOk = bRet
        && (payload.m_byAddr == 0x01)
        && (payload.m_byCmd1 == 0x00)
        && (payload.m_byCmd2 == 0x00)
        && (payload.m_byData1 == 0x00)
        && (payload.m_byData2 == 0x00)
        && dequeBuffer.empty();
    CheckResult(bOk, "PopFrame roundtrip");
}

// 拆帧：跳过前置非法数据（无 0xFF）
static void TestSkipGarbagePrefix()
{
    std::deque<uint8_t> dequeBuffer;
    uint8_t szGarbage[] = {0x00, 0x11, 0x22, 0x33};
    uint8_t szFrame[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
    PushBytes(dequeBuffer, szGarbage, sizeof(szGarbage));
    PushBytes(dequeBuffer, szFrame, DEF_PELCO_D_FRAME_LEN);

    CPelcoDPayload payload;
    bool bRet = CPelcoDCodec::PopFrame(dequeBuffer, payload);
    bool bOk = bRet && (payload.m_byAddr == 0x01) && dequeBuffer.empty();
    CheckResult(bOk, "skip garbage prefix");
}

// 拆帧：跳过校验和错误的帧，解析后续完整帧
static void TestSkipCorruptFrame()
{
    std::deque<uint8_t> dequeBuffer;
    // 校验和错误的帧（末字节应为 0x01，实际为 0x02）
    uint8_t szCorrupt[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02};
    // 完整帧：地址 2，校验和 = 0x02+0x00+0x00+0x00+0x00 = 0x02
    uint8_t szFrame[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02};
    PushBytes(dequeBuffer, szCorrupt, DEF_PELCO_D_FRAME_LEN);
    PushBytes(dequeBuffer, szFrame, DEF_PELCO_D_FRAME_LEN);

    CPelcoDPayload payload;
    bool bRet = CPelcoDCodec::PopFrame(dequeBuffer, payload);
    bool bOk = bRet && (payload.m_byAddr == 0x02) && dequeBuffer.empty();
    CheckResult(bOk, "skip corrupt checksum payload");
}

// 拆帧：解析连续多帧
static void TestPopFrames()
{
    std::deque<uint8_t> dequeBuffer;
    // 地址 1：校验和 0x01；地址 2：校验和 0x02；地址 3：校验和 0x03
    uint8_t szFrame1[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
    uint8_t szFrame2[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02};
    uint8_t szFrame3[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03};
    PushBytes(dequeBuffer, szFrame1, DEF_PELCO_D_FRAME_LEN);
    PushBytes(dequeBuffer, szFrame2, DEF_PELCO_D_FRAME_LEN);
    PushBytes(dequeBuffer, szFrame3, DEF_PELCO_D_FRAME_LEN);

    std::vector<CPelcoDPayload> vecFrames;
    uint32_t dwCount = CPelcoDCodec::PopFrames(dequeBuffer, vecFrames);
    bool bOk = (dwCount == 3)
        && (vecFrames.size() == 3)
        && (vecFrames[0].m_byAddr == 0x01)
        && (vecFrames[1].m_byAddr == 0x02)
        && (vecFrames[2].m_byAddr == 0x03)
        && dequeBuffer.empty();
    CheckResult(bOk, "pop multiple frames");
}

// 拆帧：尾部不完整帧应保留在缓冲区
static void TestIncompleteTail()
{
    std::deque<uint8_t> dequeBuffer;
    uint8_t szFrame[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
    uint8_t szTail[] = {0xFF, 0x01, 0x00, 0x00};
    PushBytes(dequeBuffer, szFrame, DEF_PELCO_D_FRAME_LEN);
    PushBytes(dequeBuffer, szTail, sizeof(szTail));

    CPelcoDPayload payload;
    bool bRet = CPelcoDCodec::PopFrame(dequeBuffer, payload);
    bool bOk = bRet
        && (payload.m_byAddr == 0x01)
        && (dequeBuffer.size() == sizeof(szTail))
        && (dequeBuffer[0] == 0xFF)
        && (dequeBuffer[3] == 0x00);
    CheckResult(bOk, "keep incomplete tail");
}

// 拆帧：数据字节含 0xFF 不应误判为同步头
static void TestFFInData()
{
    std::deque<uint8_t> dequeBuffer;
    // 数据 2 为 0xFF：校验和 = 0x01+0x00+0x00+0x00+0xFF = 0x100，低 8 位 0x00
    uint8_t szFrame[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0xFF, 0x00};
    PushBytes(dequeBuffer, szFrame, DEF_PELCO_D_FRAME_LEN);

    CPelcoDPayload payload;
    bool bRet = CPelcoDCodec::PopFrame(dequeBuffer, payload);
    bool bOk = bRet
        && (payload.m_byData2 == 0xFF)
        && dequeBuffer.empty();
    CheckResult(bOk, "payload with 0xFF in data");
}

// 拆帧：空缓冲区
static void TestEmptyBuffer()
{
    std::deque<uint8_t> dequeBuffer;
    CPelcoDPayload payload;
    CheckResult(CPelcoDCodec::PopFrame(dequeBuffer, payload) == false, "empty buffer");
}

// 拆帧：缓冲区只有非法数据（无 0xFF），应全部跳过
static void TestGarbageOnly()
{
    std::deque<uint8_t> dequeBuffer;
    uint8_t szGarbage[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    PushBytes(dequeBuffer, szGarbage, sizeof(szGarbage));

    CPelcoDPayload payload;
    bool bRet = CPelcoDCodec::PopFrame(dequeBuffer, payload);
    CheckResult((bRet == false) && dequeBuffer.empty(), "skip all garbage");
}

// 拆帧：混合流（非法数据 + 错误帧 + 2 个完整帧 + 不完整尾）
static void TestMixedStream()
{
    std::deque<uint8_t> dequeBuffer;
    uint8_t szGarbage[] = {0xAA, 0xBB, 0xCC, 0x00, 0xFF, 0x11};
    // 校验和错误的帧（末字节应为 0x01，实际为 0x99）
    uint8_t szCorrupt[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x99};
    // 地址 1：校验和 0x01
    uint8_t szFrame1[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
    // 地址 2、命令 0x0800：校验和 = 0x02+0x08+0x00+0x00+0x00 = 0x0A
    uint8_t szFrame2[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x02, 0x08, 0x00, 0x00, 0x00, 0x0A};
    uint8_t szTail[] = {0xFF, 0x05, 0x00};
    PushBytes(dequeBuffer, szGarbage, sizeof(szGarbage));
    PushBytes(dequeBuffer, szCorrupt, DEF_PELCO_D_FRAME_LEN);
    PushBytes(dequeBuffer, szFrame1, DEF_PELCO_D_FRAME_LEN);
    PushBytes(dequeBuffer, szFrame2, DEF_PELCO_D_FRAME_LEN);
    PushBytes(dequeBuffer, szTail, sizeof(szTail));

    std::vector<CPelcoDPayload> vecFrames;
    uint32_t dwCount = CPelcoDCodec::PopFrames(dequeBuffer, vecFrames);
    bool bOk = (dwCount == 2)
        && (vecFrames[0].m_byAddr == 0x01)
        && (vecFrames[1].m_byAddr == 0x02)
        && (dequeBuffer.size() == sizeof(szTail))
        && (dequeBuffer[0] == 0xFF)
        && (dequeBuffer[2] == 0x00);
    CheckResult(bOk, "mixed stream");
}

// ================= 附加协议（PTZ Extended Protocol V0.1）测试 =================

// 附加协议组帧：读位置（0xF7，7 字节）
static void TestExtBuildQueryPositionFrame()
{
    // 读位置：FF 01 00 F7 00 00 F8（校验和 = 0x01+0x00+0xF7+0x00+0x00 = 0xF8）
    uint8_t szExpect[DEF_PELCO_D_EXT_FRAME_LEN_MIN] = {0xFF, 0x01, 0x00, 0xF7, 0x00, 0x00, 0xF8};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDExtCodec::BuildQueryPositionFrame(0x01, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_EXT_FRAME_LEN_MIN) == 0), "Ext BuildQueryPositionFrame");
}

// 附加协议组帧：写位置（0xF9，9 字节）
static void TestExtBuildSetPositionFrame()
{
    // 水平 0x1234、俯仰 0xABCD：FF 01 00 F9 12 34 AB CD B8（校验和 = 0x2B8，低 8 位 0xB8）
    uint8_t szExpect[DEF_PELCO_D_EXT_FRAME_LEN_MAX] = {0xFF, 0x01, 0x00, 0xF9, 0x12, 0x34, 0xAB, 0xCD, 0xB8};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDExtCodec::BuildSetPositionFrame(0x01, 0x1234, 0xABCD, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_EXT_FRAME_LEN_MAX) == 0), "Ext BuildSetPositionFrame");
}

// 附加协议组帧：预置点间速度（0x85，7 字节）
static void TestExtBuildSetPresetSpeedFrame()
{
    // 速度 0x05：FF 01 00 85 00 05 8B（校验和 = 0x01+0x00+0x85+0x00+0x05 = 0x8B）
    uint8_t szExpect[DEF_PELCO_D_EXT_FRAME_LEN_MIN] = {0xFF, 0x01, 0x00, 0x85, 0x00, 0x05, 0x8B};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDExtCodec::BuildSetPresetSpeedFrame(0x01, 0x05, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_EXT_FRAME_LEN_MIN) == 0), "Ext BuildSetPresetSpeedFrame");
}

// 附加协议组帧：物理校零（0x87，7 字节）
static void TestExtBuildCalibrateZeroFrame()
{
    // 校零：FF 01 00 87 00 00 88（校验和 = 0x01+0x00+0x87+0x00+0x00 = 0x88）
    uint8_t szExpect[DEF_PELCO_D_EXT_FRAME_LEN_MIN] = {0xFF, 0x01, 0x00, 0x87, 0x00, 0x00, 0x88};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDExtCodec::BuildCalibrateZeroFrame(0x01, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_EXT_FRAME_LEN_MIN) == 0), "Ext BuildCalibrateZeroFrame");
}

// 附加协议拆帧：7 字节帧往返
static void TestExtPopFrame7Bytes()
{
    std::deque<uint8_t> dequeBuffer;
    uint8_t szFrame[DEF_PELCO_D_EXT_FRAME_LEN_MIN] = {0xFF, 0x01, 0x00, 0xF7, 0x00, 0x00, 0xF8};
    PushBytes(dequeBuffer, szFrame, DEF_PELCO_D_EXT_FRAME_LEN_MIN);

    CPelcoDExtPayload payload;
    bool bRet = CPelcoDExtCodec::PopFrame(dequeBuffer, payload);
    bool bOk = bRet
        && (payload.m_byAddr == 0x01)
        && (payload.m_byCmd2 == 0xF7)
        && (payload.m_byData1 == 0x00)
        && (payload.m_byData2 == 0x00)
        && (payload.m_byData3 == 0x00)
        && (payload.m_byData4 == 0x00)
        && dequeBuffer.empty();
    CheckResult(bOk, "Ext PopFrame 7 bytes");
}

// 附加协议拆帧：9 字节帧往返
static void TestExtPopFrame9Bytes()
{
    std::deque<uint8_t> dequeBuffer;
    uint8_t szFrame[DEF_PELCO_D_EXT_FRAME_LEN_MAX] = {0xFF, 0x01, 0x00, 0xF9, 0x12, 0x34, 0xAB, 0xCD, 0xB8};
    PushBytes(dequeBuffer, szFrame, DEF_PELCO_D_EXT_FRAME_LEN_MAX);

    CPelcoDExtPayload payload;
    bool bRet = CPelcoDExtCodec::PopFrame(dequeBuffer, payload);
    bool bOk = bRet
        && (payload.m_byData1 == 0x12)
        && (payload.m_byData2 == 0x34)
        && (payload.m_byData3 == 0xAB)
        && (payload.m_byData4 == 0xCD)
        && dequeBuffer.empty();
    CheckResult(bOk, "Ext PopFrame 9 bytes");
}

// 附加协议拆帧：7/9 字节混合流
static void TestExtPopFramesMixed()
{
    std::deque<uint8_t> dequeBuffer;
    uint8_t szFrame7[DEF_PELCO_D_EXT_FRAME_LEN_MIN] = {0xFF, 0x01, 0x00, 0xF7, 0x00, 0x00, 0xF8};
    uint8_t szFrame9[DEF_PELCO_D_EXT_FRAME_LEN_MAX] = {0xFF, 0x01, 0x00, 0xF9, 0x12, 0x34, 0xAB, 0xCD, 0xB8};
    uint8_t szFrame85[DEF_PELCO_D_EXT_FRAME_LEN_MIN] = {0xFF, 0x01, 0x00, 0x85, 0x00, 0x05, 0x8B};
    PushBytes(dequeBuffer, szFrame7, DEF_PELCO_D_EXT_FRAME_LEN_MIN);
    PushBytes(dequeBuffer, szFrame9, DEF_PELCO_D_EXT_FRAME_LEN_MAX);
    PushBytes(dequeBuffer, szFrame85, DEF_PELCO_D_EXT_FRAME_LEN_MIN);

    std::vector<CPelcoDExtPayload> vecFrames;
    uint32_t dwCount = CPelcoDExtCodec::PopFrames(dequeBuffer, vecFrames);
    bool bOk = (dwCount == 3)
        && (vecFrames[0].m_byCmd2 == 0xF7)
        && (vecFrames[1].m_byCmd2 == 0xF9)
        && (vecFrames[2].m_byCmd2 == 0x85)
        && dequeBuffer.empty();
    CheckResult(bOk, "Ext pop mixed 7/9 bytes");
}

// 附加协议拆帧：跳过非法前缀
static void TestExtSkipGarbage()
{
    std::deque<uint8_t> dequeBuffer;
    uint8_t szGarbage[] = {0x00, 0x11, 0x22, 0x33};
    uint8_t szFrame[DEF_PELCO_D_EXT_FRAME_LEN_MAX] = {0xFF, 0x01, 0x00, 0xF9, 0x12, 0x34, 0xAB, 0xCD, 0xB8};
    PushBytes(dequeBuffer, szGarbage, sizeof(szGarbage));
    PushBytes(dequeBuffer, szFrame, DEF_PELCO_D_EXT_FRAME_LEN_MAX);

    CPelcoDExtPayload payload;
    bool bRet = CPelcoDExtCodec::PopFrame(dequeBuffer, payload);
    bool bOk = bRet && (payload.m_byData3 == 0xAB) && dequeBuffer.empty();
    CheckResult(bOk, "Ext skip garbage prefix");
}

// 附加协议拆帧：跳过校验错误帧，解析后续完整帧
static void TestExtSkipCorrupt()
{
    std::deque<uint8_t> dequeBuffer;
    // 校验错误 7 字节帧（末字节应为 0xF8，实际为 0x99）
    uint8_t szCorrupt[DEF_PELCO_D_EXT_FRAME_LEN_MIN] = {0xFF, 0x01, 0x00, 0xF7, 0x00, 0x00, 0x99};
    // 地址 2 的写位置帧：校验和 = 0x02+0x00+0xF9+0x12+0x34+0xAB+0xCD = 0x2B9，低 8 位 0xB9
    uint8_t szFrame[DEF_PELCO_D_EXT_FRAME_LEN_MAX] = {0xFF, 0x02, 0x00, 0xF9, 0x12, 0x34, 0xAB, 0xCD, 0xB9};
    PushBytes(dequeBuffer, szCorrupt, DEF_PELCO_D_EXT_FRAME_LEN_MIN);
    PushBytes(dequeBuffer, szFrame, DEF_PELCO_D_EXT_FRAME_LEN_MAX);

    CPelcoDExtPayload payload;
    bool bRet = CPelcoDExtCodec::PopFrame(dequeBuffer, payload);
    bool bOk = bRet && (payload.m_byAddr == 0x02) && dequeBuffer.empty();
    CheckResult(bOk, "Ext skip corrupt checksum payload");
}

// 附加协议拆帧：9 字节帧不完整时保留，补足后解析
static void TestExtIncomplete()
{
    std::deque<uint8_t> dequeBuffer;
    // 9 字节帧只推 7 字节
    uint8_t szTail9[DEF_PELCO_D_EXT_FRAME_LEN_MAX - 2] = {0xFF, 0x01, 0x00, 0xF9, 0x12, 0x34, 0xAB};
    PushBytes(dequeBuffer, szTail9, sizeof(szTail9));

    CPelcoDExtPayload payload;
    bool bRet = CPelcoDExtCodec::PopFrame(dequeBuffer, payload);
    bool bOk = (bRet == false)
        && (dequeBuffer.size() == sizeof(szTail9))
        && (dequeBuffer[0] == 0xFF)
        && (dequeBuffer[3] == 0xF9);
    CheckResult(bOk, "Ext keep incomplete 9-byte tail");

    // 补足后 2 字节可解析
    uint8_t szRest[] = {0xCD, 0xB8};
    PushBytes(dequeBuffer, szRest, sizeof(szRest));
    bRet = CPelcoDExtCodec::PopFrame(dequeBuffer, payload);
    bOk = bRet && (payload.m_byData4 == 0xCD) && dequeBuffer.empty();
    CheckResult(bOk, "Ext complete after tail append");
}

// 附加协议拆帧：位置回复帧解析（0xF5）
static void TestExtGetPosition()
{
    std::deque<uint8_t> dequeBuffer;
    // 云台回复：水平 0x1234、俯仰 0xABCD；校验和 = 0x01+0x00+0xF5+0x12+0x34+0xAB+0xCD = 0x2B4，低 8 位 0xB4
    uint8_t szFrame[DEF_PELCO_D_EXT_FRAME_LEN_MAX] = {0xFF, 0x01, 0x00, 0xF5, 0x12, 0x34, 0xAB, 0xCD, 0xB4};
    PushBytes(dequeBuffer, szFrame, DEF_PELCO_D_EXT_FRAME_LEN_MAX);

    CPelcoDExtPayload payload;
    bool bRet = CPelcoDExtCodec::PopFrame(dequeBuffer, payload);
    bool bOk = bRet
        && (CPelcoDExtCodec::GetPanPosition(payload) == 0x1234)
        && (CPelcoDExtCodec::GetTiltPosition(payload) == 0xABCD)
        && dequeBuffer.empty();
    CheckResult(bOk, "Ext get position from reply");
}

// 附加协议拆帧：9 字节帧数据含 0xFF
static void TestExtFFInData()
{
    std::deque<uint8_t> dequeBuffer;
    // 水平 0x12FF、俯仰 0xABCD：校验和 = 0x383，低 8 位 0x83
    uint8_t szFrame[DEF_PELCO_D_EXT_FRAME_LEN_MAX] = {0xFF, 0x01, 0x00, 0xF9, 0x12, 0xFF, 0xAB, 0xCD, 0x83};
    PushBytes(dequeBuffer, szFrame, DEF_PELCO_D_EXT_FRAME_LEN_MAX);

    CPelcoDExtPayload payload;
    bool bRet = CPelcoDExtCodec::PopFrame(dequeBuffer, payload);
    bool bOk = bRet
        && (CPelcoDExtCodec::GetPanPosition(payload) == 0x12FF)
        && dequeBuffer.empty();
    CheckResult(bOk, "Ext payload with 0xFF in data");
}


// 附加协议组帧：通用命令组帧直接调用
static void TestExtBuildCommandFrame()
{
    // 通用组帧：addr=1，命令 0x0000/0x85，数据 0x00/0x05 → FF 01 00 85 00 05 8B
    uint8_t szExpect[DEF_PELCO_D_EXT_FRAME_LEN_MIN] = {0xFF, 0x01, 0x00, 0x85, 0x00, 0x05, 0x8B};
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDExtCodec::BuildCommandFrame(0x01, 0x00, DEF_PELCO_D_EXT_CMD_PRESET_SPEED, 0x00, 0x05, vecFrame);
    CheckResult(bRet && (memcmp(vecFrame.data(), szExpect, DEF_PELCO_D_EXT_FRAME_LEN_MIN) == 0), "Ext BuildCommandFrame");
}

// 用 CPelcoDExtCodec 同时处理标准帧与扩展帧（继承 + using 引入基类接口）
static void TestExtCodecHandlesStandard()
{
    // 组标准帧：继承自基类的便捷接口（7 字节）
    std::vector<uint8_t> vecFrame;
    bool bRet = CPelcoDExtCodec::BuildMoveRightFrame(0x01, 0x20, vecFrame);
    bool bOk = bRet && (vecFrame.size() == DEF_PELCO_D_FRAME_LEN)
        && (vecFrame[0] == DEF_PELCO_D_SYNC_BYTE)
        && (vecFrame[3] == DEF_PELCO_D_CMD2_PAN_RIGHT);
    CheckResult(bOk, "ExtCodec builds standard frame");

    // 拆标准帧：using 引入的基类 PopFrame 重载（7 字节）
    std::deque<uint8_t> dequeBuffer;
    uint8_t szStdFrame[DEF_PELCO_D_FRAME_LEN] = {0xFF, 0x01, 0x00, 0x02, 0x20, 0x00, 0x23};
    PushBytes(dequeBuffer, szStdFrame, DEF_PELCO_D_FRAME_LEN);
    CPelcoDPayload stdPayload;
    bRet = CPelcoDExtCodec::PopFrame(dequeBuffer, stdPayload);
    bOk = bRet && (stdPayload.m_byAddr == 0x01)
        && (stdPayload.m_byCmd2 == DEF_PELCO_D_CMD2_PAN_RIGHT)
        && dequeBuffer.empty();
    CheckResult(bOk, "ExtCodec pops standard frame");

    // 拆扩展帧：扩展类 PopFrame 重载（9 字节位置帧）
    uint8_t szExtFrame[DEF_PELCO_D_EXT_FRAME_LEN_MAX] = {0xFF, 0x01, 0x00, 0xF9, 0x12, 0x34, 0xAB, 0xCD, 0xB8};
    PushBytes(dequeBuffer, szExtFrame, DEF_PELCO_D_EXT_FRAME_LEN_MAX);
    CPelcoDExtPayload extPayload;
    bRet = CPelcoDExtCodec::PopFrame(dequeBuffer, extPayload);
    bOk = bRet && (extPayload.m_byAddr == 0x01)
        && (extPayload.m_byData3 == 0xAB)
        && (extPayload.m_byData4 == 0xCD)
        && dequeBuffer.empty();
    CheckResult(bOk, "ExtCodec pops ext frame");
}

int main()
{
    printf("=== Pelco-D codec test ===\n");

    TestBuildFrame();
    TestBuildFrameWithCommand();
    TestBuildFrameCombineCommand();
    TestBuildMoveUpFrame();
    TestBuildStopFrame();
    TestBuildCallPresetFrame();
    TestBuildQueryPanFrame();
    TestBuildSetAuxFrame();
    TestPopFrameBasic();
    TestSkipGarbagePrefix();
    TestSkipCorruptFrame();
    TestPopFrames();
    TestIncompleteTail();
    TestFFInData();
    TestEmptyBuffer();
    TestGarbageOnly();
    TestMixedStream();

    // 附加协议
    TestExtBuildCommandFrame();
    TestExtCodecHandlesStandard();
    TestExtBuildQueryPositionFrame();
    TestExtBuildSetPositionFrame();
    TestExtBuildSetPresetSpeedFrame();
    TestExtBuildCalibrateZeroFrame();
    TestExtPopFrame7Bytes();
    TestExtPopFrame9Bytes();
    TestExtPopFramesMixed();
    TestExtSkipGarbage();
    TestExtSkipCorrupt();
    TestExtIncomplete();
    TestExtGetPosition();
    TestExtFFInData();

    printf("\nPass: %d, Fail: %d\n", s_nPassCount, s_nFailCount);
    return (s_nFailCount == 0) ? 0 : 1;
}
