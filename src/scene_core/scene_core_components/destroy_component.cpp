#include "destroy_component.h"

namespace mite {
DestroyComponent::DestroyComponent(std::weak_ptr<Entity> owner) : ComponentTraits(owner) {}
};  // namespace mite
