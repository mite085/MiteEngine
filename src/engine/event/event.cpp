#include "event.h"

namespace mite {
std::string Event::ToString() const {
  std::stringstream ss;
  ss << "Event[" << typeid(*this).name() << "]";

  // 添加结果状态信息
  switch (m_Result) {
    case EventResult::None:
      ss << " - None";
      break;
    case EventResult::Handled:
      ss << " - Handled";
      break;
    case EventResult::Consumed:
      ss << " - Consumed";
      break;
    case EventResult::Failed:
      ss << " - Failed";
      break;
    case EventResult::Blocked:
      ss << " - Blocked";
      break;
    case EventResult::Deferred:
      ss << " - Deferred";
      break;
    case EventResult::HandledAndStop:
      ss << " - HandledAndStop";
      break;
    case EventResult::FailedAndStop:
      ss << " - FailedAndStop";
      break;
    default:
      ss << " - UnknownResult(" << static_cast<int>(m_Result) << ")";
      break;
  }

  return ss.str();
}
bool Event::IsInCategory(EventCategory category) {
  return (GetCategoryFlags() & static_cast<int>(category)) != 0;
}
}  // namespace mite