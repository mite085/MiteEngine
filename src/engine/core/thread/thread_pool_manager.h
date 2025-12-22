#ifndef MITE_CORE_THREAD_POOL_MANAGER_H
#define MITE_CORE_THREAD_POOL_MANAGER_H

#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "logger/logger.h"
#include "thread_pool_config.h"

namespace mite {
/**
 * @brief 线程池管理器 - 核心类
 *
 * 负责统一管理系统中所有的线程池资源，避免重复创建和资源浪费
 * 支持多种配置的线程池和命名线程池，提供统一的资源管理接口
 *
 * 设计目标：
 * 1. 统一管理所有线程池资源
 * 2. 避免线程池重复创建
 * 3. 支持多种配置选项
 * 4. 提供线程安全的访问接口
 * 5. 支持生命周期管理
 *
 * 使用示例：
 *
 * // 获取默认线程池
 * auto& default_pool = ThreadPoolManager::GetDefaultPool();
 *
 * // 获取指定配置的线程池
 * auto& high_perf_pool =
 * ThreadPoolManager::GetPool<ThreadPoolConfig::HIGH_PERFORMANCE_FLAGS>();
 *
 * // 获取命名专用线程池
 * auto& render_pool = ThreadPoolManager::GetNamedPool("render", 2);
 */
class ThreadPoolManager {
 public:
  // 删除拷贝构造函数和赋值运算符
  ThreadPoolManager(const ThreadPoolManager &) = delete;
  ThreadPoolManager &operator=(const ThreadPoolManager &) = delete;

  /**
   * @brief 获取默认全局线程池（使用默认配置）
   * @return 默认线程池的引用
   */
  static BS::thread_pool<ThreadPoolConfig::DEFAULT_FLAGS> &GetDefaultPool();

  /**
   * @brief 获取指定配置的线程池
   * @tparam OptFlags 线程池配置标志
   * @return 指定配置线程池的引用
   */
  template <BS::tp OptFlags = ThreadPoolConfig::DEFAULT_FLAGS>
  static BS::thread_pool<OptFlags> &GetPool();

  /**
   * @brief 获取指定名称的专用线程池
   * @tparam OptFlags 线程池配置标志
   * @param name 线程池名称
   * @param thread_count 线程数量，0表示自动检测
   * @return 命名线程池的引用
   */
  template <BS::tp OptFlags = ThreadPoolConfig::DEFAULT_FLAGS>
  static BS::thread_pool<OptFlags> &GetNamedPool(
      const std::string &name,
      size_t thread_count = ThreadPoolConfig::ThreadCounts::AUTO);

  /**
   * @brief 初始化所有线程池
   *
   * 可选调用，用于预初始化常用线程池
   */
  static void Initialize();

  /**
   * @brief 关闭所有线程池
   *
   * 程序退出时调用，确保所有线程正确退出
   */
  static void Shutdown();

  /**
   * @brief 获取系统建议的线程数量
   * @return 推荐的线程数量
   */
  static size_t GetRecommendedThreadCount();

  /**
   * @brief 检查指定名称的线程池是否存在
   * @tparam OptFlags 线程池配置标志
   * @param name 线程池名称
   * @return 是否存在
   */
  template <BS::tp OptFlags = ThreadPoolConfig::DEFAULT_FLAGS>
  static bool HasNamedPool(const std::string &name);

  /**
   * @brief 移除指定名称的线程池
   * @tparam OptFlags 线程池配置标志
   * @param name 线程池名称
   */
  template <BS::tp OptFlags = ThreadPoolConfig::DEFAULT_FLAGS>
  static void RemoveNamedPool(const std::string &name);

  /**
   * @brief 获取所有命名线程池的数量
   * @tparam OptFlags 线程池配置标志
   * @return 线程池数量
   */
  template <BS::tp OptFlags = ThreadPoolConfig::DEFAULT_FLAGS>
  static size_t GetNamedPoolCount();

 private:
  /**
   * @brief 私有构造函数，确保单例模式
   */
  ThreadPoolManager() = default;

  /**
   * @brief 默认线程池实现
   * @tparam OptFlags 线程池配置标志
   */
  template <BS::tp OptFlags>
  static BS::thread_pool<OptFlags> &GetDefaultPoolImpl();

  /**
   * @brief 命名线程池实现
   * @tparam OptFlags 线程池配置标志
   */
  template <BS::tp OptFlags>
  static BS::thread_pool<OptFlags> &GetNamedPoolImpl(const std::string &name,
                                                     size_t thread_count);

  // 线程安全互斥锁
  static std::mutex s_Mutex;

  /**
   * @brief 命名线程池存储模板
   * @tparam OptFlags 线程池配置标志
   */
  template <BS::tp OptFlags>
  struct NamedPoolStorage {
    static std::unordered_map<std::string,
                              std::unique_ptr<BS::thread_pool<OptFlags>>>
        pools;
  };
};

// 模板静态成员定义
template <BS::tp OptFlags>
std::unordered_map<std::string, std::unique_ptr<BS::thread_pool<OptFlags>>>
    ThreadPoolManager::NamedPoolStorage<OptFlags>::pools;

// 模板方法实现
template <BS::tp OptFlags>
BS::thread_pool<OptFlags> &ThreadPoolManager::GetPool() {
  return GetDefaultPoolImpl<OptFlags>();
}

template <BS::tp OptFlags>
BS::thread_pool<OptFlags> &ThreadPoolManager::GetNamedPool(
    const std::string &name, size_t thread_count) {
  return GetNamedPoolImpl<OptFlags>(name, thread_count);
}

template <BS::tp OptFlags>
BS::thread_pool<OptFlags> &ThreadPoolManager::GetDefaultPoolImpl() {
  static BS::thread_pool<OptFlags> default_pool(GetRecommendedThreadCount());
  return default_pool;
}

template <BS::tp OptFlags>
BS::thread_pool<OptFlags> &ThreadPoolManager::GetNamedPoolImpl(
    const std::string &name, size_t thread_count) {
  std::lock_guard<std::mutex> lock(s_Mutex);

  auto &pools = NamedPoolStorage<OptFlags>::pools;
  auto it = pools.find(name);

  if (it == pools.end()) {
    size_t actual_thread_count =
        thread_count > 0 ? thread_count : GetRecommendedThreadCount();
    auto pool =
        std::make_unique<BS::thread_pool<OptFlags>>(actual_thread_count);
    auto result = pools.emplace(name, std::move(pool));
    it = result.first;

    // 日志记录线程池创建
    LOG_INFO("Created named thread pool: {} with {} threads", name,
             actual_thread_count);
  }

  return *it->second;
}

template <BS::tp OptFlags>
bool ThreadPoolManager::HasNamedPool(const std::string &name) {
  std::lock_guard<std::mutex> lock(s_Mutex);
  auto &pools = NamedPoolStorage<OptFlags>::pools;
  return pools.find(name) != pools.end();
}

template <BS::tp OptFlags>
void ThreadPoolManager::RemoveNamedPool(const std::string &name) {
  std::lock_guard<std::mutex> lock(s_Mutex);
  auto &pools = NamedPoolStorage<OptFlags>::pools;
  pools.erase(name);

  // 日志记录线程池移除
  LOG_INFO("Removed named thread pool: {}", name);
}

template <BS::tp OptFlags>
size_t ThreadPoolManager::GetNamedPoolCount() {
  std::lock_guard<std::mutex> lock(s_Mutex);
  auto &pools = NamedPoolStorage<OptFlags>::pools;
  return pools.size();
}
}  // namespace mite

#endif  // MITE_CORE_THREAD_POOL_MANAGER_H
