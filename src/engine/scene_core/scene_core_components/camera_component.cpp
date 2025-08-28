#include "camera_component.h"
#include "transform_component.h"

namespace mite {
CameraComponent::CameraComponent(std::shared_ptr<Camera> camera) : m_Camera(camera) {}

void CameraComponent::ProcessDirty(float deltaTime, SceneRegistry& reg) 
{
  if (!IsDirty()) {
    return;
  }
  // ProcessDirty应当同步ECS的Transform到Camera
  // 但由于Camera内部使用了独立的Transform系统，
  // 每次Rotate、Pan等均为立即执行，
  // 无需等待每帧的ProcessDirty步骤。
  // 
  // TODO: 
  // Camera内部使用了独立的Transform系统并不太标准，
  // 应当使用同一套统一的Transform系统，但如果Camera维护Transform，
  // 此时CameraComponent和TransformComponent为同级关系，
  // 先更新Transform结束后，再更新Camera，是否会导致覆盖更新？
  // 
  // 初步解决方案：
  // CameraEntity维护一个普通的TransformComponent，仅仅记录欧拉角旋转，
  // CameraComponent执行的Rotate、Pan、Move等均累积记录，待ProcessDirty
  // 执行时，将这些作用于TransformComponent的矩阵上，并将矩阵用于Camera的View矩阵更新
  // 
  //if (reg.HasComponent<TransformComponent>(GetOwnerEntity())) {
  //  auto &transform = reg.GetComponent<TransformComponent>(GetOwnerEntity());

  //  // 获取世界变换矩阵
  //  glm::mat4 worldMatrix = transform.GetWorldMatrix(reg);

  //  // 从世界矩阵提取位置和旋转
  //  glm::vec3 position = glm::vec3(worldMatrix[3]);
  //  glm::mat3 rotationMat = glm::mat3(worldMatrix);

  //  // 设置Camera的位置和朝向
  //  m_Camera->SetViewMatrix(glm::inverse(worldMatrix));
  //}
  ClearDirty();
}

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