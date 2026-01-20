# 1. 指定目标系统：Windows（核心，区别于 Linux 主机）
set(CMAKE_SYSTEM_NAME Windows)
# 2. 指定目标架构：x64（可选改为 i686 对应 32 位 Windows）
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# 3. 指定 MinGW-w64 交叉编译器路径（无需完整路径，系统已添加到 PATH）
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_AR ${TOOLCHAIN_PREFIX}-ar)
set(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}-ranlib)
set(CMAKE_LINKER ${TOOLCHAIN_PREFIX}-ld)

# 4. 禁用本地（Linux）库和可执行文件查找，仅使用交叉编译工具链的资源
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# 5. 配置 Windows 交叉编译专属选项（适配 fswatcher 和 libuv）
# 5.1 定义 Windows 版本宏，确保 libuv 兼容 Windows 平台
add_compile_definitions(_WIN32_WINNT=0x0601)  # 对应 Windows 7 及以上，兼容大部分场景
# 5.2 静态库编译选项（强制生成 Windows 静态库 .lib）
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
# 5.3 禁用 MinGW 特有警告，提升兼容性
add_compile_options(-Wno-unknown-pragmas -Wno-format)
# 5.4 Windows 平台需要链接的系统库（libuv 依赖这些库，必须添加）
set(WIN32_SYSTEM_LIBS ws2_32 iphlpapi psapi shell32 user32 advapi32 kernel32 ntdll)
link_directories(/usr/${TOOLCHAIN_PREFIX}/lib)