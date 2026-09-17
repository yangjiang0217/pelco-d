# Pelco-D 协议实现

## 目录结构

```
pelco-d/
├── CMakeLists.txt
├── include/pelco_d/PelcoDPayload.h    # 标准帧载荷类
├── include/pelco_d/PelcoDCodec.h      # 标准组帧拆帧类（含命令定义）
├── include/pelco_d/PelcoDExtPayload.h # 扩展帧载荷类（继承标准帧）
├── include/pelco_d/PelcoDExtCodec.h   # 扩展组帧拆帧类（继承标准编解码）
├── src/PelcoDCodec.cpp                # 标准实现
├── src/PelcoDExtCodec.cpp             # 扩展实现
└── test/main.cpp                   # 测试程序
└── docs/API.md                       # 接口文档
`

> 完整接口说明（宏定义、类型、全部接口签名、示例）见 [docs/API.md](docs/API.md)。``

调用方通过 `#include "pelco_d/PelcoDCodec.h"` 引用（该头文件已包含 `PelcoDPayload.h`），CMake 已通过 `target_include_directories` 导出 include 目录。

所有类型与接口均位于 `pelco_d` 命名空间中，命令宏使用 `DEF_PELCO_D_` 前缀，避免与其他代码冲突。


## 附加协议（PTZ Extended Protocol V0.1）

适配基于 Pelco-D 的云台附加协议，类型体系体现"附加协议基于标准协议"的继承关系：
- 载荷：`CPelcoDExtPayload` 继承 `CPelcoDPayload`（扩展帧载荷）
- 编解码：`CPelcoDExtCodec` 继承 `CPelcoDCodec`，复用基类校验和与组帧接口

### 帧格式

| 同步字节 | 云台地址 | 命令一 | 命令二 | 数据一 | 数据二 | 校验值 |
| --- | --- | --- | --- | --- | --- | --- |
| 0xFF | ADDR | COMD1 | COMD2 | DATA1 | DATA2 | CKSUM |

基础帧为 **7 字节**；位置类指令（0xF5 云台回复 / 0xF9 写位置）为 **9 字节**（数据四字节：水平高/低、俯仰高/低）。
帧长由 COMD2 判定：`0xF5`/`0xF9` 为 9 字节，其余 7 字节。校验和沿用 Pelco-D 算法（地址+命令+数据累加取低 8 位）。

### 命令定义

| 命令值（COMD2） | 含义 | 帧长 | 说明 |
| --- | --- | --- | --- |
| 0xF7 | 读位置 | 7 字节 | 主机发送，同时读取水平/俯仰位置 |
| 0xF5 | 位置回复 | 9 字节 | 云台回复：水平=DATA1<<8+DATA2，俯仰=DATA3<<8+DATA4 |
| 0xF9 | 写位置 | 9 字节 | 向指定位置以设定速度直线运行，云台不回复 |
| 0x85 | 预置点间速度 | 7 字节 | 巡航模式运行速度放 DATA2 |
| 0x87 | 物理校零 | 7 字节 | 水平/俯仰运行至物理零点（水平 0，垂直 27000） |

位置范围：水平 0~35999；俯仰 0~9000 与 27000~35999。

### 便捷接口

| 分类 | 接口 | 说明 |
| --- | --- | --- |
| 通用 | `BuildCommandFrame(byAddr, byCmd1, byCmd2, byData1, byData2, out)` | 任意命令组帧（7 字节） |
| 查询 | `BuildQueryPositionFrame(byAddr, out)` | 读水平/俯仰位置（0xF7） |
| 位置 | `BuildSetPositionFrame(byAddr, wPanPos, wTiltPos, out)` | 写位置直线运行（0xF9） |
| 速度 | `BuildSetPresetSpeedFrame(byAddr, bySpeed, out)` | 预置点间速度设定（0x85） |
| 校零 | `BuildCalibrateZeroFrame(byAddr, out)` | 物理校零（0x87） |
| 解析 | `GetPanPosition(payload)` / `GetTiltPosition(payload)` | 从回复帧提取位置 |

拆帧接口 `PopFrame` / `PopFrames` 与标准协议一致：从 `std::deque<uint8_t>` 缓冲区解析完整帧并移除，非法数据安全跳过，不足一帧保留等待后续数据。

### 使用示例

```cpp
#include "pelco_d/PelcoDExtCodec.h"

using namespace pelco_d;

std::vector<uint8_t> vecFrame;
// 查询位置（7 字节帧）
CPelcoDExtCodec::BuildQueryPositionFrame(0x01, vecFrame);
// 写入位置：水平 0x1234、俯仰 0xABCD（9 字节帧）
CPelcoDExtCodec::BuildSetPositionFrame(0x01, 0x1234, 0xABCD, vecFrame);

std::deque<uint8_t> dequeBuffer;
CPelcoDExtPayload payload;
if (CPelcoDExtCodec::PopFrame(dequeBuffer, payload))
{
    // 云台回复帧 0xF5：提取位置
    uint16_t wPan = CPelcoDExtCodec::GetPanPosition(payload);
    uint16_t wTilt = CPelcoDExtCodec::GetTiltPosition(payload);
}
```

## 组帧拆帧

- `CPelcoDPayload`：标准帧载荷（地址、命令、数据），不含同步字节与校验和
- `CPelcoDExtPayload`：扩展帧载荷，继承 `CPelcoDPayload`
- `CPelcoDCodec::BuildFrame`：组帧，输出完整 7 字节帧（同步字节 0xFF + 载荷 + 校验和）
- `CPelcoDCodec::PopFrame`：从 `std::deque<uint8_t>` 缓冲区解析一帧完整数据，解析成功后从缓冲区移除
- `CPelcoDCodec::PopFrames`：从缓冲区解析出全部完整帧

拆帧规则：

- 非同步字节（0xFF）直接跳过
- 校验和错误的帧整体跳过，继续查找下一帧
- 数据不足一帧时保留缓冲区内容，等待更多数据后再次调用
- 帧内数据字节可为 0xFF，不会误判为同步头

## 命令定义

命令由 `m_byCmd1` / `m_byCmd2` 两个字节的位标志组合而成，可按位叠加：

| 字节   | Bit7 | Bit6 | Bit5 | Bit4            | Bit3      | Bit2     | Bit1     | Bit0     |
| ------ | ---- | ---- | ---- | --------------- | --------- | -------- | -------- | -------- |
| 命令 1 | 感测位 | 保留 | 保留 | 自动/手动扫描    | 相机关闭  | 关光圈   | 开光圈   | 近焦     |
| 命令 2 | 远焦  | 缩小 | 放大 | 下移            | 上移      | 左移     | 右移     | 固定 0   |

扩展命令（`m_byCmd1 = 0x00`，直接赋给 `m_byCmd2`）：

| 命令         | 值    | 说明                                   |
| ------------ | ----- | -------------------------------------- |
| 设置预置位   | 0x03  | 预置位号放 `m_byData2`                  |
| 清除预置位   | 0x05  | 预置位号放 `m_byData2`                  |
| 调用预置位   | 0x07  | 预置位号放 `m_byData2`                  |
| 打开辅助开关 | 0x09  | 辅助号 1~8 放 `m_byData2`               |
| 关闭辅助开关 | 0x0B  | 辅助号 1~8 放 `m_byData2`               |
| 远程复位     | 0x0F  | -                                      |
| 停止巡迹     | 0x21  | -                                      |
| 运行巡迹     | 0x23  | 巡迹号放 `m_byData2`                    |
| 设置变焦速度 | 0x25  | 速度 0~3 放 `m_byData2`                 |
| 设置聚焦速度 | 0x27  | 速度 0~3 放 `m_byData2`                 |
| 相机恢复默认 | 0x29  | -                                      |
| 设置零位     | 0x49  | -                                      |
| 设置水平位置 | 0x4B  | `m_byData1`=高字节，`m_byData2`=低字节  |
| 设置垂直位置 | 0x4D  | `m_byData1`=高字节，`m_byData2`=低字节  |
| 设置变焦位置 | 0x4F  | `m_byData1`=高字节，`m_byData2`=低字节  |
| 查询水平位置 | 0x51  | -                                      |
| 查询垂直位置 | 0x53  | -                                      |
| 查询变焦位置 | 0x55  | -                                      |
| 查询版本信息 | 0x73  | -                                      |

示例：右转 = `m_byCmd1=0x00`、`m_byCmd2=DEF_PELCO_D_CMD2_PAN_RIGHT`、水平速度放 `m_byData1`

## 便捷接口

常用操作已封装为便捷接口，直接产出完整 7 字节帧（校验和自动计算）：

| 类别 | 接口 | 说明 |
| ---- | ---- | ---- |
以下接口均位于 `pelco_d` 命名空间：

| 通用 | `BuildCommandFrame(byAddr, byCmd1, byCmd2, byData1, byData2, out)` | 任意命令组帧 |
| 云台 | `BuildMoveUpFrame/DownFrame(byAddr, byTiltSpeed, out)` | 上/下移，速度 0x00~0x3F |
| 云台 | `BuildMoveLeftFrame/RightFrame(byAddr, byPanSpeed, out)` | 左/右移，速度 0x00~0x3F |
| 云台 | `BuildStopFrame(byAddr, out)` | 停止移动 |
| 镜头 | `BuildZoomInFrame/OutFrame(byAddr, out)` | 放大/缩小 |
| 镜头 | `BuildFocusFarFrame/NearFrame(byAddr, out)` | 远焦/近焦 |
| 镜头 | `BuildIrisOpenFrame/CloseFrame(byAddr, out)` | 开/关光圈 |
| 预置位 | `BuildSetPresetFrame/ClearPresetFrame/CallPresetFrame(byAddr, byPresetId, out)` | 设置/清除/调用预置位 |
| 辅助 | `BuildSetAuxFrame/ClearAuxFrame(byAddr, byAuxId, out)` | 打开/关闭辅助开关（1~8） |
| 复位 | `BuildRemoteResetFrame(byAddr, out)` | 远程复位 |
| 查询 | `BuildQueryPanFrame/TiltFrame/ZoomFrame(byAddr, out)` | 查询水平/垂直/变焦位置 |

示例：

```cpp
std::vector<uint8_t> vecFrame;
pelco_d::CPelcoDCodec::BuildMoveRightFrame(0x01, 0x20, vecFrame);   // 右转，水平速度 0x20
pelco_d::CPelcoDCodec::BuildCallPresetFrame(0x01, 0x05, vecFrame);  // 调用 5 号预置位
pelco_d::CPelcoDCodec::BuildQueryPanFrame(0x01, vecFrame);          // 查询水平位置
```

## 帧格式

| 字节 | 内容                       |
| ---- | -------------------------- |
| 0    | 同步字节 0xFF              |
| 1    | 设备地址                   |
| 2    | 命令字节 1                 |
| 3    | 命令字节 2                 |
| 4    | 数据 1（水平速度）         |
| 5    | 数据 2（垂直速度）         |
| 6    | 校验和（字节 1~5 累加，取低 8 位） |

## 构建与测试

默认生成**动态库**（Windows 下为 `pelco_d.dll`，另可选生成静态库）：

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure   # 或直接运行 build/bin/pelco_d_test(.exe)
```

生成**静态库**（`libpelco_d.a`）：

```sh
cmake -S . -B build-static -DPELCO_D_BUILD_STATIC=ON
cmake --build build-static
ctest --test-dir build-static --output-on-failure
```

动态库模式在 Windows 下通过 `PELCO_D_API` 导出符号（构建期由 CMake 定义 `PELCO_D_BUILD_SHARED`）；
外部使用方链接动态库可定义 `PELCO_D_USE_SHARED` 导入符号，静态链接无需任何定义。
