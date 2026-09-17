# Pelco-D 协议编解码库接口文档

> 适用版本：1.0.0（2026-09-18）
> 项目：pelco-d — Pelco-D 协议与云台附加协议（PTZ Extended Protocol V0.1）的 C++ 组帧/拆帧库
> 编译要求：C++17 及以上；不依赖网络层，接收缓冲区为 `std::deque<uint8_t>`
> 构建：默认生成动态库，可选 `-DPELCO_D_BUILD_STATIC=ON` 生成静态库（见 README「构建与测试」）

## 1. 概述

本库提供两类协议帧的编解码：

| 协议 | 帧长 | 说明 |
| ---- | ---- | ---- |
| 标准 Pelco-D | 7 字节定长 | `FF \| 地址 \| 命令1 \| 命令2 \| 数据1 \| 数据2 \| 校验和` |
| 附加协议 | 7 / 9 字节变长 | 基础帧 7 字节；位置类指令（0xF5 回复 / 0xF9 写位置）9 字节 |

- 校验和：地址 + 命令 2 字节 + 数据累加，取低 8 位（不含同步字节与校验和本身）。
- 同步字节 `0xFF` 不参与校验和计算。
- 所有类型位于命名空间 `pelco_d`。
- 全部接口为静态方法，无实例化要求，无内部状态，天然线程安全。

## 2. 帧格式

### 2.1 标准 Pelco-D 帧（7 字节）

| 字节 | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
| ---- | - | - | - | - | - | - | - |
| 内容 | 0xFF | 地址 | 命令1 | 命令2 | 数据1 | 数据2 | 校验和 |

### 2.2 附加协议帧

基础帧（7 字节）结构同上；位置类帧（9 字节）：

| 字节 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
| ---- | - | - | - | - | - | - | - | - | - |
| 内容 | 0xFF | 地址 | 0x00 | 命令2 | 数据1 | 数据2 | 数据3 | 数据4 | 校验和 |

帧长由命令字节 2（COMD2）判定：`0xF5` / `0xF9` 为 9 字节，其余 7 字节。

## 3. 宏定义

### 3.1 帧宏（PelcoDPayload.h）

| 宏 | 值 | 说明 |
| --- | --- | --- |
| `DEF_PELCO_D_SYNC_BYTE` | `0xFF` | 帧同步字节 |
| `DEF_PELCO_D_FRAME_LEN` | `7` | 标准帧总长度 |

### 3.2 附加协议帧宏（PelcoDExtCodec.h）

| 宏 | 值 | 说明 |
| --- | --- | --- |
| `DEF_PELCO_D_EXT_FRAME_LEN_MIN` | `7` | 最短帧长（数据 2 字节） |
| `DEF_PELCO_D_EXT_FRAME_LEN_MAX` | `9` | 最长帧长（数据 4 字节） |

### 3.3 命令 1 位定义（m_byCmd1，可按位组合）

| 宏 | 值 | 说明 |
| --- | --- | --- |
| `DEF_PELCO_D_CMD1_SENSE` | `0x80` | 感测位（与相机关闭位配合） |
| `DEF_PELCO_D_CMD1_AUTO_SCAN` | `0x10` | 自动/手动扫描 |
| `DEF_PELCO_D_CMD1_CAMERA_OFF` | `0x08` | 相机关闭（1 关闭，0 开启） |
| `DEF_PELCO_D_CMD1_IRIS_CLOSE` | `0x04` | 关光圈 |
| `DEF_PELCO_D_CMD1_IRIS_OPEN` | `0x02` | 开光圈 |
| `DEF_PELCO_D_CMD1_FOCUS_NEAR` | `0x01` | 近焦 |

### 3.4 命令 2 位定义（m_byCmd2，可按位组合）

| 宏 | 值 | 说明 |
| --- | --- | --- |
| `DEF_PELCO_D_CMD2_FOCUS_FAR` | `0x80` | 远焦 |
| `DEF_PELCO_D_CMD2_ZOOM_WIDE` | `0x40` | 缩小 |
| `DEF_PELCO_D_CMD2_ZOOM_TELE` | `0x20` | 放大 |
| `DEF_PELCO_D_CMD2_TILT_DOWN` | `0x10` | 下移 |
| `DEF_PELCO_D_CMD2_TILT_UP` | `0x08` | 上移 |
| `DEF_PELCO_D_CMD2_PAN_LEFT` | `0x04` | 左移 |
| `DEF_PELCO_D_CMD2_PAN_RIGHT` | `0x02` | 右移 |

### 3.5 扩展命令（m_byCmd1 = 0x00，值赋给 m_byCmd2）

| 宏 | 值 | 说明 |
| --- | --- | --- |
| `DEF_PELCO_D_CMD_PRESET_SET` | `0x03` | 设置预置位（预置位号放数据2） |
| `DEF_PELCO_D_CMD_PRESET_CLEAR` | `0x05` | 清除预置位（预置位号放数据2） |
| `DEF_PELCO_D_CMD_PRESET_CALL` | `0x07` | 调用预置位（预置位号放数据2） |
| `DEF_PELCO_D_CMD_AUX_SET` | `0x09` | 打开辅助开关（辅助号 1~8 放数据2） |
| `DEF_PELCO_D_CMD_AUX_CLEAR` | `0x0B` | 关闭辅助开关（辅助号 1~8 放数据2） |
| `DEF_PELCO_D_CMD_REMOTE_RESET` | `0x0F` | 远程复位 |
| `DEF_PELCO_D_CMD_STOP_PATTERN` | `0x21` | 停止巡迹 |
| `DEF_PELCO_D_CMD_RUN_PATTERN` | `0x23` | 运行巡迹（巡迹号放数据2） |
| `DEF_PELCO_D_CMD_SET_ZOOM_SPEED` | `0x25` | 设置变焦速度（0~3 放数据2） |
| `DEF_PELCO_D_CMD_SET_FOCUS_SPEED` | `0x27` | 设置聚焦速度（0~3 放数据2） |
| `DEF_PELCO_D_CMD_RESET_DEFAULTS` | `0x29` | 相机恢复默认设置 |
| `DEF_PELCO_D_CMD_SET_ZERO` | `0x49` | 设置零位 |
| `DEF_PELCO_D_CMD_SET_PAN` | `0x4B` | 设置水平位置（数据1=高字节，数据2=低字节） |
| `DEF_PELCO_D_CMD_SET_TILT` | `0x4D` | 设置垂直位置（数据1=高字节，数据2=低字节） |
| `DEF_PELCO_D_CMD_SET_ZOOM` | `0x4F` | 设置变焦位置（数据1=高字节，数据2=低字节） |
| `DEF_PELCO_D_CMD_QUERY_PAN` | `0x51` | 查询水平位置 |
| `DEF_PELCO_D_CMD_QUERY_TILT` | `0x53` | 查询垂直位置 |
| `DEF_PELCO_D_CMD_QUERY_ZOOM` | `0x55` | 查询变焦位置 |
| `DEF_PELCO_D_CMD_QUERY_VERSION` | `0x73` | 查询版本信息 |

### 3.6 附加协议命令（m_byCmd1 = 0x00，值赋给 m_byCmd2）

| 宏 | 值 | 帧长 | 说明 |
| --- | --- | --- | --- |
| `DEF_PELCO_D_EXT_CMD_QUERY_POS` | `0xF7` | 7 字节 | 读指令：同时读取水平/俯仰位置（主机发送） |
| `DEF_PELCO_D_EXT_CMD_POS_REPLY` | `0xF5` | 9 字节 | 云台回复位置（水平高/低、俯仰高/低） |
| `DEF_PELCO_D_EXT_CMD_SET_POS` | `0xF9` | 9 字节 | 写指令：向指定位置以设定速度直线运行（云台不回复） |
| `DEF_PELCO_D_EXT_CMD_PRESET_SPEED` | `0x85` | 7 字节 | 写指令：预置点间速度设定（速度放数据2） |
| `DEF_PELCO_D_EXT_CMD_CALIB_ZERO` | `0x87` | 7 字节 | 写指令：水平/俯仰运行至物理零点（水平 0、垂直 27000） |

## 4. 类型定义

### 4.1 CPelcoDPayload — 标准帧载荷

头文件：`pelco_d/PelcoDPayload.h`

```cpp
class CPelcoDPayload
{
public:
    CPelcoDPayload();                // 全部成员清零
    uint8_t m_byAddr;                // 设备地址
    uint8_t m_byCmd1;                // 命令字节 1
    uint8_t m_byCmd2;                // 命令字节 2
    uint8_t m_byData1;               // 数据 1（水平速度，0x00~0x3F）
    uint8_t m_byData2;               // 数据 2（垂直速度，0x00~0x3F）
};
```

### 4.2 CPelcoDExtPayload — 附加协议帧载荷

头文件：`pelco_d/PelcoDExtPayload.h`；继承 `CPelcoDPayload`，表达"附加协议基于标准 Pelco-D"。

```cpp
class CPelcoDExtPayload : public CPelcoDPayload
{
public:
    CPelcoDExtPayload();             // 调用基类构造后清零数据 3/4
    uint8_t m_byData3;               // 数据 3（俯仰位置高字节）
    uint8_t m_byData4;               // 数据 4（俯仰位置低字节）
};
```

## 5. 接口 — CPelcoDCodec（标准协议）

头文件：`pelco_d/PelcoDCodec.h`

### 5.1 组帧

```cpp
// 组帧：将帧载荷组装为完整 7 字节帧
static bool BuildFrame(const CPelcoDPayload& refPayload, std::vector<uint8_t>& refVecFrame);

// 命令组帧：按地址、命令、数据组装标准帧（7 字节）
static bool BuildCommandFrame(uint8_t byAddr, uint8_t byCmd1, uint8_t byCmd2,
                              uint8_t byData1, uint8_t byData2,
                              std::vector<uint8_t>& refVecFrame);
```

参数：

- `refPayload` — 帧载荷（地址、命令、数据）
- `byAddr` — 设备地址
- `byCmd1` / `byCmd2` — 命令字节（见第 3 节宏定义）
- `byData1` / `byData2` — 数据字节
- `refVecFrame` — 输出完整帧，长度自动置为 7，内容为 `FF | 地址 | 命令1 | 命令2 | 数据1 | 数据2 | 校验和`

返回：`true` 成功。

### 5.2 便捷组帧接口

> 出参均为 `std::vector<uint8_t>& refVecFrame`（完整 7 字节帧），返回 `bool`。

| 接口 | 参数 | 说明 |
| ---- | ---- | ---- |
| `BuildMoveUpFrame` | `byAddr, byTiltSpeed` | 上移（垂直速度 0x00~0x3F） |
| `BuildMoveDownFrame` | `byAddr, byTiltSpeed` | 下移 |
| `BuildMoveLeftFrame` | `byAddr, byPanSpeed` | 左移（水平速度 0x00~0x3F） |
| `BuildMoveRightFrame` | `byAddr, byPanSpeed` | 右移 |
| `BuildStopFrame` | `byAddr` | 停止移动 |
| `BuildZoomInFrame` | `byAddr` | 放大 |
| `BuildZoomOutFrame` | `byAddr` | 缩小 |
| `BuildFocusFarFrame` | `byAddr` | 远焦 |
| `BuildFocusNearFrame` | `byAddr` | 近焦 |
| `BuildIrisOpenFrame` | `byAddr` | 开光圈 |
| `BuildIrisCloseFrame` | `byAddr` | 关光圈 |
| `BuildSetPresetFrame` | `byAddr, byPresetId` | 设置预置位（1~255） |
| `BuildClearPresetFrame` | `byAddr, byPresetId` | 清除预置位 |
| `BuildCallPresetFrame` | `byAddr, byPresetId` | 调用预置位 |
| `BuildSetAuxFrame` | `byAddr, byAuxId` | 打开辅助开关（1~8） |
| `BuildClearAuxFrame` | `byAddr, byAuxId` | 关闭辅助开关 |
| `BuildRemoteResetFrame` | `byAddr` | 远程复位 |
| `BuildQueryPanFrame` | `byAddr` | 查询水平位置（0x51） |
| `BuildQueryTiltFrame` | `byAddr` | 查询垂直位置（0x53） |
| `BuildQueryZoomFrame` | `byAddr` | 查询变焦位置（0x55） |

### 5.3 拆帧

```cpp
// 从接收缓冲区解析一帧完整数据，解析成功后从缓冲区移除
static bool PopFrame(std::deque<uint8_t>& refDequeBuffer, CPelcoDPayload& refPayload);

// 从接收缓冲区解析出全部完整帧
static uint32_t PopFrames(std::deque<uint8_t>& refDequeBuffer, std::vector<CPelcoDPayload>& refVecFrames);
```

参数：

- `refDequeBuffer` — 接收缓冲区（网络层写入）。解析成功的帧会从缓冲区移除；非同步字节与校验失败的数据被安全跳过；数据不足一帧时保留缓冲区内容，等待更多数据后再次调用。
- `refPayload` / `refVecFrames` — 输出帧载荷 / 帧载荷容器

返回：`PopFrame` 返回 `true` 解析成功、`false` 无完整帧；`PopFrames` 返回解析出的帧数量。

## 6. 接口 — CPelcoDExtCodec（附加协议，含标准协议）

头文件：`pelco_d/PelcoDExtCodec.h`；继承 `CPelcoDCodec`。

> 通过继承与 `using` 声明，**一个类即可处理标准与扩展两种协议**：
> - 组帧：继承基类全部标准接口（`BuildFrame`、`BuildMoveRightFrame` 等）；
> - 拆帧：按载荷类型自动分派 —— 传 `CPelcoDPayload` 走标准 7 字节，传 `CPelcoDExtPayload` 走扩展 7/9 字节。

```cpp
using CPelcoDCodec::PopFrame;    // 标准帧拆帧重载（CPelcoDPayload）
using CPelcoDCodec::PopFrames;
```

### 6.1 组帧

```cpp
// 命令组帧：按地址、命令、数据组装 7 字节帧
static bool BuildCommandFrame(uint8_t byAddr, uint8_t byCmd1, uint8_t byCmd2,
                              uint8_t byData1, uint8_t byData2,
                              std::vector<uint8_t>& refVecFrame);

// 读位置帧：同时读取当前水平/俯仰位置（0xF7，7 字节）
static bool BuildQueryPositionFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);

// 写位置帧：向指定位置以设定速度直线运行（0xF9，9 字节）
static bool BuildSetPositionFrame(uint8_t byAddr, uint16_t wPanPos, uint16_t wTiltPos,
                                  std::vector<uint8_t>& refVecFrame);

// 预置点间速度设定帧（0x85，7 字节）
static bool BuildSetPresetSpeedFrame(uint8_t byAddr, uint8_t bySpeed,
                                     std::vector<uint8_t>& refVecFrame);

// 物理校零帧：水平/俯仰运行至物理零点（0x87，7 字节）
static bool BuildCalibrateZeroFrame(uint8_t byAddr, std::vector<uint8_t>& refVecFrame);
```

参数补充说明：

- `wPanPos` — 水平位置（0~35999，高字节在前）
- `wTiltPos` — 俯仰位置（0~9000 或 27000~35999，高字节在前）
- `bySpeed` — 预置点间巡航速度（放数据2）

返回：`true` 成功。

### 6.2 拆帧

```cpp
// 从接收缓冲区解析一帧完整数据（7/9 字节按 COMD2 自动判定）
static bool PopFrame(std::deque<uint8_t>& refDequeBuffer, CPelcoDExtPayload& refPayload);

// 从接收缓冲区解析出全部完整帧
static uint32_t PopFrames(std::deque<uint8_t>& refDequeBuffer, std::vector<CPelcoDExtPayload>& refVecFrames);
```

语义与标准协议一致（跳过非法数据、保留不完整尾部）。

### 6.3 位置解析

```cpp
// 读取水平位置（回复帧 0xF5：数据1<<8 + 数据2）
static uint16_t GetPanPosition(const CPelcoDExtPayload& refPayload);

// 读取俯仰位置（回复帧 0xF5：数据3<<8 + 数据4）
static uint16_t GetTiltPosition(const CPelcoDExtPayload& refPayload);
```

返回：`wPanPos` 水平位置（0~35999）；`wTiltPos` 俯仰位置（0~9000 或 27000~35999）。

## 7. 使用示例

### 7.1 标准协议

```cpp
#include "pelco_d/PelcoDCodec.h"
#include <deque>
#include <vector>

using namespace pelco_d;

// 组帧：右转，水平速度 0x20
std::vector<uint8_t> vecFrame;
CPelcoDCodec::BuildMoveRightFrame(0x01, 0x20, vecFrame);

// 拆帧：从接收缓冲区解析
std::deque<uint8_t> dequeBuffer;
CPelcoDPayload payload;
if (CPelcoDCodec::PopFrame(dequeBuffer, payload))
{
    // payload.m_byAddr / m_byCmd1 / m_byCmd2 / m_byData1 / m_byData2
}
```

### 7.2 附加协议

```cpp
#include "pelco_d/PelcoDExtCodec.h"

using namespace pelco_d;

// 查询位置（0xF7，7 字节）
std::vector<uint8_t> vecFrame;
CPelcoDExtCodec::BuildQueryPositionFrame(0x01, vecFrame);

// 写入位置：水平 0x1234、俯仰 0xABCD（0xF9，9 字节）
CPelcoDExtCodec::BuildSetPositionFrame(0x01, 0x1234, 0xABCD, vecFrame);

// 解析云台回复帧（0xF5）
std::deque<uint8_t> dequeBuffer;
CPelcoDExtPayload payload;
if (CPelcoDExtCodec::PopFrame(dequeBuffer, payload))
{
    uint16_t wPan = CPelcoDExtCodec::GetPanPosition(payload);   // 水平位置
    uint16_t wTilt = CPelcoDExtCodec::GetTiltPosition(payload); // 俯仰位置
}
```

### 7.3 统一处理标准与扩展

```cpp
#include "pelco_d/PelcoDExtCodec.h"

using namespace pelco_d;

std::deque<uint8_t> dequeBuffer;

// 拆标准帧（7 字节）：传 CPelcoDPayload，自动走基类重载
CPelcoDPayload stdPayload;
if (CPelcoDExtCodec::PopFrame(dequeBuffer, stdPayload))
{
    // 标准协议处理
}

// 拆扩展帧（7/9 字节）：传 CPelcoDExtPayload，自动走扩展重载
CPelcoDExtPayload extPayload;
if (CPelcoDExtCodec::PopFrame(dequeBuffer, extPayload))
{
    // 附加协议处理（可提取位置等）
}

// 组标准帧：直接使用继承的便捷接口
std::vector<uint8_t> vecFrame;
CPelcoDExtCodec::BuildMoveRightFrame(0x01, 0x20, vecFrame);
```

## 8. 注意事项

1. **帧载荷不含同步字节与校验和**：`CPelcoDPayload` / `CPelcoDExtPayload` 仅承载地址、命令与数据字段。
2. **同步字节不参与校验和**：校验和 = 地址 + 命令 2 字节 + 数据累加取低 8 位（标准 5 字节；扩展 9 字节帧为 7 字节）。
3. **拆帧缓冲区语义**：解析成功的帧从 `std::deque<uint8_t>` 移除；非 `0xFF` 开头或校验失败的数据被安全跳过；不足一帧的尾部保留，等待更多数据后再次调用 `PopFrame` / `PopFrames`。
4. **帧内数据字节可为 `0xFF`**：不会误判为同步头（由帧长与校验和约束）。
5. **附加协议位置回复**（0xF5）仅在解析该类帧后可用 `GetPanPosition` / `GetTiltPosition` 提取位置。
6. **线程安全**：所有接口均为静态方法且无内部状态，可跨线程安全调用；不同缓冲区的并发拆帧互不影响。
