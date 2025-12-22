#include "component_event_publisher.h"
namespace mite {
ComponentEventPublisher::ComponentEventPublisher() {}

ComponentEventPublisher::~ComponentEventPublisher() { UnregisterCallbacks(); }

void ComponentEventPublisher::UnregisterCallbacks() {
  std::unique_lock lock(m_Mutex);
  m_ConstructCallbacks.clear();
  m_DestroyCallbacks.clear();
}
};  // namespace mite