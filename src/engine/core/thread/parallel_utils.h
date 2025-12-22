#ifndef MITE_CORE_PARALLEL_UTILS_H
#define MITE_CORE_PARALLEL_UTILS_H

#include <functional>
#include <future>
#include <iterator>
#include <type_traits>
#include <vector>

#include "logger/logger.h"
#include "thread_pool_manager.h"

namespace mite {
/**
 * @brief 并行处理工具类
 *
 * 提供通用的并行处理函数，简化容器元素的并行操作
 *
 * 示例1：处理std::vector
 * std::vector<int> numbers = {1, 2, 3, 4, 5};
 * ParallelUtils::ForEach(numbers, [](int& num) {
 *     num *= 2; // 并行处理每个元素
 * });
 *
 * 示例2：处理std::list
 * std::list<std::string> strings = {"a", "b", "c"};
 * ParallelUtils::ForEach(strings, [](std::string& str) {
 *     str += "_processed";
 * });
 *
 * 示例3：使用迭代器范围
 * std::vector<Entity*> entities = GetEntities();
 * ParallelUtils::ForEachRange(entities.begin(), entities.end(), [](Entity*
 * entity) { entity->Process();
 * });
 *
 * 示例4：带索引的处理
 * std::vector<Component*> components = GetComponents();
 * ParallelUtils::ForEachIndexed(components, [](Component* comp, size_t index) {
 *     comp->SetIndex(index);
 * });
 */
class ParallelUtils {
 public:
  // 删除拷贝构造函数和赋值运算符
  ParallelUtils(const ParallelUtils &) = delete;
  ParallelUtils &operator=(const ParallelUtils &) = delete;

  /**
   * @brief 并行处理容器中的所有元素
   * @tparam Container 容器类型
   * @tparam Func 处理函数类型
   * @param container 要处理的容器
   * @param func 处理每个元素的函数
   * @param batch_size 批处理大小，0表示自动计算
   */
  template <typename Container, typename Func>
  static void ForEach(Container &container, Func func, size_t batch_size = 0) {
    if (container.empty()) {
      return;
    }

    auto &thread_pool = ThreadPoolManager::GetDefaultPool();
    const size_t element_count = container.size();
    const size_t thread_count = thread_pool.get_thread_count();

    // 如果元素数量较少或只有一个线程，使用串行处理
    if (element_count <= 1 || thread_count <= 1) {
      for (auto &element : container) {
        func(element);
      }
      return;
    }

    // 自动计算批处理大小
    if (batch_size == 0) {
      batch_size = std::max<size_t>(1, element_count / thread_count);
    }

    std::vector<std::future<void>> futures;

    // 使用迭代器进行批处理
    auto begin = std::begin(container);
    auto end = std::end(container);

    for (auto it = begin; it != end;) {
      auto batch_start = it;
      auto batch_end = it;

      // 前进batch_size个元素，但不能超过end
      std::advance(batch_end,
                   std::min<size_t>(batch_size, std::distance(it, end)));

      futures.push_back(
          thread_pool.submit_task([batch_start, batch_end, &func]() {
            for (auto elem_it = batch_start; elem_it != batch_end; ++elem_it) {
              func(*elem_it);
            }
          }));

      it = batch_end;
    }

    // 等待所有任务完成
    WaitForFutures(futures);
  }

  /**
   * @brief 并行处理迭代器范围内的元素
   * @tparam Iterator 迭代器类型
   * @tparam Func 处理函数类型
   * @param begin 起始迭代器
   * @param end 结束迭代器
   * @param func 处理每个元素的函数
   * @param batch_size 批处理大小
   */
  template <typename Iterator, typename Func>
  static void ForEachRange(Iterator begin, Iterator end, Func func,
                           size_t batch_size = 0) {
    const size_t element_count = std::distance(begin, end);
    if (element_count == 0) {
      return;
    }
    BS::thread_pool<ThreadPoolConfig::DEFAULT_FLAGS> &thread_pool =
        ThreadPoolManager::GetDefaultPool();
    const size_t thread_count = thread_pool.get_thread_count();
    if (element_count <= 1 || thread_count <= 1) {
      for (auto it = begin; it != end; ++it) {
        func(*it);
      }
      return;
    }
    if (batch_size == 0) {
      batch_size = std::max<size_t>(1, element_count / thread_count);
    }
    std::vector<std::future<void>> futures;
    Iterator current = begin;
    while (current != end) {
      Iterator batch_start = current;
      Iterator batch_end = current;
      size_t advance_count =
          std::min<size_t>(batch_size, std::distance(current, end));
      std::advance(batch_end, advance_count);
      // 使用 detach_task() 替代 submit() 提交任务
      futures.push_back(
          thread_pool.detach_task([batch_start, batch_end, &func]() {
            for (auto it = batch_start; it != batch_end; ++it) {
              func(*it);
            }
          }));
      current = batch_end;
    }
    WaitForFutures(futures);
  }

  /**
   * @brief 并行处理容器元素（带索引版本）
   * @tparam Container 容器类型
   * @tparam Func 处理函数类型（接受元素和索引）
   */
  template <typename Container, typename Func>
  static void ForEachIndexed(Container &container, Func func,
                             size_t batch_size = 0) {
    if (container.empty()) {
      return;
    }
    auto &thread_pool = ThreadPoolManager::GetDefaultPool();
    const size_t element_count = container.size();
    const size_t thread_count = thread_pool.get_thread_count();
    if (element_count <= 1 || thread_count <= 1) {
      for (size_t i = 0; i < element_count; ++i) {
        func(container[i], i);
      }
      return;
    }
    if (batch_size == 0) {
      batch_size = std::max<size_t>(1, element_count / thread_count);
    }
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < element_count; i += batch_size) {
      size_t start = i;
      size_t end = std::min(i + batch_size, element_count);

      // 使用值捕获，确保线程安全
      futures.push_back(
          thread_pool.detach_task([&container, start, end, func]() {
            for (size_t j = start; j < end; ++j) {
              func(container[j], j);
            }
          }));
    }
    WaitForFutures(futures);
  }

 private:
  /**
   * @brief 等待所有future完成并处理异常
   */
  static void WaitForFutures(std::vector<std::future<void>> &futures) {
    for (auto &future : futures) {
      try {
        future.get();
      } catch (const std::exception &e) {
        LOG_ERROR("Parallel processing error: {}", e.what());
      } catch (...) {
        LOG_ERROR("Unknown error in parallel processing");
      }
    }
  }
};
}  // namespace mite

#endif  // MITE_CORE_PARALLEL_UTILS_H
