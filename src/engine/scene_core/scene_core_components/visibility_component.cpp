#include "visibility_component.h"
#include "basic_data/bounding_volumes.h"
#include "scene_core/component_id.h"
#include "scene_core/scene_registry.h"
#include "scene_core_components/transform_component.h"

namespace mite {
// ==================== VisibilityComponent ====================

VisibilityComponent::VisibilityComponent() : ComponentTraits(){}


void VisibilityComponent::ProcessDirty(float deltaTime, SceneRegistry &reg)
{
  if (!IsDirty()) {
    return;
  }
  // 保存上一帧状态
  m_WasVisible = m_IsVisible;
  // 如果没有手动覆盖，执行自动可见性计算
  // 注意：现在可见性计算应该由专门的剔除系统处理
  // 这里只处理基本的掩码匹配逻辑
  if (!m_ManualOverride) {
    // 默认可见，实际可见性应由专门的剔除系统设置
    m_IsVisible = true;
  }
  // 如果可见性发生变化，发布事件
  if (VisibilityChanged()) {
    EventBus::Publish<VisibilityChangedEvent>(
        VisibilityChangedEvent(GetEntity(), *this, m_IsVisible));
  }

  ClearDirty();
}

void VisibilityComponent::SetVisible(bool visible)
{
  if (m_IsVisible != visible) {
    m_IsVisible = visible;
    m_ManualOverride = true;  // 设置为手动覆盖模式
    MarkDirty();
  }
}

void VisibilityComponent::SetVisibilityMask(uint32_t mask)
{
  if (m_VisibilityMask != mask) {
    uint32_t oldMask = m_VisibilityMask;
    m_VisibilityMask = mask;

    // 发布掩码改变事件
    EventBus::Publish<VisibilityMaskChangedEvent>(
        VisibilityMaskChangedEvent(GetEntity(), *this, oldMask, mask));
    MarkDirty();
  }
}

std::vector<std::type_index> VisibilityComponent::GetDependencies() const
{
  return {};
}

bool VisibilityComponent::Serialize(std::ostream &output) const
{
  Component::Serialize(output);
  // 序列化基础数据
  // TODO: 实现具体的序列化逻辑
  return !output.fail();
}

bool VisibilityComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);
  // 反序列化基础数据
  // TODO: 实现具体的反序列化逻辑
  return !input.fail();
}

}  // namespace mite