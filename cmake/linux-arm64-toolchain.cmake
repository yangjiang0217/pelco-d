# ============================================================================
# @file        linux-arm64-toolchain.cmake
# @version     1.0.0
# @date        2026-09-18
# @author      yangjiang
# @brief       Linux aarch64 交叉编译工具链（GitHub Actions ubuntu-latest）
# @details     配合 .github/workflows/build.yml 的 Linux-arm64 job 使用，
#              编译器由该 job 的 apt 步骤安装（gcc/g++-aarch64-linux-gnu）。
# ============================================================================

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_AR aarch64-linux-gnu-ar)
set(CMAKE_RANLIB aarch64-linux-gnu-ranlib)
set(CMAKE_STRIP aarch64-linux-gnu-strip)

# 仅查找目标系统的库与头文件（宿主工具仍用本机程序）
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
