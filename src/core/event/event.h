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

// ͨ�ó�Ա�����󶨣�֧�� void* �ص���
#define BIND_EVENT_FN(fn) [this](void* payload) -> void { \
    using EventT = std::remove_reference_t<decltype(*static_cast<Event*>(payload))>; \
    if (auto* event = dynamic_cast<EventT*>(static_cast<Event*>(payload))) { \
        this->fn(*event); \
    } \
}

}  // namespace mite
#endif
