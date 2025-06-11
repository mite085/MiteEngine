#include "hierarchy_component.h"

namespace mite {
size_t HierarchyComponent::GetDepth(const entt::registry &registry) const
{
  // ����Ѿ��Ǹ��ڵ㣬���Ϊ0
  if (IsRoot()) {
    return 0;
  }

  // ��黺����Ч��
  if (m_DepthCache > 0) {
    return m_DepthCache;
  }

  // �ݹ�������
  size_t depth = 0;
  entt::entity current = m_Parent;

  while (current != entt::null) {
    if (!registry.valid(current)) {
      break;  // ������Чʵ����ֹ
    }

    const auto *parentHierarchy = registry.try_get<HierarchyComponent>(current);
    if (!parentHierarchy) {
      break;  // ��ʵ��û�в�����
    }

    ++depth;
    current = parentHierarchy->m_Parent;
  }

  // ���»��棨ע�⣺����ֻ�ڼ����ڼ���Ч�����־û���
  m_DepthCache = depth;

  return depth;
}

void HierarchyComponent::AddChild(entt::entity child)
{
  // Error check: Cannot add null entity as child!
  assert(child != entt::null);

  // ����Ƿ��Ѿ����ӽڵ�
  if (std::find(m_Children.begin(), m_Children.end(), child) != m_Children.end()) {
    return;
  }

  m_Children.push_back(child);
  m_DepthCache = 0;  // ʹ��Ȼ���ʧЧ
}

bool HierarchyComponent::RemoveChild(entt::entity child)
{
  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it != m_Children.end()) {
    m_Children.erase(it);
    m_DepthCache = 0;  // ʹ��Ȼ���ʧЧ
    return true;
  }
  return false;
}

void HierarchyComponent::ClearChildren()
{
  m_Children.clear();
  m_DepthCache = 0;  // ʹ��Ȼ���ʧЧ
}

void HierarchyComponent::SetParent(entt::entity parent)
{
  m_Parent = parent;
  m_DepthCache = 0;  // ʹ��Ȼ���ʧЧ
}

};
