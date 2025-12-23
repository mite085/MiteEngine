#ifndef MITE_CORE_THREAD_POOL_CONFIG_H
#define MITE_CORE_THREAD_POOL_CONFIG_H

#include "BS_thread_pool.hpp"

namespace mite {
/**
 * @brief 线程池配置常量定义
 *
 * 提供预定义的线程池配置选项，便于统一管理和使用
 */
namespace ThreadPoolConfig {
/**
 * @brief 默认线程池配置标志
 *
 * 包含：
 * - BS::tp::priority: 启用任务优先级支持
 * - BS::tp::wait_deadlock_checks: 启用等待时的死锁检查
 */
constexpr BS::tp DEFAULT_FLAGS =
    static_cast<BS::tp>(BS::tp::priority | BS::tp::wait_deadlock_checks);

/**
 * @brief 高性能线程池配置标志
 *
 * 适用于计算密集型任务，仅启用优先级支持
 */
constexpr BS::tp HIGH_PERFORMANCE_FLAGS = BS::tp::priority;

/**
 * @brief 安全优先线程池配置标志
 *
 * 适用于需要严格错误检查的场景，启用所有安全检查
 */
constexpr BS::tp SAFETY_FIRST_FLAGS =
    static_cast<BS::tp>(BS::tp::priority | BS::tp::wait_deadlock_checks);

/**
 * @brief 简单线程池配置标志
 *
 * 无额外功能，适用于简单场景
 */
constexpr BS::tp SIMPLE_FLAGS = BS::tp::none;

/**
 * @brief 预定义的线程池名称常量
 */
namespace PoolNames {
constexpr const char *DEFAULT = "default";        // 默认线程池
constexpr const char *RENDER = "render";          // 渲染线程池
constexpr const char *PHYSICS = "physics";        // 物理线程池
constexpr const char *IO = "io";                  // IO操作线程池
constexpr const char *NETWORK = "network";        // 网络线程池
constexpr const char *BACKGROUND = "background";  // 后台任务线程池
}  // namespace PoolNames

/**
 * @brief 预定义的线程数量配置
 */
namespace ThreadCounts {
constexpr size_t AUTO = 0;       // 自动检测硬件并发数
constexpr size_t SINGLE = 1;     // 单线程
constexpr size_t DOUBLE = 2;     // 双线程
constexpr size_t QUADRUPLE = 4;  // 四线程
constexpr size_t OCTA = 8;       // 八线程
}  // namespace ThreadCounts
}  // namespace ThreadPoolConfig
}  // namespace mite

#endif  // MITE_CORE_THREAD_POOL_CONFIG_H
