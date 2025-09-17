#ifndef MITE_CORE_EVENT_BUS
#define MITE_CORE_EVENT_BUS

#include "dispatcher.h"
#include "subscription_flags.h"

namespace mite {
// 线程池配置标志
constexpr int THREAD_POOL_FLAGS = BS::tp::priority |             // 启用任务优先级
                                  BS::tp::wait_deadlock_checks;  // 启用死锁检查

/**
 * @brief 事件总线系统 - 核心类
 *
 * 负责管理事件的订阅和分发，作为系统中各个模块间通信的枢纽
 * 支持同步处理、异步处理、延迟处理和线程安全处理四种模式
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

  // 订阅者信息结构
  struct Subscription {
    HandlerID id;
    EventHandler handler;
    SubscriptionFlags flags;
    std::string group;

    int priority;  // 优先级
    bool operator<(const Subscription &other) const
    {
      return priority > other.priority;  // 优先级高（数字大）的在前
    }
  };

  // 异步事件包装器
  struct AsyncEventWrapper {
    std::unique_ptr<Event> event;
    std::function<void(Event &)> processor;
    SubscriptionFlags flags;
  };

  /**
   * @brief 单例模式：获取全局唯一实例
   * @return EventBus单例的引用
   */
  static EventBus &Get();

  // 辅助的发布函数
  template<typename T> static void Publish(T &event)
  {
    Get().Post<T>(event);
  }

  // 删除拷贝构造函数和赋值运算符
  EventBus(const EventBus &) = delete;
  EventBus &operator=(const EventBus &) = delete;

  /**
   * @brief 初始化线程池
   * @param threadCount 线程数量，0表示自动检测
   */
  void InitializeThreadPool(size_t threadCount = 0);

  /**
   * @brief 关闭线程池
   */
  void ShutdownThreadPool();

  /**
   * @brief 订阅指定类型的事件
   * @tparam T 事件类型
   * @param handler 事件处理函数
   * @param priority 处理优先级
   * @param flags 处理标志
   * @param group 订阅组标识
   * @return HandlerID 用于取消订阅的ID
   */
  template<typename T>
  HandlerID Subscribe(EventFn<T> handler,
                      int priority = static_cast<int>(EventPriority::Normal),
                      SubscriptionFlags flags = SubscriptionFlags::Sync,
                      const std::string &group = "")
  {
    static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");

    // 将处理函数转换为通用事件处理函数
    auto genericHandler = [handler](Event &event) {
      // 使用Dispatcher确保类型安全
      EventDispatcher dispatcher(event);
      dispatcher.Dispatch<T>(handler);
    };
    std::lock_guard<std::mutex> lock(m_Mutex);
    HandlerID id = m_NextHandlerID++;

    // 创建订阅者信息
    Subscription sub;
    sub.id = id;
    sub.handler = genericHandler;
    sub.priority = priority;
    sub.flags = flags;
    sub.group = group;

    // 获取事件类型typeIndex作为键
    std::type_index typeIndex = typeid(T);

    // 按照事件类型存储订阅者（用于处理事件时按类查询）
    m_Subscribers[typeIndex].emplace_back(sub);

    // 按优先级排序（应当在执行之前进行排序，而非每次Subcribe，这里仅记录）
    m_NeedsSorting[typeIndex] = true;

    // 按照累增ID维护Handler信息（用于取消订阅）
    m_HandlerInfo[id] = HandlerInfo(typeIndex, flags);
    return id;
  }

  /**
   * @brief 订阅指定类别的事件
   *
   * 大类订阅时，无法使用Dispatcher确保类型安全
   * 所以需要自定义Dispatcher，进行进一步分发
   *
   * 使用示例：
   *   EventBus::Get().SubscribeByCategory(EventCategory::EVENT_CATEGORY_INPUT,
   *                                       [this](Event &e) { ProcessEvent(e); });
   *   void ProcessEvent(Event &e)
   *   {
   *     EventDispatcher dispatcher(e);
   *     dispatcher.Dispatch<MouseMoveEvent>(BIND_DISPATCH_FN(handleMouseMove));
   *     dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_DISPATCH_FN(handleMouseButtonPressed));
   *     dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_DISPATCH_FN(handleMouseButtonReleased));
   *   }
   */
  HandlerID SubscribeByCategory(EventCategory category,
                                EventHandler handler,
                                int priority = static_cast<int>(EventPriority::Normal),
                                SubscriptionFlags flags = SubscriptionFlags::Sync,
                                const std::string &group = "");

  /**
   * @brief 取消订阅
   * @param id 订阅时返回的HandlerID
   */
  void Unsubscribe(HandlerID id);

  /**
   * @brief 发布事件
   * @tparam T 事件类型
   * @param event 事件对象
   * @param flags 处理标志（覆盖订阅者的默认标志）
   */
  template<typename T> void Post(T &event, SubscriptionFlags flags = SubscriptionFlags::Sync)
  {
    static_assert(std::is_base_of<Event, T>::value, "Must inherit from Event");
    if (SubscriptionFlagUtil::IsAsync(flags)) {
      PostAsync<T>(event, flags);
    }
    else if (SubscriptionFlagUtil::IsDeferred(flags)) {
      PostDeferred<T>(event);
    }
    else {
      ProcessEvent<T>(event);
    }
  }
  /**
   * @brief 处理队列中的事件(每帧执行，处理延迟事件)
   */
  void ProcessQueue();

  /**
   * @brief 清空所有订阅
   */
  void Clear();

  /**
   * @brief 获取线程池实例
   */
  BS::thread_pool<THREAD_POOL_FLAGS> &GetThreadPool();

 private:
  // 处理器信息
  struct HandlerInfo {
    std::type_index typeIndex;
    SubscriptionFlags flags;
    EventCategory category = EventCategory::None;
    // 提供构造函数
    HandlerInfo(std::type_index index,
                SubscriptionFlags f,
                EventCategory cat = EventCategory::None)
        : typeIndex(index), flags(f), category(cat)
    {
    }
    // 默认构造函数（需要有效的type_index）
    HandlerInfo()
        : typeIndex(typeid(void)), flags(SubscriptionFlags::Sync), category(EventCategory::None)
    {
    }

  };

  // 单例模式：构造函数私有化
  EventBus() = default;
  ~EventBus();

  // 事件处理实现
  template<typename T> void ProcessEvent(Event &event);

  // 确保订阅者列表和大类订阅列表已排序
  void EnsureSubscribersSorted(std::type_index typeIndex, std::vector<Subscription> &subscribers);
  void EnsureCategorySubscribersSorted(EventCategory category,
                                       std::vector<Subscription> &subscribers);

  // 异步发布
  template<typename T> void PostAsync(T &event, SubscriptionFlags flags);

  // 延迟发布
  template<typename T> void PostDeferred(T &event);

  // 处理延迟事件
  void ProcessDeferredEvents();

 private:
  // 基于类型索引的订阅者列表
  std::unordered_map<std::type_index, std::vector<Subscription>> m_Subscribers;

  // 基于事件类别的订阅者列表
  std::unordered_map<EventCategory, std::vector<Subscription>> m_CategorySubscribers;

  // 处理器ID到具体处理器信息的映射
  std::unordered_map<HandlerID, HandlerInfo> m_HandlerInfo;

  // 事件队列
  std::vector<AsyncEventWrapper> m_DeferredQueue;
  std::queue<AsyncEventWrapper> m_AsyncQueue;

  // 排序标记
  std::unordered_map<std::type_index, bool> m_NeedsSorting;
  std::unordered_map<EventCategory, bool> m_CategoryNeedsSorting;

  // 线程相关
  std::mutex m_Mutex;
  std::mutex m_DeferredMutex;

  std::unique_ptr<BS::thread_pool<THREAD_POOL_FLAGS>>
      m_ThreadPool;  // 启用任务优先级, 启用死锁检查

  // 下一个可用的处理器ID
  HandlerID m_NextHandlerID = 1;
};
};  // namespace mite

#endif
