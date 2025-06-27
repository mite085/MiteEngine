#include "visibility_component.h"
#include "transform_component.h"

namespace mite {
VisibilityComponent::VisibilityComponent() : ComponentTraits() {}

// 基础可见性控制 ======================================
void VisibilityComponent::SetVisible(bool visible)
{
  if (m_IsVisible != visible) {
    m_IsVisible = visible;
    EventBus::Get().Post(VisibilityChangedEvent(GetOwnerEntity(), *this, visible));
  }
}

bool VisibilityComponent::IsVisible() const
{
  return m_IsVisible && !m_AlwaysVisible;
}

void VisibilityComponent::ToggleVisible()
{
  SetVisible(!m_IsVisible);
}

// 视锥体裁剪 ==========================================
void VisibilityComponent::SetFrustumCulling(bool cull)
{
  m_FrustumCulling = cull;
}

bool VisibilityComponent::GetFrustumCulling() const
{
  return m_FrustumCulling;
}

bool VisibilityComponent::WasInFrustum() const
{
  return m_WasInFrustum;
}

void VisibilityComponent::SetCustomBounds(const AABB &aabb)
{
  m_CustomBounds = aabb;
}

AABB VisibilityComponent::GetCustomBounds() const
{
  return m_CustomBounds;
}

// 距离剔除 ============================================
void VisibilityComponent::SetMaxVisibleDistance(float distance)
{
  m_MaxVisibleDistance = std::max(0.0f, distance);
}

float VisibilityComponent::GetMaxVisibleDistance() const
{
  return m_MaxVisibleDistance;
}

bool VisibilityComponent::WasInDistance() const
{
  return m_WasInDistance;
}

// 层级可见性 ==========================================
void VisibilityComponent::SetLayerMask(uint32_t mask)
{
  m_LayerMask = mask;
}

uint32_t VisibilityComponent::GetLayerMask() const
{
  return m_LayerMask;
}

// 调试功能 ============================================
void VisibilityComponent::SetAlwaysVisible(bool always)
{
  m_AlwaysVisible = always;
}

bool VisibilityComponent::IsAlwaysVisible() const
{
  return m_AlwaysVisible;
}

void VisibilityComponent::SetShowBounds(bool show)
{
  m_ShowBounds = show;
}

bool VisibilityComponent::GetShowBounds() const
{
  return m_ShowBounds;
}

// 组件接口实现 ========================================
std::vector<std::type_index> VisibilityComponent::GetDependencies() const
{
  return {typeid(TransformComponent)};
}

bool VisibilityComponent::Serialize(std::ostream &output) const
{
  Component::Serialize(output);  // 序列化基类数据

  // 序列化可见性设置
  output.write(reinterpret_cast<const char *>(&m_IsVisible), sizeof(m_IsVisible));
  output.write(reinterpret_cast<const char *>(&m_FrustumCulling), sizeof(m_FrustumCulling));
  output.write(reinterpret_cast<const char *>(&m_MaxVisibleDistance),
               sizeof(m_MaxVisibleDistance));
  output.write(reinterpret_cast<const char *>(&m_LayerMask), sizeof(m_LayerMask));

  // 序列化自定义包围盒
  if (!m_CustomBounds.Serialize(output)) {
    return false;
  }

  return !output.fail();
}

bool VisibilityComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);  // 反序列化基类数据

  // 反序列化可见性设置
  input.read(reinterpret_cast<char *>(&m_IsVisible), sizeof(m_IsVisible));
  input.read(reinterpret_cast<char *>(&m_FrustumCulling), sizeof(m_FrustumCulling));
  input.read(reinterpret_cast<char *>(&m_MaxVisibleDistance), sizeof(m_MaxVisibleDistance));
  input.read(reinterpret_cast<char *>(&m_LayerMask), sizeof(m_LayerMask));

  // 反序列化自定义包围盒
  if (!m_CustomBounds.Deserialize(input)) {
    return false;
  }

  return !input.fail();
}

// Visibility组件系统实现 ==================================
void VisibilitySystem::Initialize(SceneRegistry &registry)
{
  // 初始化系统资源
}

void VisibilitySystem::Shutdown(SceneRegistry &registry)
{
  // 清理系统资源
}

void VisibilitySystem::Update(float deltaTime, SceneRegistry &registry)
{
  // TODO: 调试绘制
  //if (DebugDraw::IsEnabled()) {
  //  DebugDrawBounds(registry);
  //}
}

void VisibilitySystem::PerformFrustumCulling(const Frustum &frustum, SceneRegistry &registry)
{
  auto view = registry.GetEntitiesWith<VisibilityComponent, TransformComponent>();

  for (auto entity : view) {
    auto &visibility = registry.GetComponent<VisibilityComponent>(entity);
    auto &transform = registry.GetComponent<TransformComponent>(entity);

    // 跳过不进行裁剪测试的对象
    if (!visibility.GetFrustumCulling() || visibility.IsAlwaysVisible()) {
      visibility.m_WasInFrustum = true;
      continue;
    }

    // 获取包围盒(优先使用自定义包围盒)
    AABB bounds = visibility.GetCustomBounds();
    if (bounds.IsEmpty()) {
      // TODO: 从MeshComponent获取包围盒
      bounds = AABB(glm::vec3(-0.5f), glm::vec3(0.5f));  // 默认单位包围盒
    }

    // 变换到世界空间
    bounds.Transform(transform.GetWorldMatrix(registry));

    // 执行视锥体测试
    visibility.m_WasInFrustum = frustum.Intersects(bounds);
  }
}

void VisibilitySystem::PerformDistanceCulling(const glm::vec3 &cameraPosition,
                                              SceneRegistry &registry)
{
  auto view = registry.GetEntitiesWith<VisibilityComponent, TransformComponent>();

  for (auto entity : view) {
    auto &visibility = registry.GetComponent<VisibilityComponent>(entity);
    auto &transform = registry.GetComponent<TransformComponent>(entity);

    // 跳过无限距离或总是可见的对象
    if (visibility.GetMaxVisibleDistance() <= 0.0f || visibility.IsAlwaysVisible()) {
      visibility.m_WasInDistance = true;
      continue;
    }

    // 计算距离
        float distance = glm::distance((transform.GetWorldPosition(registry)), cameraPosition);
        visibility.m_WasInDistance = (distance <= visibility.GetMaxVisibleDistance());
  }
}

void VisibilitySystem::DebugDrawBounds(SceneRegistry &registry)
{
  auto view = registry.GetEntitiesWith<VisibilityComponent, TransformComponent>();

  for (auto entity : view) {
    auto &visibility = registry.GetComponent<VisibilityComponent>(entity);
    auto &transform = registry.GetComponent<TransformComponent>(entity);

    if (!visibility.GetShowBounds())
      continue;

    // 获取包围盒(优先使用自定义包围盒)
    AABB bounds = visibility.GetCustomBounds();
    if (bounds.IsEmpty()) {
      // TODO: 从MeshComponent获取包围盒
      bounds = AABB(glm::vec3(-0.5f), glm::vec3(0.5f));  // 默认单位包围盒
    }

    // 变换到世界空间
    bounds.Transform(transform.GetWorldMatrix(registry));

    // 根据可见性状态选择颜色
    glm::vec4 color = visibility.IsVisible() ?
                          (visibility.WasInFrustum() ? glm::vec4(0, 1, 0, 0.2f) :
                                                       glm::vec4(1, 1, 0, 0.1f)) :
                          glm::vec4(1, 0, 0, 0.1f);

    // TODO: 绘制包围盒
    //DebugDraw::DrawAABB(bounds, color);
  }
}
};  // namespace mite