#ifndef MITE_CORE_EVENT_DISPATCHER
#define MITE_CORE_EVENT_DISPATCHER

#include <functional>
#include "event/event.h"

namespace mite {

template<typename T> using EventFn = std::function<void(T &)>;

/**
 * @brief 事件分发器类
 *
 * 用于将事件分发给对应的处理函数，确保类型安全的事件处理
 */
class EventDispatcher {
 public:
  /**
   * @brief 默认构造函数
   *
   * 创建一个未关联任何事件的分发器
   */
  EventDispatcher() = default;

  /**
   * @brief 构造函数（带事件引用）
   * @param event 要分发的事件引用
   *
   * 创建一个与指定事件关联的分发器
   */
  explicit EventDispatcher(Event &event) : m_Event(&event) {}

  /**
   * @brief 设置当前要分发的事件
   * @param event 要分发的事件引用
   *
   * 可以在分发器创建后重新设置关联的事件
   */
  void SetEvent(Event &event)
  {
    m_Event = &event;
  }

  /**
   * @brief 分发事件到指定类型的处理函数
   * @tparam T 具体的事件类型
   * @param func 事件处理函数，接受T类型事件
   * @return bool 是否成功分发（事件类型匹配时返回true）
   *
   * 1. 检查当前事件是否与模板类型T匹配
   * 2. 如果匹配，将事件转换为具体类型并调用处理函数
   * 3. 将处理函数的返回值设置到事件的handled标志（该步骤删除，由func自主决定是否handled）
   */
  template<typename T> bool Dispatch(std::function<void(T &)> func)
  {
    // 检查是否有有效事件且事件类型匹配
    if (m_Event && m_Event->GetEventType() == T::GetStaticType()) {
      // 将基类Event转换为具体事件类型T
      // 调用处理函数
      func(static_cast<T &>(*m_Event));
      return true;  // 分发成功
    }
    return false;  // 事件类型不匹配，分发失败
  }

 private:
  Event *m_Event = nullptr;  ///< 指向当前要分发的事件对象的指针
};

}  // namespace mite

/**
 * @brief 辅助宏：用于 EventDispatcher 分发（保持原有类型安全）
 *
 * 使用示例（订阅事件时）：
 * auto handlerId = EventBus::Get().Subscribe<WindowResizeEvent>(
 *  BIND_DISPATCH_FN(OnWindowResized)
 * );
 * 此时，BIND_DISPATCH_FN(OnWindowResized) 等价于 [this](auto&& e) { OnWindowResized(e); }
 */
#define BIND_DISPATCH_FN(fn) [this](auto &&event) -> bool { return this->fn(event); }


#endif
