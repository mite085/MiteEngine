#ifndef MITE_EVENT
#define MITE_EVENT

#include <string>
#include "event/event_types.h"

namespace mite {

#define EVENT_CLASS_TYPE(type) \
  static EventType GetStaticType() \
  { \
    return EventType::type; \
  } \
  virtual EventType GetEventType() const override \
  { \
    return GetStaticType(); \
  } \
  virtual const char *GetName() const override \
  { \
    return #type; \
  }

#define EVENT_CLASS_CATEGORY(category) \
  virtual int GetCategoryFlags() const override \
  { \
    return category; \
  }

class Event {
 public:
  virtual ~Event() = default;

  // ʹ�ú�ֱ��ʵ��
  virtual EventType GetEventType() const = 0;
  virtual const char *GetName() const = 0;
  virtual int GetCategoryFlags() const = 0;

  virtual std::string ToString() const;
  bool IsInCategory(EventCategory category);

  bool handled = false;  // ����¼��Ƿ��ѱ�����
};

// ����Ա������Ϊ�¼��ص�
template<typename T, typename EventT>
auto BindEventFn(T *instance, bool (T::*fn)(EventT &))
{
  return [instance, method](Event &e) -> bool {
    if (e.GetEventType() == EventT::GetStaticType()) {
      return (instance->*method)(static_cast<EventT &>(e));
    }
    return false;
  };
}

}  // namespace mite
#endif
