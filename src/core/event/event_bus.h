#ifndef MITE_CORE_EVENT_BUS
#define MITE_CORE_EVENT_BUS

#include "event/dispatcher.h"
#include "event/event.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <variant>

namespace mite {
/**
 * @brief 事件总线系统 - 核心类
 *
 * 负责管理事件的订阅和分发，作为系统中各个模块间通信的枢纽
 *
 * 使用示例：
 * 
 * 1. 创建并发布事件（触发事件）：
 *      mite::EventBus::Get().Post(event);
 * 2. 订阅事件（在模块Initialize时订阅，onXxxEvent为处理该事件的逻辑）：
 *      m_XxxHandler = mite::EventBus::Get().Subscribe<XxxEvent>(BIND_DISPATCH_FN(onXxxEvent));
 * 3. 触发事件（主循环调用ProcessQueue()自动触发onEvent函数）
 *      while (window.IsRunning()) {
 *          mite::EventBus::Get().ProcessQueue();
 *      }
 * 4. 取消订阅（在模块ShutDown时取消）：
 *      mite::EventBus::Get().Unsubscribe(m_XxxHandler);
 */
class EventBus {
 public:
  using EventHandler = std::function<bool(Event &)>;  // 事件处理函数类型, bool返回值决定是否继续传播
  using HandlerID = size_t;                           // 处理器ID类型

  /**
   * @brief 单例模式：获取全局唯一实例
   * @return EventBus单例的引用
   * 
   * 使用"Meyer's singleton"方式（即函数局部静态变量）
   * 这种实现具有以下特性：
   * 线程安全：C++11标准保证静态局部变量的初始化是线程安全的
   * 按需构造：只有在第一次调用Get()时才创建实例
   * 自动销毁：程序结束时自动调用析构函数
   */
  static EventBus &Get()
  {
    static EventBus instance;
    return instance;
  }

  // 删除拷贝构造函数和赋值运算符
  EventBus(const EventBus &) = delete;
  EventBus &operator=(const EventBus &) = delete;

  /**
   * @brief 订阅指定类型的事件
   * @tparam T 事件类型
   * @param handler 事件处理函数
   * @return HandlerID 用于取消订阅的ID
   */
  template<typename T> HandlerID Subscribe(EventFn<T> handler)
  {
    // 将处理函数转换为通用事件处理函数
    auto genericHandler = [handler](Event &event) -> bool {
      // 使用Dispatcher确保类型安全
      EventDispatcher dispatcher(event);
      return dispatcher.Dispatch<T>(handler);
    };

    // 获取事件类型的哈希值作为键
    HandlerID id = m_NextHandlerID++;
    EventType type = T::GetStaticType();

    // 将处理函数存储到对应事件类型的列表中
    m_Subscribers[type].emplace_back(id, genericHandler);
    m_HandlerIDs[id] = type;

    return id;
  }

  /**
   * @brief 订阅指定类别的事件
   * @param category 事件类别(EventCategory枚举值)
   * @param handler 事件处理函数
   * @return HandlerID 用于取消订阅的ID
   */
  HandlerID SubscribeByCategory(EventCategory category, EventHandler handler)
  {
    HandlerID id = m_NextHandlerID++;
    m_CategorySubscribers[category].emplace_back(id, handler);
    m_HandlerIDs[id] = category;  // 这里使用category作为类型标记
    return id;
  }

  /**
   * @brief 取消订阅
   * @param id 订阅时返回的HandlerID
   */
  void Unsubscribe(HandlerID id)
  {
    auto it = m_HandlerIDs.find(id);
    if (it == m_HandlerIDs.end())
      return;

    // 判断是基于类型还是基于类别的订阅
    // 
    // std::holds_alternative 是 C++17 引入的一个模板函数，
    // 用于检查给定的 std::variant 对象是否包含指定类型的值。
    if (std::holds_alternative<EventType>(it->second)) {
      EventType type = std::get<EventType>(it->second);
      auto &handlers = m_Subscribers[type];
      handlers.erase(std::remove_if(handlers.begin(),
                                    handlers.end(),
                                    [id](const auto &pair) { return pair.first == id; }),
                     handlers.end());
    }
    else {
      EventCategory category = std::get<EventCategory>(it->second);
      auto &handlers = m_CategorySubscribers[category];
      handlers.erase(std::remove_if(handlers.begin(),
                                    handlers.end(),
                                    [id](const auto &pair) { return pair.first == id; }),
                     handlers.end());
    }

    m_HandlerIDs.erase(it);
  }

  /**
   * @brief 发布事件(立即处理)
   * @param event 事件对象
   * @param immediate 是否立即处理(默认为true)
   */
  void Post(Event &event, bool immediate = true)
  {
    if (immediate) {
      ProcessEvent(event);
    }
    else {
      // 异步处理预留(当前未实现)
      m_EventQueue.push_back(std::unique_ptr<Event>(event.Clone()));
    }
  }

  /**
   * @brief 处理队列中的事件
   */
  void ProcessQueue()
  {
    for (auto &event : m_EventQueue) {
      ProcessEvent(*event);
    }
    m_EventQueue.clear();
  }

  /**
   * @brief 清空所有订阅
   */
  void Clear()
  {
    m_Subscribers.clear();
    m_CategorySubscribers.clear();
    m_HandlerIDs.clear();
    m_EventQueue.clear();
    m_NextHandlerID = 0;
  }

 private:
  /**
   * @brief 处理单个事件
   * @param event 事件对象
   */
  void ProcessEvent(Event &event)
  {
    // 1. 首先处理特定类型订阅者
    auto type = event.GetEventType();
    if (m_Subscribers.find(type) != m_Subscribers.end()) {
      for (auto &[id, handler] : m_Subscribers[type]) {
        if (event.handled)
          break;  // 如果事件已被标记为处理，则停止传播
        handler(event);
      }
    }

    // 2. 然后处理类别订阅者
    auto categories = event.GetCategoryFlags();
    for (auto &[category, handlers] : m_CategorySubscribers) {
      if ((categories & category) && !event.handled) {
        for (auto &[id, handler] : handlers) {
          if (event.handled)
            break;
          handler(event);
        }
      }
    }
  }

 private:
  // 单例模式：构造函数私有化
  EventBus() = default;

  // 基于事件类型的订阅者列表
  std::unordered_map<EventType, std::vector<std::pair<HandlerID, EventHandler>>> m_Subscribers;

  // 基于事件类别的订阅者列表
  std::unordered_map<EventCategory, std::vector<std::pair<HandlerID, EventHandler>>>
      m_CategorySubscribers;

  // 处理器ID到订阅类型的映射(用于取消订阅)
  std::unordered_map<HandlerID, std::variant<EventType, EventCategory>> m_HandlerIDs;

  // 事件队列(用于延迟处理)
  std::vector<std::unique_ptr<Event>> m_EventQueue;

  // 下一个可用的处理器ID
  HandlerID m_NextHandlerID = 1;
};
};

#endif
