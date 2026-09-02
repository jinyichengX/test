#pragma once

// =============================================================================
// 数据解包器命名空间宏定义文件
// -----------------------------------------------------------------------------
// 本文件定义了数据解包器(dataunpacker)子系统所使用的命名空间包装宏。
// 所有 dataunpacker 模块中的类、函数都应当包裹在 sl::internal 命名空间内，
// 以避免与 SDK 其他模块（如驱动层 sl 命名空间）发生符号冲突。
//
// 使用方式：
//   在头文件/源文件开头使用 BEGIN_DATAUNPACKER_NS()，
//   在文件结尾使用 END_DATAUNPACKER_NS()，将所有定义包裹其中。
// =============================================================================

// 开始数据解包器命名空间：展开为 namespace sl{ namespace internal{
// sl 是 Slamtec LIDAR SDK 的顶层命名空间
// internal 表示这是 SDK 内部实现细节，不作为对外公开 API 的一部分
#define BEGIN_DATAUNPACKER_NS()  namespace sl{ namespace internal{

// 结束数据解包器命名空间：闭合两个 namespace 大括号
#define END_DATAUNPACKER_NS()  }}