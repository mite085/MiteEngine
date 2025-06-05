#include "event/dispatcher.h"

namespace mite {
template<typename T> bool EventDispatcher::Dispatch(EventFn<T> func)
{
  if (m_Event.GetEventType() == T::GetStaticType()) {
    m_Event.handled = func(*(static_cast<T *>(&m_Event)));
    return true;
  }
  return false;
}
}  // namespace mite