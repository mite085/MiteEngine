#include "event_bus.h"

namespace mite {
EventBus::HandlerID EventBus::SubscribeByCategory(EventCategory category,
                                                  EventHandler handler,
                                                  EventPriority priority,
                                                  SubscriptionFlags flags,
                                                  const std::string &group)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  HandlerID id = m_NextHandlerID++;

  // 创建订阅者信息
  Subscription sub;
  sub.id = id;
  sub.handler = handler;
  sub.priority = static_cast<int>(priority);
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

BS::thread_pool<ThreadPoolConfig::DEFAULT_FLAGS> &EventBus::GetThreadPool()
{
  // 使用统一的线程池管理器获取默认线程池
  return ThreadPoolManager::GetDefaultPool();
}

EventBus::~EventBus()
{
  Clear();
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
}  // namespace mite