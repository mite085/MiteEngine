#include "thread_pool_manager.h"

#include <thread>

#include "logger/logger.h"

namespace mite {
// 静态成员初始化
std::mutex ThreadPoolManager::s_Mutex;

BS::thread_pool<ThreadPoolConfig::DEFAULT_FLAGS> &
ThreadPoolManager::GetDefaultPool() {
  return GetDefaultPoolImpl<ThreadPoolConfig::DEFAULT_FLAGS>();
}

size_t ThreadPoolManager::GetRecommendedThreadCount() {
  // 获取硬件并发线程数
  size_t hardware_threads = std::thread::hardware_concurrency();

  // 如果无法检测到硬件并发数，使用保守的默认值
  if (hardware_threads == 0) {
    // 根据常见CPU核心数设置合理的默认值
    // 避免创建过多线程导致上下文切换开销
    return 4;  // 默认4个线程
  }

  // 预留一个线程给主线程或其他系统任务
  // 避免所有核心都被线程池占用导致系统响应变慢
  return std::max<size_t>(1, hardware_threads - 1);
}

void ThreadPoolManager::Initialize() {
  // 预初始化默认线程池，避免首次使用时的创建延迟
  GetDefaultPool();

  // 可选：预初始化其他常用线程池
  // GetPool<ThreadPoolConfig::HIGH_PERFORMANCE_FLAGS>();
  // GetNamedPool(ThreadPoolConfig::PoolNames::RENDER, 2);
  // GetNamedPool(ThreadPoolConfig::PoolNames::IO, 2);

  // 日志记录初始化完成
  LOG_INFO("ThreadPoolManager initialized with recommended thread count: {}",
           GetRecommendedThreadCount());
}

void ThreadPoolManager::Shutdown() {
  std::lock_guard<std::mutex> lock(s_Mutex);

  // 关闭所有默认配置的命名线程池
  auto &default_pools =
      NamedPoolStorage<ThreadPoolConfig::DEFAULT_FLAGS>::pools;
  for (auto &[name, pool] : default_pools) {
    if (pool) {
      pool->wait();  // 等待所有任务完成
      LOG_DEBUG("Waiting for thread pool {} to complete tasks", name);
    }
  }
  default_pools.clear();

  // 关闭高性能配置的命名线程池
  auto &high_perf_pools =
      NamedPoolStorage<ThreadPoolConfig::HIGH_PERFORMANCE_FLAGS>::pools;
  for (auto &[name, pool] : high_perf_pools) {
    if (pool) {
      pool->wait();
    }
  }
  high_perf_pools.clear();

  // 关闭安全优先配置的命名线程池
  auto &safety_pools =
      NamedPoolStorage<ThreadPoolConfig::SAFETY_FIRST_FLAGS>::pools;
  for (auto &[name, pool] : safety_pools) {
    if (pool) {
      pool->wait();
    }
  }
  safety_pools.clear();

  // 关闭简单配置的命名线程池
  auto &simple_pools = NamedPoolStorage<ThreadPoolConfig::SIMPLE_FLAGS>::pools;
  for (auto &[name, pool] : simple_pools) {
    if (pool) {
      pool->wait();
    }
  }
  simple_pools.clear();

  // 日志记录关闭完成
  LOG_INFO("ThreadPoolManager shutdown completed");
}
}  // namespace mite