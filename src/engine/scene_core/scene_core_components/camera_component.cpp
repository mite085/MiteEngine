#include "camera_component.h"
#include "transform_component.h"

namespace mite {
CameraComponent::CameraComponent(CameraProjectionType type)
    : m_Camera(std::make_shared<Camera>())
{
  m_Camera->SetProjectionType(type);
}

void CameraComponent::SetPerspective(float fov, float near, float far)
{
  float aspect = m_Camera->GetAspectRatio();
  m_Camera->SetPerspective(fov, near, far);
}
void CameraComponent::SetOrthographic(float size, float near, float far)
{
  float aspect = m_Camera->GetAspectRatio();
  m_Camera->SetOrthographic(size, near, far);
}

void CameraComponent::SetAspectRatio(float aspect)
{
  m_Camera->SetAspectRatio(aspect);
}
void CameraComponent::SetProjectionType(CameraProjectionType type)
{
  m_Camera->SetProjectionType(type);
}
void CameraComponent::Zoom(float amount)
{
  m_Camera->Zoom(amount);
}
//CameraUsage CameraComponent::GetUsage() const
//{
//  return m_Usage;
//}
//void CameraComponent::SetUsage(CameraUsage usage)
//{
//  m_Usage = usage;
//}

glm::mat4 CameraComponent::GetProjectionMatrix() const
{
  // Get时处理Transform内部的Dirty，所以无需在组件ProcessDirty
  return m_Camera->GetProjectionMatrix();
}


std::shared_ptr<Camera> &CameraComponent::GetCamera()
{
  return m_Camera;
}

void CameraComponent::SetViewportSize(uint32_t width, uint32_t height)
{
  if (height == 0)
    return;
  float aspect = static_cast<float>(width) / height;
  m_Camera->SetAspectRatio(aspect);
}

bool CameraComponent::Serialize(std::ostream &output) const
{
  // TODO: 序列化投影类型、参数等

  return !output.fail();
}

bool CameraComponent::Deserialize(std::istream &input)
{
  // TODO: 反序列化并重建Camera状态
  return !input.fail();
}

std::vector<std::type_index> CameraComponent::GetDependencies() const
{
  return {typeid(TransformComponent)};
}

std::shared_ptr<Camera> CameraComponent::GetSnapshotData() const
{
  return m_Camera;
}

void CameraComponent::SetSnapshotData(const std::shared_ptr<Camera> &data)
{
  m_Camera = data;
  // 发布更新事件
  EventBus::Publish<CameraChangedEvent>(CameraChangedEvent(GetEntity(), *this));
}

// ==================== CameraComponentSystem ====================

std::vector<std::type_index> CameraComponentSystem::GetSystemDependencies() const
{
  return {typeid(TransformComponentSystem)};  // 需要变换信息
}

//Entity CameraComponentSystem::GetMainCameraEntity() const
//{
//  for (auto [entity, component] : m_AllComponents) {
//    if (component && component->GetUsage() == CameraUsage::MainView) {
//      return component->GetEntity();
//    }
//  }
//
//  LOG_ERROR("No main camera component pointer in camera component system!");
//  return Entity{};
//}
//
//void CameraComponentSystem::SetMainCameraEntity(Entity mainCamera)
//{
//  // 遍历组件列表，获取新camera组件和旧camera组件
//  CameraComponent *oldMain = nullptr, *newMain = nullptr;
//
//  for (auto [entity, component] : m_AllComponents) {
//    if (!component) {
//      LOG_ERROR("Empty camera component pointer in camera component system!");
//      continue;
//    }
//    else if (component->GetUsage() == CameraUsage::MainView) {
//      oldMain = component;
//    }
//    else if (component->GetEntity() == mainCamera) {
//      newMain = component;
//    }
//  }
//
//  // 如果新旧相同（旧的存在，都是nullptr的相同不算相同），则省略修改
//  if (oldMain && oldMain == newMain)
//    return;
//
//  // 清除之前的主相机标记
//  if (oldMain) {
//    oldMain->SetUsage(CameraUsage::FreeView);
//  }
//  // 设置新的主相机
//  if (newMain) {
//    newMain->SetUsage(CameraUsage::MainView);
//    EventBus::Publish<MainCameraChangedEvent>(MainCameraChangedEvent(mainCamera, *newMain));
//  }
//  else {
//    LOG_ERROR("Invalid camera entity when setting new main camera");
//  }
//}
};  // namespace mite