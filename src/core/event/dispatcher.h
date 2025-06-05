#ifndef MITE_EVENT_DISPATCHER
#define MITE_EVENT_DISPATCHER

#include <functional>
#include "event/event.h"

namespace mite {

template<typename T> using EventFn = std::function<bool(T &)>;

class EventDispatcher {
 public:
  explicit EventDispatcher(Event &event) : m_Event(event) {}

  template<typename T> bool Dispatch(EventFn<T> func);

 private:
  Event &m_Event;
};

}  // namespace mite
#endif
