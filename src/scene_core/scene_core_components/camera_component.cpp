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

glm::mat4 CameraComponent::GetViewMatrix(SceneRegistry &reg) const
{
  auto &transform = reg.GetComponent<TransformComponent>(GetOwnerEntity());
  glm::vec3 position = transform.GetWorldPosition(reg);
  glm::quat rotation = transform.GetWorldRotation(reg);

  glm::mat4 view = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation);
  return glm::inverse(view);
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
  return {typeid(TransformComponent)};
}

std::optional<Entity> CameraComponentSystem::GetMainCameraEntity() const
{
  for (auto component : m_AllComponents) {
    if (!component) {
      LOG_ERROR("Empty camera component pointer in camera component system!");
    }
    else if (component->GetUsage() == CameraUsage::MainView) {
      return component->GetOwnerEntity();
    }
  }
  return std::nullopt;
}

void CameraComponentSystem::SetMainCameraEntity(Entity main_camera)
{
  // 遍历组件列表，获取新camera组件和旧camera组件
  CameraComponent *oldMain = nullptr, *newMain = nullptr;
  for (auto component : m_AllComponents) {
    if (!component) {
      LOG_ERROR("Empty camera component pointer in camera component system!");
    }
    else if (component->GetUsage() == CameraUsage::MainView)
    {
      oldMain = component;
    }
    else if (component->GetOwnerEntity() == main_camera) {
      newMain = component;
    }
  }

  // 清除之前的主相机标记
  if (oldMain) {
    oldMain->SetUsage(CameraUsage::FreeView);
  }
  // 设置新的主相机
  if (newMain) {
    newMain->SetUsage(CameraUsage::MainView);
  }
  else {
    LOG_ERROR("Invalid camera entity when setting new main camera");
  }
}

void CameraComponentSystem::ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
{
  // 处理视口变化等逻辑
  for (auto *comp : m_DirtyComponents) {
    comp->CleanDirty();
  }
  m_DirtyComponents.clear();
}
};  // namespace mite