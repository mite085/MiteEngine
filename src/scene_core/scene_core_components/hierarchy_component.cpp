#include "hierarchy_component.h"
#include "scene_core/entity.h"

namespace mite {
size_t HierarchyComponent::GetDepth(const entt::registry &registry)
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
  // TODO: 此处不应直接接触entt::registry的逻辑，应当对其进行进一步封装。
  size_t depth = 0;
  entt::entity current = m_Parent->GetHandle();

  while (current != entt::null) {
    if (!registry.valid(current)) {
      break;  // 遇到无效实体终止
    }

    const auto *parentHierarchy = registry.try_get<HierarchyComponent>(current);
    if (!parentHierarchy) {
      break;  // 父实体没有层次组件
    }

    ++depth;
    current = parentHierarchy->m_Parent->GetHandle();
  }

  // 更新缓存（注意：缓存只在计算期间有效，不持久化）
  m_DepthCache = depth;

  return depth;
}

void HierarchyComponent::AddChild(std::shared_ptr<Entity> child)
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

bool HierarchyComponent::RemoveChild(std::shared_ptr<Entity> child)
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

void HierarchyComponent::SetParent(std::shared_ptr<Entity> parent)
{
  m_Parent = parent;
  m_DepthCache = 0;  // 使深度缓存失效
}

};
