#ifndef MITE_CORE_EVENT_BUS
#define MITE_CORE_EVENT_BUS

#include "event/dispatcher.h"
#include "event/event.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <variant>
#include <typeindex>

namespace mite {
/**
 * @brief 事件总线系统 - 核心类
 *
 * 负责管理事件的订阅和分发，作为系统中各个模块间通信的枢纽
 *
 * 使用示例：（注意：第二步和第四步可以由SubscriptionGroup代为实现）
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
  using EventHandler = std::function<void(Event &)>;  // 事件处理函数类型
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
  // 辅助的发布函数
  template<typename T>
  static void Publish(T& event) {
    Get().Post<T>(event);
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
    static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");

    // 将处理函数转换为通用事件处理函数
    auto genericHandler = [handler](Event &event) -> bool {
      // 使用Dispatcher确保类型安全
      EventDispatcher dispatcher(event);
      return dispatcher.Dispatch<T>(handler);
    };

    // 获取事件类型typeIndex作为键
    HandlerID id = m_NextHandlerID++;
    std::type_index typeIndex = typeid(T);

    // 存储处理函数
    m_Subscribers[typeIndex].emplace_back(id, genericHandler);
    m_HandlerTypes[id] = &typeid(T);
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
    m_HandlerCategories[id] = category;
    return id;
  }

  /**
   * @brief 取消订阅
   * @param id 订阅时返回的HandlerID
   */
  void Unsubscribe(HandlerID id)
  {
    // 首先尝试从类型订阅中移除
    if (auto it = m_HandlerTypes.find(id); it != m_HandlerTypes.end()) {
      std::type_index typeIndex = *it->second;
      auto &handlers = m_Subscribers[typeIndex];
      handlers.erase(std::remove_if(handlers.begin(),
                                    handlers.end(),
                                    [id](const auto &pair) { return pair.first == id; }),
                     handlers.end());
      m_HandlerTypes.erase(it);
      return;
    }  
    // 然后尝试从类别订阅中移除
    if (auto it = m_HandlerCategories.find(id); it != m_HandlerCategories.end()) {
      EventCategory category = it->second;
      auto &handlers = m_CategorySubscribers[category];
      handlers.erase(std::remove_if(handlers.begin(),
                                    handlers.end(),
                                    [id](const auto &pair) { return pair.first == id; }),
                     handlers.end());
      m_HandlerCategories.erase(it);
    }
  }

  /**
   * @brief 发布事件(立即处理)
   * @param event 事件对象
   * @param immediate 是否立即处理(默认为true，否则推入队列异步处理)
   */
  template<typename T>
  void Post(T &e, bool immediate = true)
  {
    static_assert(std::is_base_of<Event, T>::value, "Must inherit from Event");

    //LOG_DEBUG("Posting event: {}", e.ToString());

    if (immediate) {
      // 立即执行
      ProcessEvent<T>(e);
    }
    else {
      // 创建包装器，存储事件拷贝，保存类型信息和处理逻辑（方便异步处理时获取正确类型）
      EventWrapper wrapper;
      wrapper.event = std::unique_ptr<Event>(e.Clone());

      // 设置类型特定的处理器
      wrapper.processor = [](EventBus &bus, Event &storedEvent) {
        // 直接static_cast，如果类型不匹配会在编译时或运行时报错
        T &specificEvent = static_cast<T &>(storedEvent);
        bus.ProcessEvent<T>(specificEvent);
      };
      // 等待异步处理(当前未验证多线程安全性)
      m_EventQueue.push_back(std::move(wrapper));
    }
  }

  /**
   * @brief 处理队列中的事件
   * 
   * 异步处理(当前未验证多线程安全性，未启用)
   */
  void ProcessQueue()
  {
    for (auto &wrapper : m_EventQueue) {
      wrapper.processor(*this, *wrapper.event);
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
    m_HandlerTypes.clear();
    m_HandlerCategories.clear();
    m_EventQueue.clear();
    m_NextHandlerID = 0;
  }

 private:
  /**
   * @brief 处理单个事件
   * @param event 事件对象
   */
  template<typename T>
  void ProcessEvent(Event &event)
  {
    std::type_index typeIndex = typeid(T);
    // 1. 首先处理特定类型订阅者
    if (m_Subscribers.find(typeIndex) != m_Subscribers.end()) {
      for (auto &[id, handler] : m_Subscribers[typeIndex]) {
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

    //LOG_DEBUG("Processing event: {}", event.ToString());
  }

 private:
  // 单例模式：构造函数私有化
  EventBus() = default;

  // 基于类型索引的订阅者列表
  std::unordered_map<std::type_index, std::vector<std::pair<HandlerID, EventHandler>>>
      m_Subscribers;

  // 基于事件类别的订阅者列表
  std::unordered_map<EventCategory, std::vector<std::pair<HandlerID, EventHandler>>>
      m_CategorySubscribers;

  // 处理器ID到类型索引的映射，注意使用 std::type_info* 而不是 std::type_index
  std::unordered_map<HandlerID, const std::type_info *> m_HandlerTypes;

  // 处理器ID到事件类别的映射
  std::unordered_map<HandlerID, EventCategory> m_HandlerCategories;


  // 使用std::any存储事件对象，保持类型信息
  struct EventWrapper {
    std::unique_ptr<Event> event;  // 存储事件对象的拷贝
    std::function<void(EventBus &, Event &)> processor;
  };
  // 事件队列
  std::vector<EventWrapper> m_EventQueue;

  // 下一个可用的处理器ID
  HandlerID m_NextHandlerID = 1;
};
};

#endif
