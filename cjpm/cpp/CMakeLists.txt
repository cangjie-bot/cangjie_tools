# 1. 指定 CMake 最低版本（兼容大部分平台，建议 3.10+）
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# 2. 定义项目名称和编程语言（支持 C/C++，因为 libuv 是 C 库，fswatcher 是 C++ 实现）
project(fswatcher 
    LANGUAGES C CXX
    DESCRIPTION "Cross-platform file watcher static library based on libuv")

# 3. 配置跨平台编译选项
# 3.1 指定 C++ 标准（兼容 C++11 及以上，按需调整）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 3.2 跨平台编译选项优化（消除警告、适配平台）
if(MSVC)  # Windows 平台（MSVC 编译器）
    # 关闭 MSVC 不必要的警告
    add_compile_options(/W3 /wd4996 /wd4244)
    # 支持 UTF-8 编码
    add_compile_options(/utf-8)
else()  # Linux/macOS 平台（GCC/Clang 编译器）
    # 开启更多警告，提升代码质量
    add_compile_options(-Wall -Wextra -Wpedantic -Werror=return-type)
    # 静态库编译优化
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# 3.3 配置静态库输出目录（统一输出路径，方便查找）
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # 静态库输出目录

# 4. 引入 libuv 依赖（嵌入式方式，优先构建 libuv 静态库）
# 4.1 检查 libuv 源码目录是否存在
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/third_party/libuv/CMakeLists.txt)
    message(FATAL_ERROR "libuv source not found! Please clone libuv to third_party/libuv first.")
endif()

# 4.2 添加 libuv 子目录，构建 libuv 静态库（libuv 自带 CMake 支持，会生成 uv 静态库）
add_subdirectory(third_party/libuv EXCLUDE_FROM_ALL)

# 5. 收集 fswatcher 源码和头文件
# 5.1 收集 src 目录下的 C++ 源码文件
file(GLOB_RECURSE FSWATCHER_SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# 6. 生成 fswatcher 静态库（核心目标：STATIC 表示静态库）
add_library(fswatcher STATIC
    ${FSWATCHER_SRC_FILES}
)

# 7. 配置 fswatcher 静态库的编译依赖
# 7.1 指定头文件包含目录（PUBLIC：外部使用该库时，也会包含这些目录；PRIVATE：仅内部编译使用）
target_include_directories(fswatcher
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src      # 源码目录（内部包含，可选）
        ${CMAKE_SOURCE_DIR}/third_party/libuv/include  # libuv 头文件目录（私有依赖）
)

# 7.2 链接 libuv 静态库（uv 是 libuv 构建后生成的目标名，跨平台兼容）
target_link_libraries(fswatcher
    PRIVATE
        uv  # 链接 libuv 静态库（确保 libuv 已生成 uv 目标）
)

# 8. 可选：设置静态库属性（提升跨平台兼容性）
set_target_properties(fswatcher PROPERTIES
    OUTPUT_NAME "fswatcher"  # 静态库名称（最终生成 fswatcher.a / fswatcher.lib）
    ARCHIVE_OUTPUT_NAME "fswatcher"  # 归档库（静态库）名称
    DEBUG_POSTFIX "d"  # 调试版本后缀（可选，如 fswatcherd.a）
)