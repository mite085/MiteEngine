#include "light_component.h"
#include "transform_component.h"

namespace mite {

LightComponent::LightComponent() : SnapshotComponentTraits(), m_Light(nullptr) {}

LightComponent::LightComponent(std::shared_ptr<Light> light)
    : SnapshotComponentTraits(), m_Light(light)
{
}

std::vector<std::type_index> LightComponent::GetDependencies() const
{
  // 光源组件需要变换组件来获取世界坐标和方向
  return {typeid(TransformComponent)};
}

// ==================== 光源管理 ====================

void LightComponent::SetLight(std::shared_ptr<Light> light)
{
  if (m_Light != light) {
    m_Light = light;
    EventBus::Publish<LightUpdatedEvent>(LightUpdatedEvent(GetEntity(), *this));
  }
}

std::shared_ptr<Light> LightComponent::GetLight() const
{
  return m_Light;
}

bool LightComponent::HasLight() const
{
  return m_Light != nullptr;
}

// ==================== 类型相关 ====================

LightType LightComponent::GetLightType() const
{
  return m_Light ? m_Light->GetType() : LightType::POINT;
}

std::string LightComponent::GetLightTypeName() const
{
  return m_Light ? m_Light->GetLightTypeName() : "Invalid";
}

// ==================== 基础属性访问 ====================

void LightComponent::SetColor(const glm::vec3 &color)
{
  if (m_Light && m_Light->GetColor() != color) {
    m_Light->SetColor(color);
    EventBus::Publish<LightUpdatedEvent>(LightUpdatedEvent(GetEntity(), *this));
  }
}

const glm::vec3 &LightComponent::GetColor() const
{
  static const glm::vec3 defaultColor(1.0f);
  return m_Light ? m_Light->GetColor() : defaultColor;
}

void LightComponent::SetIntensity(float intensity)
{
  if (m_Light && m_Light->GetIntensity() != intensity) {
    m_Light->SetIntensity(intensity);
    EventBus::Publish<LightUpdatedEvent>(LightUpdatedEvent(GetEntity(), *this));
  }
}

float LightComponent::GetIntensity() const
{
  return m_Light ? m_Light->GetIntensity() : 0.0f;
}

void LightComponent::SetEnabled(bool enabled)
{
  if (m_Light && m_Light->IsEnabled() != enabled) {
    m_Light->SetEnabled(enabled);
    EventBus::Publish<LightUpdatedEvent>(LightUpdatedEvent(GetEntity(), *this));
  }
}

bool LightComponent::IsEnabled() const
{
  return m_Light ? m_Light->IsEnabled() : false;
}

// ==================== 完整属性访问 ====================

LightProperties &LightComponent::GetProperties()
{
  static LightProperties defaultProperties;
  return m_Light ? m_Light->GetProperties() : defaultProperties;
}

const LightProperties &LightComponent::GetProperties() const
{
  static const LightProperties defaultProperties;
  return m_Light ? m_Light->GetProperties() : defaultProperties;
}

void LightComponent::SetProperties(const LightProperties &props)
{
  if (m_Light) {
    m_Light->SetProperties(props);
    EventBus::Publish<LightUpdatedEvent>(LightUpdatedEvent(GetEntity(), *this));
  }
}


// ==================== 序列化接口 ====================

bool LightComponent::Serialize(std::ostream &output) const
{
  Component::Serialize(output);
  // 序列化留空，等待序列化模块
  return !output.fail();
}

bool LightComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);
  // 反序列化留空，等待序列化模块
  return !input.fail();
}

// ==================== 快照接口实现 ====================
const Light &LightComponent::GetSnapshotData() const
{
  return *m_Light;
}

void LightComponent::SetSnapshotData(const Light &data)
{
  *m_Light = data;
  // 发布更新事件
  EventBus::Publish<LightUpdatedEvent>(LightUpdatedEvent(GetEntity(), *this));
}

}  // namespace mite
