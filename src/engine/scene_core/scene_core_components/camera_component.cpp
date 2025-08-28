#include "camera_component.h"
#include "transform_component.h"

namespace mite {
CameraComponent::CameraComponent(std::shared_ptr<Camera> camera) : m_Camera(camera) {}

void CameraComponent::SetPerspective(float fov, float near, float far)
{
  float aspect = m_Camera->GetAspectRatio();
  m_Camera->SetPerspective(fov, aspect, near, far);
  MarkDirty();
}

void CameraComponent::SetOrthographic(float size, float near, float far)
{
  float aspect = m_Camera->GetAspectRatio();
  m_Camera->SetOrthographic(size, aspect, near, far);
  MarkDirty();
}

void CameraComponent::Rotate(float yaw, float pitch)
{
  m_Camera->Rotate(yaw, pitch);
  MarkDirty();
}

void CameraComponent::Pan(float right, float up)
{
  m_Camera->Pan(right, up);
  MarkDirty();
}

void CameraComponent::Zoom(float amount)
{
  m_Camera->Zoom(amount);
  MarkDirty();
}

void CameraComponent::Move(const glm::vec3 &direction)
{
  m_Camera->Move(direction);
  MarkDirty();
}

CameraUsage CameraComponent::GetUsage() const
{
  return m_Usage;
}

void CameraComponent::SetUsage(CameraUsage usage)
{
  m_Usage = usage;
  MarkDirty();
}

std::shared_ptr<Camera> CameraComponent::GetCamera()
{
  return m_Camera;
}

glm::mat4 CameraComponent::GetViewMatrix() const
{
  return m_Camera->GetViewMatrix();
}

glm::mat4 CameraComponent::GetProjectionMatrix() const
{
  return m_Camera->GetProjectionMatrix();
}

void CameraComponent::SetViewportSize(uint32_t width, uint32_t height)
{
  if (height == 0)
    return;
  float aspect = static_cast<float>(width) / height;
  m_Camera->SetAspectRatio(aspect);
  MarkDirty();
}

void CameraComponent::SetVisibilityMask(uint32_t mask)
{
  if (m_VisibilityMask != mask) {
    m_VisibilityMask = mask;
    MarkDirty();
    EventBus::Get().Post(CameraVisibilityMaskChangedEvent(m_OwnerEntity, *this, m_VisibilityMask));
  }
}

uint32_t CameraComponent::GetVisibilityMask() const
{
  return m_VisibilityMask;
}

void CameraComponent::AddVisibilityLayer(uint32_t mask)
{
  uint32_t old_mask = m_VisibilityMask;
  m_VisibilityMask |= mask;
  if (m_VisibilityMask != mask) {
    MarkDirty();
    EventBus::Get().Post(CameraVisibilityMaskChangedEvent(m_OwnerEntity, *this, m_VisibilityMask));
  }
}
void CameraComponent::RemoveVisibilityLayer(uint32_t mask)
{
  uint32_t old_mask = m_VisibilityMask;
  m_VisibilityMask &= ~mask;
  if (m_VisibilityMask != mask) {
    MarkDirty();
    EventBus::Get().Post(CameraVisibilityMaskChangedEvent(m_OwnerEntity, *this, m_VisibilityMask));
  }
}

bool CameraComponent::HasVisibilityLayer(uint32_t mask) const
{
  return (m_VisibilityMask & mask) != 0;
}

bool CameraComponent::Serialize(std::ostream &output) const
{
  // TODO: 序列化投影类型、参数等

  return !output.fail();
}

bool CameraComponent::Deserialize(std::istream &input)
{
  // TODO: 反序列化并重建Camera状态

  MarkDirty();
  return !input.fail();
}

std::vector<std::type_index> CameraComponent::GetDependencies() const
{
  return {};
}

// ==================== CameraComponentSystem ====================

std::vector<std::type_index> CameraComponentSystem::GetSystemDependencies() const
{
  return {typeid(TransformComponentSystem)};  // 需要变换信息
}

Entity CameraComponentSystem::GetMainCameraEntity() const
{
  for (auto component : m_AllComponents) {
    if (!component) {
      LOG_ERROR("Empty camera component pointer in camera component system!");
    }
    else if (component->GetUsage() == CameraUsage::MainView) {
      return component->GetOwnerEntity();
    }
  }
  return Entity{};
}

void CameraComponentSystem::SetMainCameraEntity(Entity main_camera)
{
  // 遍历组件列表，获取新camera组件和旧camera组件
  CameraComponent *oldMain = nullptr, *newMain = nullptr;
  for (auto component : m_AllComponents) {
    if (!component) {
      LOG_ERROR("Empty camera component pointer in camera component system!");
    }
    else if (component->GetUsage() == CameraUsage::MainView) {
      oldMain = component;
    }
    else if (component->GetOwnerEntity() == main_camera) {
      newMain = component;
    }
  }

  // 如果新旧相同，则省略修改
  if (oldMain == newMain)
    return;

  // 清除之前的主相机标记
  if (oldMain) {
    oldMain->SetUsage(CameraUsage::FreeView);
  }
  // 设置新的主相机
  if (newMain) {
    newMain->SetUsage(CameraUsage::MainView);
    EventBus::Get().Post(MainCameraChangedEvent(main_camera, *newMain));
  }
  else {
    LOG_ERROR("Invalid camera entity when setting new main camera");
  }
}

void CameraComponentSystem::ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
{
  // 处理视口变化等逻辑
  for (auto *comp : m_DirtyComponents) {
    comp->ClearDirty();
  }
  m_DirtyComponents.clear();
}
};  // namespace mite