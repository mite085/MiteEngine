#include "camera_component.h"
#include "transform_component.h"

namespace mite {
CameraComponent::CameraComponent()
{
  m_Camera.SetPerspective(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);
}

void CameraComponent::SetPerspective(float fov, float near, float far)
{
  float aspect = m_Camera.GetAspectRatio();
  m_Camera.SetPerspective(fov, aspect, near, far);
  MarkDirty();
}

void CameraComponent::SetOrthographic(float size, float near, float far)
{
  float aspect = m_Camera.GetAspectRatio();
  m_Camera.SetOrthographic(size, aspect, near, far);
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
  return m_Camera.GetProjectionMatrix();
}

void CameraComponent::SetViewportSize(uint32_t width, uint32_t height)
{
  if (height == 0)
    return;
  float aspect = static_cast<float>(width) / height;
  m_Camera.SetAspectRatio(aspect);
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

void CameraSystem::ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
{
  // 处理视口变化等逻辑
  for (auto *comp : m_DirtyComponents) {
    comp->CleanDirty();
  }
  m_DirtyComponents.clear();
}
};
