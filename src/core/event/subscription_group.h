#ifndef MITE_CORE_SUBSCRIPTION_GROUP
#define MITE_CORE_SUBSCRIPTION_GROUP 

#include "event_bus.h"

namespace mite {
/**
 * @brief 订阅组 - 用于集中管理多个事件订阅
 *
 * 提供RAII风格的事件订阅管理，自动在析构时取消所有订阅
 * 适用于需要管理多个事件订阅的类，避免手动跟踪和取消订阅
 * 
 * 使用示例：
 *
 * 1. 创建并发布事件（触发事件）：
 *      mite::EventBus::Get().Post(event);
 * 2. 订阅事件（在模块Initialize时订阅，onXxxEvent为处理该事件的逻辑）：
 *      m_EventSubscriptions.Subscribe<XxxEvent>(BIND_DISPATCH_FN(onXxxEvent));
 * 3. 触发事件（主循环调用ProcessQueue()自动触发onEvent函数）
 *      while (window.IsRunning()) {
 *          mite::EventBus::Get().ProcessQueue();
 *      }
 * 4. 取消订阅（在模块ShutDown时取消）：
 *      m_EventSubscriptions.UnsubscribeAll();
 */
class SubscriptionGroup {
 public:
  /**
   * @brief 构造函数
   * @param bus 事件总线引用
   */
  SubscriptionGroup() : m_EventBus(EventBus::Get()) {}

  /**
   * @brief 析构函数 - 自动取消所有订阅
   */
  ~SubscriptionGroup()
  {
    UnsubscribeAll();
  }

  // 禁止拷贝构造和赋值
  SubscriptionGroup(const SubscriptionGroup &) = delete;
  SubscriptionGroup &operator=(const SubscriptionGroup &) = delete;

  // 允许移动语义
  SubscriptionGroup(SubscriptionGroup &&) = default;
  SubscriptionGroup &operator=(SubscriptionGroup &&) = default;

  /**
   * @brief 添加事件订阅到组内
   * @tparam T 事件类型
   * @param handler 事件处理函数
   */
  template<typename T> void Subscribe(EventFn<T> handler)
  {
    m_Handlers.push_back(m_EventBus.Subscribe<T>(std::move(handler)));
  }

  /**
   * @brief 取消组内所有订阅
   */
  void UnsubscribeAll()
  {
    for (auto id : m_Handlers) {
      m_EventBus.Unsubscribe(id);
    }
    m_Handlers.clear();
  }

  /**
   * @brief 检查订阅组是否为空
   * @return bool 是否没有任何订阅
   */
  bool IsEmpty() const
  {
    return m_Handlers.empty();
  }

  /**
   * @brief 获取订阅数量
   * @return size_t 当前管理的订阅数
   */
  size_t Count() const
  {
    return m_Handlers.size();
  }

 private:
  EventBus &m_EventBus;                         // 事件总线引用
  std::vector<EventBus::HandlerID> m_Handlers;  // 存储所有订阅ID
};

};

#endif
