#include "event/event.h"
#include <typeinfo>

namespace mite {
std::string Event::ToString() const
{
  return typeid(*this).name();
}
bool Event::IsInCategory(EventCategory category)
{
  return GetCategoryFlags() & category;
}
}
