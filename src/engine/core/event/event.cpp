#include "event/event.h"
namespace mite {
std::string Event::ToString() const
{
  return GetName();
}
bool Event::IsInCategory(EventCategory category)
{
  return GetCategoryFlags() & category;
}
}
