#include "bounding_volume_component.h"
#include "scene_core/component_id.h"
#include "scene_core/scene_registry.h"

namespace mite {
// ==================== BoundingVolumeComponent ====================

BoundingVolumeComponent::BoundingVolumeComponent()
{
  // 默认创建AABB包围盒
  m_Volume = std::make_shared<BoundingVolume>(BoundingVolumeType::None);
}

void BoundingVolumeComponent::SetVolume(const BoundingVolume &volume)
{
  // 直接使用默认的拷贝构造函数
  *m_Volume = volume;
  EventBus::Publish<BoundingVolumeChangedEvent>(BoundingVolumeChangedEvent(GetEntity(), *this));
}

std::vector<std::type_index> BoundingVolumeComponent::GetDependencies() const
{
  return {};  // 不依赖其他组件
}

bool BoundingVolumeComponent::Serialize(std::ostream &output) const
{
  Component::Serialize(output);

  // 序列化包围体类型

  return !output.fail();
}

bool BoundingVolumeComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);

  // 反序列化包围体类型

  return !input.fail();
}
const BoundingVolume &BoundingVolumeComponent::GetSnapshotData() const
{
  return *m_Volume;
}

void BoundingVolumeComponent::SetSnapshotData(const BoundingVolume &data)
{
  *m_Volume = data;
  // 发布更新事件
  EventBus::Publish<BoundingVolumeChangedEvent>(BoundingVolumeChangedEvent(GetEntity(), *this));
}
// ==================== BoundingVolumeComponentSystem ====================
}  // namespace mite