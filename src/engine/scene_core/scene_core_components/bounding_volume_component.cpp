#include "bounding_volume_component.h"
#include "scene_core/component_id.h"
#include "scene_core/scene_registry.h"

namespace mite {
// ==================== BoundingVolumeComponent ====================

BoundingVolumeComponent::BoundingVolumeComponent()
    : m_Volume(BoundingVolumeType::None)  // 默认创建无效包围盒
{
}

void BoundingVolumeComponent::SetVolume(const BoundingVolume &volume)
{
  // 检查类型是否改变
  BoundingVolumeType oldType = m_Volume.GetType();
  BoundingVolumeType newType = volume.GetType();

  m_Volume = volume;

  // 如果类型改变，发布事件，由SceneGraph负责处理，使用修改后的包围盒维护SceneNode数据
  if (oldType != newType) {
    EventBus::Publish<BoundingVolumeChangedEvent>(BoundingVolumeChangedEvent(GetEntity(), *this));
  }
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
  return m_Volume;
}

void BoundingVolumeComponent::SetSnapshotData(const BoundingVolume &data)
{
  m_Volume = data;
  // 发布更新事件
  EventBus::Publish<BoundingVolumeChangedEvent>(BoundingVolumeChangedEvent(GetEntity(), *this));
}
// ==================== BoundingVolumeComponentSystem ====================


}  // namespace mite
