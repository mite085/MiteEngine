#include "hierarchy_component.h"
#include "scene_core/entity.h"

namespace mite {
size_t HierarchyComponent::GetDepth(SceneRegistry &registry)
{
  // 如果已经是根节点，深度为0
  if (IsRoot()) {
    return 0;
  }

  // 检查缓存有效性
  if (m_DepthCache > 0) {
    return m_DepthCache;
  }

  // 递归计算深度
  size_t depth = 0;
  Entity current = m_Parent;

  while (current.IsValid()) {
    if (!registry.IsValid(current)) {
      break;  // 遇到无效实体终止
    }

    auto parentHierarchy = registry.TryGetComponent<HierarchyComponent>(current);
    if (!parentHierarchy) {
      break;  // 父实体没有层次组件
    }

    ++depth;
    current = parentHierarchy->GetParent();
  }

  // 更新缓存（注意：缓存只在计算期间有效，不持久化）
  m_DepthCache = depth;

  return depth;
}

void HierarchyComponent::AddChild(Entity child)
{
  // Error check: Cannot add null entity as child!
  assert(child != entt::null);

  // 检查是否已经是子节点
  if (std::find(m_Children.begin(), m_Children.end(), child) != m_Children.end()) {
    return;
  }

  m_Children.push_back(child);
  m_DepthCache = 0;  // 使深度缓存失效
}

bool HierarchyComponent::RemoveChild(Entity child)
{
  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it != m_Children.end()) {
    m_Children.erase(it);
    m_DepthCache = 0;  // 使深度缓存失效
    return true;
  }
  return false;
}

void HierarchyComponent::ClearChildren()
{
  m_Children.clear();
  m_DepthCache = 0;  // 使深度缓存失效
}

void HierarchyComponent::SetParent(Entity parent)
{
  m_Parent = parent;
  m_DepthCache = 0;  // 使深度缓存失效
}

};
