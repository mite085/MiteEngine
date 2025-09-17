#include "event_bus.h"

namespace mite {
EventBus &EventBus::Get()
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
void EventBus::InitializeThreadPool(size_t threadCount)
{
  if (threadCount == 0) {
    threadCount = std::thread::hardware_concurrency();
    if (threadCount == 0)
      threadCount = 1;  // 确保至少1个线程
  }

  m_ThreadPool.reset(new BS::thread_pool<THREAD_POOL_FLAGS>(threadCount));
}
void EventBus::ShutdownThreadPool()
{
  if (m_ThreadPool) {
    m_ThreadPool->wait();
    m_ThreadPool.reset();
  }
}

EventBus::HandlerID EventBus::SubscribeByCategory(EventCategory category,
                                                  EventHandler handler,
                                                  int priority,
                                                  SubscriptionFlags flags,
                                                  const std::string &group)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  HandlerID id = m_NextHandlerID++;

  // 创建订阅者信息
  Subscription sub;
  sub.id = id;
  sub.handler = handler;
  sub.priority = priority;
  sub.flags = flags;
  sub.group = group;

  // 按照大类存储订阅者（用于处理事件时按大类查询）
  m_CategorySubscribers[category].emplace_back(sub);

  // 按优先级排序（应当在执行之前进行排序，而非每次Subcribe，这里仅记录）
  m_CategoryNeedsSorting[category] = true;

  // 按照累增ID维护Handler信息（用于取消订阅）
  m_HandlerInfo[id] = HandlerInfo(typeid(void), flags, category);
  return id;
}

void EventBus::Unsubscribe(HandlerID id)
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

void EventBus::ProcessQueue()
{
  ProcessDeferredEvents();
}

void EventBus::Clear()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Subscribers.clear();
  m_CategorySubscribers.clear();
  m_HandlerInfo.clear();
  m_NeedsSorting.clear();
  m_CategoryNeedsSorting.clear();

  {
    std::lock_guard<std::mutex> deferredLock(m_DeferredMutex);
    m_DeferredQueue.clear();
  }

  m_NextHandlerID = 1;
}

BS::thread_pool<THREAD_POOL_FLAGS> &EventBus::GetThreadPool()
{
  if (!m_ThreadPool) {
    InitializeThreadPool();
  }
  return *m_ThreadPool;
}

EventBus::~EventBus()
{
  ShutdownThreadPool();
  Clear();
}

template<typename T> void EventBus::ProcessEvent(Event &event)
{
  std::type_index typeIndex = typeid(T);
  std::vector<Subscription> typeSubscribers;
  std::vector<Subscription> categorySubscribers;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    // 获取类型订阅者
    if (auto it = m_Subscribers.find(typeIndex); it != m_Subscribers.end()) {
      typeSubscribers = it->second;
      EnsureSubscribersSorted(typeIndex, typeSubscribers);
    }
    // 获取类别订阅者
    auto categories = event.GetCategoryFlags();
    for (int i = 0; i < 32; ++i) {
      EventCategory category = static_cast<EventCategory>(1 << i);
      if ((categories & category) && m_CategorySubscribers.count(category)) {
        auto &subs = m_CategorySubscribers[category];
        categorySubscribers.insert(categorySubscribers.end(), subs.begin(), subs.end());
        EnsureCategorySubscribersSorted(category, subs);
      }
    }
  }
  // 处理类型订阅者
  for (auto &sub : typeSubscribers) {
    if (!event.ShouldContinue()) {
      break;
    }
    sub.handler(event);
  }
  // 处理类别订阅者（按优先级排序）
  if (!categorySubscribers.empty()) {
    std::sort(categorySubscribers.begin(), categorySubscribers.end());

    for (auto &sub : categorySubscribers) {
      if (!event.ShouldContinue()) {
        break;
      }
      sub.handler(event);
    }
  }
}

void EventBus::EnsureSubscribersSorted(std::type_index typeIndex,
                                       std::vector<Subscription> &subscribers)
{
  if (m_NeedsSorting[typeIndex]) {
    std::sort(subscribers.begin(), subscribers.end());
    m_NeedsSorting[typeIndex] = false;
  }
}
void EventBus::EnsureCategorySubscribersSorted(EventCategory category,
                                               std::vector<Subscription> &subscribers)
{
  if (m_CategoryNeedsSorting[category]) {
    std::sort(subscribers.begin(), subscribers.end());
    m_CategoryNeedsSorting[category] = false;
  }
}

template<typename T> void EventBus::PostAsync(T &event, SubscriptionFlags flags)
{
  auto eventCopy = std::unique_ptr<Event>(event.Clone());

  // 根据优先级提交任务
  int priority = 0;  // 默认优先级

  // 使用辅助函数检查线程安全标志
  if (SubscriptionFlagUtil::IsThreadSafe(flags)) {
    priority = 10;  // 线程安全任务更高优先级
  }

  GetThreadPool().submit_task(
      [this, eventPtr = eventCopy.release()]() {
        std::unique_ptr<Event> uniqueEvent(eventPtr);
        T &specificEvent = static_cast<T &>(*uniqueEvent);

        // 在worker线程中处理事件
        ProcessEvent<T>(specificEvent);
      },
      priority  // 任务优先级
  );
}
template<typename T> void EventBus::PostDeferred(T &event)
{
  AsyncEventWrapper wrapper;
  wrapper.event = std::unique_ptr<Event>(event.Clone());
  wrapper.flags = SubscriptionFlags::Deferred;
  wrapper.processor = [](Event &storedEvent) {
    T &specificEvent = static_cast<T &>(storedEvent);
    // 延迟处理逻辑会在ProcessQueue中执行
  };
  std::lock_guard<std::mutex> lock(m_DeferredMutex);
  m_DeferredQueue.push_back(std::move(wrapper));
}
void EventBus::ProcessDeferredEvents()
{
  std::vector<AsyncEventWrapper> deferredEvents;

  {
    std::lock_guard<std::mutex> lock(m_DeferredMutex);
    deferredEvents = std::move(m_DeferredQueue);
    m_DeferredQueue.clear();
  }
  for (auto &wrapper : deferredEvents) {
    try {
      // 在主线程中处理延迟事件
      wrapper.processor(*wrapper.event);
    }
    catch (...) {
      // 记录错误日志
    }
  }
}

// 显式实例化常用模板
template void EventBus::ProcessEvent<Event>(Event &);
template void EventBus::PostAsync<Event>(Event &, SubscriptionFlags);
template void EventBus::PostDeferred<Event>(Event &);
}  // namespace mite