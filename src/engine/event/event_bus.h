#ifndef MITE_CORE_EVENT_BUS
#define MITE_CORE_EVENT_BUS

#include "dispatcher.h"
#include "subscription_flags.h"
#include "thread/parallel_utils.h" // 线程池
#include "thread/thread_pool_manager.h"

namespace mite {
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
  
  using HandlerID = size_t;                           // 处理器ID类型

  // 订阅者信息结构
  struct Subscription {
    HandlerID id = 0;
    EventHandler handler;
    SubscriptionFlags flags = SubscriptionFlags::Sync;
    std::string group = "";
    int priority = static_cast<int>(EventPriority::Normal);  // 优先级

    bool operator<(const Subscription &other) const
    {
      return priority > other.priority;  // 优先级高（数字大）的在前
    }
  };

  // 延迟事件包装器
  struct DeferredEventWrapper {
    std::unique_ptr<Event> event;
    std::function<void(Event &)> processor;
    bool isAsync = false;
  };

  /**
   * @brief 单例模式：获取全局唯一实例
   * @return EventBus单例的引用
   */
  static EventBus &Get()
  {
    // 使用"Meyer's singleton"方式（即函数局部静态变量）
    //
    // 这种实现具有以下特性：
    // 线程安全：C++ 11标准保证静态局部变量的初始化是线程安全的
    // 按需构造：只有在第一次调用Get()时才创建实例
    // 自动销毁：程序结束时自动调用析构函数
    static EventBus instance;
    return instance;
  }

  // 辅助的发布函数
  template<typename T> static void Publish(T &event)
  {
    Get().Post<T>(event);
  }

  /**
   * @brief 发布事件
   * @tparam T 事件类型
   * @param event 事件对象
   */
  template<typename T> void Post(T &event)
  {
    static_assert(std::is_base_of<Event, T>::value, "Must inherit from Event");

    // 复制订阅者列表（带锁）
    auto [typeSubscribers, categorySubscribers] = CopySubscribersForEvent<T>(event);

    // 根据每个订阅者的flags分别处理（无锁）
    ProcessEventWithSubscribers(event, std::move(typeSubscribers), std::move(categorySubscribers));
  }

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
                      EventPriority priority = EventPriority::Normal,
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
    sub.priority = static_cast<int>(priority);
    sub.flags = flags;
    sub.group = group;

    // 获取事件类型typeIndex作为键
    std::type_index typeIndex = typeid(T);

    // 按照事件类型存储订阅者（用于处理事件时按类查询）
    m_Subscribers[typeIndex].emplace_back(sub);

    // 按优先级排序（应当在执行之前进行排序，而非每次Subcribe，这里仅记录）
    m_NeedsSorting[typeIndex] = true;

    // 按照累增ID维护Handler信息（用于取消订阅）
    m_HandlerInfo[id] = HandlerInfo(typeIndex);
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
                                EventPriority priority = EventPriority::Normal,
                                SubscriptionFlags flags = SubscriptionFlags::Sync,
                                const std::string &group = "");

  /**
   * @brief 取消订阅
   * @param id 订阅时返回的HandlerID
   */
  void Unsubscribe(HandlerID id)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (auto it = m_HandlerInfo.find(id); it != m_HandlerInfo.end()) {
      auto &info = it->second;

      if (info.category != EventCategory::None) {
        // 首先尝试从类型订阅中移除
        auto &handlers = m_CategorySubscribers[info.category];
        handlers.erase(std::remove_if(handlers.begin(),
                                      handlers.end(),
                                      [id](const auto &sub) { return sub.id == id; }),
                       handlers.end());
        m_CategoryNeedsSorting[info.category] = true;
      }
      else {
        // 然后尝试从类别订阅中移除
        auto &handlers = m_Subscribers[info.typeIndex];
        handlers.erase(std::remove_if(handlers.begin(),
                                      handlers.end(),
                                      [id](const auto &sub) { return sub.id == id; }),
                       handlers.end());
        m_NeedsSorting[info.typeIndex] = true;
      }

      m_HandlerInfo.erase(it);
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

 private:
  // 处理器信息
  struct HandlerInfo {
    std::type_index typeIndex;
    EventCategory category = EventCategory::None;
    // 提供构造函数
    HandlerInfo(std::type_index index, EventCategory cat = EventCategory::None)
        : typeIndex(index), category(cat)
    {
    }
    // 默认构造函数（需要有效的type_index）
    HandlerInfo() : typeIndex(typeid(void)), category(EventCategory::None) {}
  };

  /**
   * @brief 核心处理逻辑：根据订阅者的flags分别处理（无锁）
   */
  void ProcessEventWithSubscribers(Event &event,
                                   std::vector<Subscription> typeSubscribers,
                                   std::vector<Subscription> categorySubscribers)
  {
    // 分离同步、异步、延迟处理
    std::vector<Subscription> syncSubscribers;
    std::vector<Subscription> asyncSubscribers;
    std::vector<Subscription> deferredSyncSubscribers;   // 延迟但同步执行
    std::vector<Subscription> deferredAsyncSubscribers;  // 延迟且异步执行
    // 定义分类函数
    auto classifySubscriber = [&](Subscription &sub) {
      if (SubscriptionFlagUtil::IsDeferredAsync(sub.flags))
      {  // 优先处理组合指令，避免被误认为单纯的IsDeferred或者IsAsync
        deferredAsyncSubscribers.push_back(sub);
      }
      else if (SubscriptionFlagUtil::IsDeferred(sub.flags)) {
        deferredSyncSubscribers.push_back(sub);
      }
      else if (SubscriptionFlagUtil::IsAsync(sub.flags)) {
        asyncSubscribers.push_back(sub);
      }
      else {
        syncSubscribers.push_back(sub);
      }
    };
    // 分类处理类型订阅者
    for (auto &sub : typeSubscribers) {
      classifySubscriber(sub);
    }
    // 分类处理类别订阅者
    if (!categorySubscribers.empty()) {
      std::sort(categorySubscribers.begin(), categorySubscribers.end());
      for (auto &sub : categorySubscribers) {
        classifySubscriber(sub);
      }
    }
    // 立即同步处理
    ProcessSyncSubscribers(event, syncSubscribers);

    // 立即异步处理
    if (!asyncSubscribers.empty()) {
      ProcessAsyncSubscribers(event, asyncSubscribers);  // 非延迟模式
    }

    // 延迟同步处理（加入延迟队列，但ProcessQueue时同步执行）
    if (!deferredSyncSubscribers.empty()) {
      ProcessDeferredSubscribers(event, deferredSyncSubscribers, false);
    }

    // 延迟异步处理（加入延迟队列，ProcessQueue时异步执行）
    if (!deferredAsyncSubscribers.empty()) {
      ProcessDeferredSubscribers(event, deferredAsyncSubscribers, true);
    }
  }
  /**
   * @brief 处理同步订阅者
   */
  void ProcessSyncSubscribers(Event &event, std::vector<Subscription> &subscribers)
  {
    for (auto &sub : subscribers) {
      if (!event.ShouldContinue()) {
        break;
      }
      try {
        // 主线程直接执行
        sub.handler(event);
      }
      catch (const std::exception &e) {
        LOG_ERROR("Event handler error: {}", e.what());
      }
      catch (...) {
        LOG_ERROR("Unknown error in event handler");
      }
    }
  } /**
     * @brief 处理异步订阅者
     */
  void ProcessAsyncSubscribers(Event &event, std::vector<Subscription> subscribers)
  {
    // 创建副本
    auto eventCopy = std::unique_ptr<Event>(event.Clone());

    // 使用[[maybe_unused]]来忽略返回值（小型项目暂时无需考虑Future管理的问题。待后续有需求时管理该返回值）
    [[maybe_unused]] auto future = ThreadPoolManager::GetDefaultPool().submit_task(
        [eventPtr = eventCopy.release(), subscribers = std::move(subscribers)]() {
          std::unique_ptr<Event> uniqueEvent(
              eventPtr);  // 转移所有权到原始指针，重新包装为unique_ptr(BSThread不支持Unique传参)

          for (auto &sub : subscribers) {
            if (!uniqueEvent->ShouldContinue()) {
              break;
            }
            try {
              // 在子线程内处理事件（完全无锁）
              sub.handler(*uniqueEvent);
            }
            catch (const std::exception &e) {
              LOG_ERROR("Async event handler error: {}", e.what());
            }
            catch (...) {
              LOG_ERROR("Unknown error in async event handler");
            }
          }
        });
  }
  /**
   * @brief 处理延迟订阅者
   */
  void ProcessDeferredSubscribers(Event &event,
                                  std::vector<Subscription> subscribers,
                                  bool isAsync = false)
  {
    DeferredEventWrapper wrapper;
    wrapper.event = std::unique_ptr<Event>(event.Clone());
    wrapper.isAsync = isAsync;

    // 根据同步/异步创建包装器
    if (isAsync) {
      // 延迟异步：ProcessQueue时在子线程执行
      wrapper.processor = [subscribers = std::move(subscribers)](Event &storedEvent) {
        // 在子线程中批量执行
        for (auto &sub : subscribers) {
          if (!storedEvent.ShouldContinue()) {
            break;
          }
          try {
            sub.handler(storedEvent);
          }
          catch (const std::exception &e) {
            LOG_ERROR("Deferred async event handler error: {}", e.what());
          }
          catch (...) {
            LOG_ERROR("Unknown error in deferred async event handler");
          }
        }
      };
    }
    else {
      // 延迟同步：ProcessQueue时在主线程执行
      wrapper.processor = [subscribers = std::move(subscribers)](Event &storedEvent) {
        for (auto &sub : subscribers) {
          if (!storedEvent.ShouldContinue()) {
            break;
          }
          try {
            sub.handler(storedEvent);
          }
          catch (const std::exception &e) {
            LOG_ERROR("Deferred sync event handler error: {}", e.what());
          }
          catch (...) {
            LOG_ERROR("Unknown error in deferred sync event handler");
          }
        }
      };
    }

    std::lock_guard<std::mutex> lock(m_DeferredMutex);
    m_DeferredQueue.push_back(std::move(wrapper));
  }
  /**
   * @brief 处理延迟事件队列
   */
  void ProcessDeferredEvents()
  {
    std::vector<DeferredEventWrapper> deferredEvents;
    {
      std::lock_guard<std::mutex> lock(m_DeferredMutex);
      deferredEvents = std::move(m_DeferredQueue);
      m_DeferredQueue.clear();
    }

    for (auto &wrapper : deferredEvents) {
      try {
        if (wrapper.isAsync) {
          // 异步执行：提交到线程池
          [[maybe_unused]] auto future = ThreadPoolManager::GetDefaultPool().submit_task(
              [processor = std::move(wrapper.processor),
               eventPtr = wrapper.event.release()]() mutable {
                std::unique_ptr<Event> uniqueEvent(
                    eventPtr);  // 转移所有权到原始指针，重新包装为unique_ptr(BSThread不支持Unique传参)
                processor(*uniqueEvent);
              });
        }
        else {
          // 同步执行：在当前线程（主线程）执行
          wrapper.processor(*wrapper.event);
        }
      }
      catch (...) {
        // 记录错误日志
      }
    }
  }

  /**
   * @brief 复制事件相关的订阅者列表（带锁）
   * @tparam T 事件类型
   * @param event 事件对象
   * @return 包含类型和类别订阅者的元组
   */
  template<typename T>
  std::tuple<std::vector<Subscription>, std::vector<Subscription>> CopySubscribersForEvent(
      const Event &event)
  {
    std::vector<Subscription> typeSubscribers;
    std::vector<Subscription> categorySubscribers;

    // 复制类型订阅者
    std::type_index typeIndex = typeid(T);
    if (auto it = m_Subscribers.find(typeIndex); it != m_Subscribers.end()) {
      typeSubscribers = it->second;
      EnsureSubscribersSorted(typeIndex, typeSubscribers); // 排序需要上锁
    }

    // 复制类别订阅者
    auto categories = event.GetCategoryFlags();
    for (int i = 0; i < 32; ++i) {
      EventCategory category = static_cast<EventCategory>(1 << i);
      if ((categories & category) && m_CategorySubscribers.count(category)) {
        auto &subs = m_CategorySubscribers[category];
        // 复制并确保排序
        std::vector<Subscription> categoryCopy = subs;
        EnsureCategorySubscribersSorted(category, categoryCopy);
        categorySubscribers.insert(
            categorySubscribers.end(), categoryCopy.begin(), categoryCopy.end());
      }
    }

    return {std::move(typeSubscribers), std::move(categorySubscribers)};
  }

  // 确保订阅者列表和大类订阅列表已排序
  void EnsureSubscribersSorted(std::type_index typeIndex, std::vector<Subscription> &subscribers);
  void EnsureCategorySubscribersSorted(EventCategory category,
                                       std::vector<Subscription> &subscribers);

 private:
  // 单例模式：构造函数私有化
  EventBus() = default;
  ~EventBus();

  // 删除拷贝构造函数和赋值运算符
  EventBus(const EventBus &) = delete;
  EventBus &operator=(const EventBus &) = delete;

  // 基于类型索引的订阅者列表
  std::unordered_map<std::type_index, std::vector<Subscription>> m_Subscribers;

  // 基于事件类别的订阅者列表
  std::unordered_map<EventCategory, std::vector<Subscription>> m_CategorySubscribers;

  // 处理器ID到具体处理器信息的映射
  std::unordered_map<HandlerID, HandlerInfo> m_HandlerInfo;

  // 事件队列
  std::vector<DeferredEventWrapper> m_DeferredQueue;

  // 排序标记
  std::unordered_map<std::type_index, bool> m_NeedsSorting;
  std::unordered_map<EventCategory, bool> m_CategoryNeedsSorting;

  // 线程相关
  std::mutex m_Mutex;
  std::mutex m_DeferredMutex;

  // 下一个可用的处理器ID
  HandlerID m_NextHandlerID = 1;
};
};  // namespace mite

#endif
