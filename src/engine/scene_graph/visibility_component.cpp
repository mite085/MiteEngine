#include "visibility_component.h"
#include "bounding_volumes.h"
#include "scene_core/component_id.h"
#include "scene_core/scene_registry.h"
#include "scene_core_components/transform_component.h"
#include "scene_node.h"
#include "transform_scene_node_system.h"

namespace mite {
// ==================== VisibilityComponent ====================

VisibilityComponent::VisibilityComponent() : ComponentTraits(), m_LocalAABB(), m_WorldAABB() {}

VisibilityComponent::VisibilityComponent(const AABB &localAABB)
    : ComponentTraits(), m_LocalAABB(localAABB), m_WorldAABB()
{
}

void VisibilityComponent::ProcessDirty(float deltaTime, SceneRegistry &reg)
{
  if (!IsDirty() && !m_BoundsDirty) {
    return;
  }

  // 更新世界空间包围盒
  if (m_BoundsDirty) {
    UpdateWorldAABB(reg);
    m_BoundsDirty = false;
  }

  // 保存上一帧状态
  m_WasVisible = m_IsVisible;

  // 如果没有手动覆盖，执行自动可见性计算
  if (!m_ManualOverride) {
    // 这里可以添加更复杂的可见性计算逻辑
    // 目前简单设置为总是可见，后续可以扩展为基于相机位置的测试
    m_IsVisible = true;
  }

  // 如果可见性发生变化，发布事件
  if (VisibilityChanged()) {
    EventBus::Get().Post(VisibilityChangedEvent(GetEntity(), *this, m_IsVisible));
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

void VisibilityComponent::SetLocalAABB(const AABB &aabb)
{
  if (m_LocalAABB.min != aabb.min || m_LocalAABB.max != aabb.max) {
    m_LocalAABB = aabb;
    MarkBoundsDirty();
    MarkDirty();
  }
}

void VisibilityComponent::SetVisibilityMask(uint32_t mask)
{
  if (m_VisibilityMask != mask) {
    m_VisibilityMask = mask;
    MarkDirty();
  }
}

Sphere VisibilityComponent::GetWorldSphere() const
{
  return Sphere::FromAABB(m_WorldAABB);
}

std::vector<std::type_index> VisibilityComponent::GetDependencies() const
{
  return {typeid(TransformComponent)};
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

void VisibilityComponent::MarkBoundsDirty()
{
  m_BoundsDirty = true;
  MarkDirty();
}

void VisibilityComponent::UpdateWorldAABB(SceneRegistry &reg)
{
  if (reg.HasComponent<TransformComponent>(GetEntity())) {
    auto &transform = reg.GetComponent<TransformComponent>(GetEntity());
    glm::mat4 worldMatrix = transform.GetWorldMatrix(reg);

    // 使用BoundingVolumes工具类变换AABB
    m_WorldAABB = BoundingVolumes::TransformAABB(m_LocalAABB, worldMatrix);
  }
  else {
    // 没有变换组件，使用局部AABB作为世界AABB
    m_WorldAABB = m_LocalAABB;
  }
}

IntersectionType VisibilityComponent::TestFrustum(const Frustum &frustum) const
{
  return frustum.TestAABB(m_WorldAABB);
}

// ==================== VisibilityComponentSystem ====================

void VisibilityComponentSystem::Initialize()
{
  // 订阅原先组件通用事件
  DirtyComponentSystem<VisibilityComponent>::Initialize();

  //// 订阅主相机修改事件和相机可见性掩码修改事件
  //m_EventSubscriptions.Subscribe<MainCameraChangedEvent>(BIND_DISPATCH_FN(OnMainCameraChanged));
  //m_EventSubscriptions.Subscribe<CameraVisibilityMaskChangedEvent>(
  //    BIND_DISPATCH_FN(OnCameraVisibilityMaskChanged));
}

std::vector<std::type_index> VisibilityComponentSystem::GetSystemDependencies() const
{
  return {typeid(TransformComponentSystem), typeid(TransformSceneNodeSystem)};  // 需要世界变换信息
}

//void VisibilityComponentSystem::SetMainCameraFrustum(const Frustum &frustum)
//{
//  mainCameraFrustum = frustum;
//}
//
//void VisibilityComponentSystem::SetCameraVisibilityMask(uint32_t mask)
//{
//  cameraVisibilityMask = mask;
//}

void VisibilityComponentSystem::ProcessDirtyComponents(float deltaTime, SceneRegistry &registry)
{
  // 收集所有需要处理的可见性组件
  std::vector<VisibilityComponent *> componentsToProcess;

  for (auto *comp : m_DirtyComponents) {
    if (comp->IsDirty() || comp->IsBoundsDirty()) {
      componentsToProcess.push_back(comp);
    }
  }

  //visibleCount = 0;

  // 并行处理可见性计算
  std::for_each(std::execution::par,
                componentsToProcess.begin(),
                componentsToProcess.end(),
                [&](VisibilityComponent *comp) {
                  // 处理脏标记
                  comp->ProcessDirty(deltaTime, registry);

                  // 统计可见实体
                  //if (comp->IsVisible() && comp->MatchesMask(cameraVisibilityMask)) {
                  //  visibleCount++;
                  //}
                });
}

//bool VisibilityComponentSystem::OnMainCameraChanged(MainCameraChangedEvent &e)
//{
//  // 获取修改后的主相机
//  auto &mainCamera = e.GetComponent();
//
//  // 根据主相机，修改相机可见性掩码
//  cameraVisibilityMask = mainCamera.GetVisibilityMask();
//
//  e.Handled();
//  return true;
//}
//
//bool VisibilityComponentSystem::OnCameraVisibilityMaskChanged(CameraVisibilityMaskChangedEvent &e)
//{
//  // 判断修改后的相机是否为主相机
//  auto &mainCamera = e.GetComponent();
//
//  if (mainCamera.GetUsage() == CameraUsage::MainView) {
//    // 根据主相机，修改相机可见性掩码
//    cameraVisibilityMask = mainCamera.GetVisibilityMask();
//  }
//
//  e.Handled();
//  return true;
//}

template<> ComponentID ComponentID::Get<VisibilityComponent>()
{
  // 使用类型信息生成确定性UUID
  const std::type_index typeIdx(typeid(VisibilityComponent));
  const size_t hash = typeIdx.hash_code();

  static const mite::ComponentID id(UUIDGenerator::Generate(hash));
  return id;
}
}  // namespace mite