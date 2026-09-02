/* ============================================================
 * RPLIDAR SDK MinGW 兼容层（强制前置包含）
 *
 * 背景：SDK 原生面向 MSVC，MSVC 把 nullptr_t 暴露在全局命名空间。
 * MinGW GCC 的 nullptr_t 只在 std:: 命名空间（std::nullptr_t），
 * 而 sl_async_transceiver.cpp 直接用 nullptr_t 不加 std:: 前缀，
 // 导致 MinGW 下编译失败。
 *
 * sl_lidar_driver.cpp 内部已用 _GXX_NULLPTR_T 守卫做了同样的 typedef，
 * 这里提前定义并置位 _GXX_NULLPTR_T：
 *   - sl_lidar_driver.cpp 检测到 _GXX_NULLPTR_T 已定义，跳过重复 typedef
 *   - sl_async_transceiver.cpp 可直接找到全局 nullptr_t
 *
 * 通过 CMake 的 -include 选项在编译每个 SDK 源文件前强制包含本头文件，
 * 不修改 SDK 任何源文件。
 * ============================================================ */

#ifndef RPLIDAR_COMPAT_FORCE_INCLUDE_HPP
#define RPLIDAR_COMPAT_FORCE_INCLUDE_HPP

#if defined(__cplusplus) && __cplusplus >= 201103L
  #ifndef _GXX_NULLPTR_T
    #define _GXX_NULLPTR_T
    typedef decltype(nullptr) nullptr_t;
  #endif
#endif

#endif /* RPLIDAR_COMPAT_FORCE_INCLUDE_HPP */
