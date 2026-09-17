# Pelco-D 协议组帧拆帧库
#
# 构建（默认生成动态库）：
#   cmake -S . -B build
#   cmake --build build
# 可选生成静态库：
#   cmake -S . -B build -DPELCO_D_BUILD_STATIC=ON
# 运行测试：
#   ctest --test-dir build --output-on-failure
# 或直接运行 build/bin/pelco_d_test(.exe)

cmake_minimum_required(VERSION 3.10)

project(pelco_d
    VERSION 1.0.0
    DESCRIPTION "Pelco-D protocol framing and unframing"
    LANGUAGES CXX
)

# 统一使用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译警告选项（按编译器区分）
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

# 构建产物统一输出到 bin/lib 目录；动态库与可执行文件同目录，构建后可直接运行
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
if(WIN32)
    # Windows：dll 归运行时目录（bin），导入库/静态库归 lib
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
else()
    # Linux/macOS：动态库与可执行文件同目录，避免构建后运行测试时找不到库
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# 可选生成静态库，默认生成动态库
option(PELCO_D_BUILD_STATIC "Build pelco_d as static library instead of shared" OFF)
if(PELCO_D_BUILD_STATIC)
    add_library(pelco_d STATIC src/PelcoDCodec.cpp src/PelcoDExtCodec.cpp)
else()
    add_library(pelco_d SHARED src/PelcoDCodec.cpp src/PelcoDExtCodec.cpp)
    # Windows 动态库导出符号（类声明中的 PELCO_D_API）
    target_compile_definitions(pelco_d PRIVATE PELCO_D_BUILD_SHARED)
endif()
target_include_directories(pelco_d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 安装规则：CI 发布工作流（.github/workflows/build.yml）用 cmake --install 收集产物
install(TARGETS pelco_d
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(FILES README.md docs/API.md DESTINATION share/doc/pelco_d)

# 测试程序：pelco_d_test
add_executable(pelco_d_test test/main.cpp)
target_link_libraries(pelco_d_test PRIVATE pelco_d)

# 注册测试
enable_testing()
add_test(NAME pelco_d_test COMMAND pelco_d_test)
