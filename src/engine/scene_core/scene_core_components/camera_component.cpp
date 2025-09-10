#include "camera_component.h"
#include "transform_component.h"

namespace mite {
CameraComponent::CameraComponent(std::shared_ptr<Camera> camera) : m_Camera(camera) {}

void CameraComponent::ProcessDirty(float deltaTime, SceneRegistry &reg)
{
  // 处理CameraInputProcessor和TransformComponent的数据同步
  if (reg.HasComponent<TransformComponent>(GetEntity())) {
    // 对比上一帧，检测Transform是否主动发生变化
    auto &transform = reg.GetComponent<TransformComponent>(GetEntity());
    TransformState currentState = {transform.GetWorldPosition(reg),
                                   transform.GetWorldRotation(reg)};

    // 若Transform主动改动（并非通过CameraInputProcessor手段改动的，而是通过TransfromComponent，如Gizmo、界面滑动条等）
    if (IsTransformChanged(currentState, m_lastTransformState)) {
      // 执行 Transform → Camera 同步（均为累加式同步，防止直接赋值导致歧义）
      m_Camera->Move(transform.GetWorldPosition(reg) - currentState.position);
      m_Camera->Rotate(transform.GetWorldRotation(reg) - currentState.rotation);
    }

    // 执行 Camera → Transform 同步
    // （均为赋值式同步，直接使用Camera数值修改Transfrom可以更直观的显示结果）
    glm::vec3 newPosition = m_Camera->GetPosition();
    glm::vec3 newRotation = m_Camera->GetRotationEuler();
    transform.SetWorldPosition(reg, m_lastTransformState.position);
    transform.SetWorldRotation(reg, m_lastTransformState.rotation);

    // 并以同步结果记录State
    // （此时即便Transform没有主动改动，transform也会不同，
    // 这个“不同”不作为主动改动依据，否则会出现多次累加的bug）
    m_lastTransformState.position = newPosition;
    m_lastTransformState.rotation = newRotation;
  }

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

// void CameraComponent::Rotate(float yaw, float pitch)
//{
//   m_Camera->Rotate(yaw, pitch);
//   MarkDirty();
// }
//
// void CameraComponent::Pan(float right, float up)
//{
//   m_Camera->Pan(right, up);
//   MarkDirty();
// }

void CameraComponent::Zoom(float amount)
{
  m_Camera->Zoom(amount);
  MarkDirty();
}

// void CameraComponent::Move(const glm::vec3 &direction)
//{
//   m_Camera->Move(direction);
//   MarkDirty();
// }

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
    EventBus::Publish<CameraVisibilityMaskChangedEvent>(
        CameraVisibilityMaskChangedEvent(m_Entity, *this, m_VisibilityMask));
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
    EventBus::Publish<CameraVisibilityMaskChangedEvent>(
        CameraVisibilityMaskChangedEvent(m_Entity, *this, m_VisibilityMask));
  }
}
void CameraComponent::RemoveVisibilityLayer(uint32_t mask)
{
  uint32_t old_mask = m_VisibilityMask;
  m_VisibilityMask &= ~mask;
  if (m_VisibilityMask != mask) {
    MarkDirty();
    EventBus::Publish<CameraVisibilityMaskChangedEvent>(
        CameraVisibilityMaskChangedEvent(m_Entity, *this, m_VisibilityMask));
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

bool CameraComponent::IsTransformChanged(const TransformState &current, const TransformState &last)
{
  // 位置容差比较
  bool positionChanged = glm::length(current.position - last.position) > POSITION_EPSILON;

  // 欧拉角容差比较（分别比较三个轴）
  glm::vec3 rotationDelta = glm::abs(current.rotation - last.rotation);
  bool rotationChanged = (rotationDelta.x > ROTATION_EPSILON) ||
                         (rotationDelta.y > ROTATION_EPSILON) ||
                         (rotationDelta.z > ROTATION_EPSILON);

  return positionChanged || rotationChanged;
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
      return component->GetEntity();
    }
  }
  return Entity{};
}

void CameraComponentSystem::SetMainCameraEntity(Entity mainCamera)
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
    else if (component->GetEntity() == mainCamera) {
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
    EventBus::Publish<MainCameraChangedEvent>(MainCameraChangedEvent(mainCamera, *newMain));
  }
  else {
    LOG_ERROR("Invalid camera entity when setting new main camera");
  }
}

void CameraComponentSystem::ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
{
  // CameraComponent负责了CameraInputProcessor和TransformComponent的数据同步，
  // 由于无法通过CameraInputProcessor标注dirty，此处应当处理所有Camera
  for (auto *comp : m_AllComponents) {
    comp->ProcessDirty(deltaTime, registry);
  }
  m_DirtyComponents.clear();
}
};  // namespace mite