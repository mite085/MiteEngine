#ifndef MITE_CORE_EVENT_DISPATCHER
#define MITE_CORE_EVENT_DISPATCHER

#include "event.h"

namespace mite {
template <typename T>
using EventFn = std::function<void(T &)>;  // 事件处理函数类型（类型安全版本）
using EventHandler =
    std::function<void(Event &)>;  // 事件处理函数类型（通用版本）
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
  void SetEvent(Event &event) { m_Event = &event; }

  /**
   * @brief 分发事件到指定类型的处理函数
   * @tparam T 具体的事件类型
   * @param func 事件处理函数，接受T类型事件
   * @return bool 是否成功分发（事件类型匹配时返回true）
   *
   * 1. 检查当前事件是否与模板类型T匹配
   * 2. 如果匹配，将事件转换为具体类型并调用处理函数
   * 3.
   * 将处理函数的返回值设置到事件的handled标志（该步骤删除，由func自主决定是否handled）
   */
  template <typename T>
  bool Dispatch(EventFn<T> func) {
    static_assert(std::is_base_of<Event, T>::value,
                  "T must inherit from Event");

    // 检查是否有有效事件且事件类型匹配
    if (m_Event && typeid(*m_Event) == typeid(T)) {
      // 将基类Event转换为具体事件类型T
      func(static_cast<T &>(*m_Event));
      return true;  // 分发成功
    }
    return false;  // 事件类型不匹配，分发失败
  }
  /**
   * @brief 检查当前事件是否匹配指定类型
   * @tparam T 要检查的事件类型
   * @return 是否匹配
   */
  template <typename T>
  bool IsType() const {
    static_assert(std::is_base_of<Event, T>::value,
                  "T must inherit from Event");
    return m_Event && typeid(*m_Event) == typeid(T);
  }

  /**
   * @brief 获取当前事件的类型信息
   * @return 类型信息指针，如果没有事件则返回nullptr
   */
  const std::type_info *GetEventType() const {
    return m_Event ? &typeid(*m_Event) : nullptr;
  }
  /**
   * @brief 检查是否有有效的事件关联
   * @return 是否有关联的事件
   */
  bool HasEvent() const { return m_Event != nullptr; }

 private:
  Event *m_Event = nullptr;  // 指向当前要分发的事件对象的指针
};
}  // namespace mite

/**
 * @brief 辅助宏：用于 EventDispatcher 分发（保持原有类型安全）
 *
 * 使用示例（订阅事件时）：
 * auto handlerId = EventBus::Get().Subscribe<WindowResizeEvent>(
 *  BIND_DISPATCH_FN(OnWindowResized)
 * );
 * 此时，BIND_DISPATCH_FN(OnWindowResized) 等价于 [this](auto&& e) {
 * this->OnWindowResized(e); }
 */
#define BIND_DISPATCH_FN(fn) [this](auto &&event) { return this->fn(event); }
/**
 * @brief 辅助宏：用于静态函数的事件分发
 *
 * 使用示例：
 * auto handlerId = EventBus::Get().Subscribe<WindowResizeEvent>(
 *  BIND_DISPATCH_FN_STATIC(OnWindowResizedStatic)
 * );
 */
#define BIND_DISPATCH_FN_STATIC(fn) [](auto &&event) { fn(event); }
/**
 * @brief 辅助宏：用于成员函数的事件分发（指定对象）
 *
 * 使用示例：
 * auto handlerId = EventBus::Get().Subscribe<WindowResizeEvent>(
 *  BIND_DISPATCH_FN_OBJ(obj, &MyClass::OnWindowResized)
 * );
 */
#define BIND_DISPATCH_FN_OBJ(obj, fn) [obj](auto &&event) { (obj->*fn)(event); }
#endif
