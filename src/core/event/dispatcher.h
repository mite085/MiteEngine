#ifndef MITE_EVENT_DISPATCHER
#define MITE_EVENT_DISPATCHER

#include <functional>
#include "event/event.h"

namespace mite {

template<typename T> using EventFn = std::function<bool(T &)>;

class EventDispatcher {
 public:
  explicit EventDispatcher(Event &event) : m_Event(event) {}

  template<typename T>
  bool Dispatch(std::function<bool(T&)> func) {
	  if (m_Event.GetEventType() == T::GetStaticType()) {
		  m_Event.handled = func(static_cast<T&>(m_Event));
		  return true;
	  }
	  return false;
  }

 private:
  Event &m_Event;
};

// 辅助宏：用于 EventDispatcher 分发（保持原有类型安全）
#define BIND_DISPATCH_FN(fn) [this](auto&& event) -> bool { \
    return this->fn(event); \
}

}  // namespace mite
#endif
