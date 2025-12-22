#ifndef MITE_CORE_CALLBACK_ADAPTER
#define MITE_CORE_CALLBACK_ADAPTER

#include "event_bus.h"

namespace mite {
/**
 * @brief 抽象回调适配器基类
 * @tparam SourceType 原始回调数据来源类型（如GLFWwindow*, entt::registry*）
 */
template <typename SourceType>
class CallbackAdapter {
 public:
  explicit CallbackAdapter() {}
  virtual ~CallbackAdapter() = default;

  /**
   * @brief 注册所有回调到原始系统
   * @param source 原始系统对象指针
   */
  virtual void RegisterCallbacks(SourceType source) = 0;

  /**
   * @brief 注销所有回调
   */
  virtual void UnregisterCallbacks() = 0;
};
};  // namespace mite

#endif
