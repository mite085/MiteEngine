#include "scene_node.h"
#include "scene_core/scene_registry.h"
#include "scene_core_components/hierarchy_component.h"
#include "scene_core_components/transform_component.h"
#include "visibility_component.h"

namespace mite {

SceneNode::SceneNode(Entity entity) : m_Entity(entity)
{
  m_LocalTransform = glm::mat4(1.0f);
  m_WorldTransform = glm::mat4(1.0f);
  m_TransformDirty = true;
  m_BoundsDirty = true;
}

SceneNode::~SceneNode()
{
  // 从父节点中移除自己
  if (m_Parent) {
    m_Parent->RemoveChild(this);
  }

  // 清空子节点（子节点会自动设置父节点为nullptr）
  for (auto child : m_Children) {
    child->m_Parent = nullptr;
  }
  m_Children.clear();
}

void SceneNode::SetParent(SceneNode *parent)
{
  if (m_Parent == parent)
    return;

  // 从原父节点移除
  if (m_Parent) {
    m_Parent->RemoveChild(this);
  }

  m_Parent = parent;

  // 添加到新父节点
  if (m_Parent) {
    m_Parent->AddChild(this);
  }

  // 标记需要更新变换
  MarkTransformDirty();
  MarkBoundsDirty();
}

void SceneNode::AddChild(SceneNode *child)
{
  if (child == nullptr || child == this)
    return;

  // 检查是否已经是子节点
  if (std::find(m_Children.begin(), m_Children.end(), child) != m_Children.end()) {
    return;
  }

  m_Children.push_back(child);
  child->m_Parent = this;

  // 标记子节点需要更新
  child->MarkTransformDirty();
  child->MarkBoundsDirty();
}

bool SceneNode::RemoveChild(SceneNode *child)
{
  if (child == nullptr)
    return false;

  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it != m_Children.end()) {
    m_Children.erase(it);
    child->m_Parent = nullptr;

    // 标记子节点需要更新
    child->MarkTransformDirty();
    child->MarkBoundsDirty();
    return true;
  }

  return false;
}

void SceneNode::SetLocalTransform(const glm::mat4 &localTransform)
{
  if (m_LocalTransform != localTransform) {
    m_LocalTransform = localTransform;
    MarkTransformDirty();
    MarkBoundsDirty();
  }
}

void SceneNode::SetLocalBounds(const AABB &localBounds)
{
  if (m_LocalBounds.min != localBounds.min || m_LocalBounds.max != localBounds.max) {
    m_LocalBounds = localBounds;
    MarkBoundsDirty();
  }
}

void SceneNode::MarkTransformDirty()
{
  if (!m_TransformDirty) {
    m_TransformDirty = true;
    MarkChildrenTransformDirty();
  }
}

void SceneNode::MarkBoundsDirty()
{
  if (!m_BoundsDirty) {
    m_BoundsDirty = true;
    MarkChildrenBoundsDirty();
  }
}

void SceneNode::MarkChildrenTransformDirty()
{
  for (auto child : m_Children) {
    child->MarkTransformDirty();
  }
}

void SceneNode::MarkChildrenBoundsDirty()
{
  for (auto child : m_Children) {
    child->MarkBoundsDirty();
  }
}

void SceneNode::Update(bool force)
{
  if (m_TransformDirty || force) {
    UpdateWorldTransform();
    m_TransformDirty = false;
  }

  if (m_BoundsDirty || force) {
    UpdateWorldBounds();
    m_BoundsDirty = false;
  }

  // 递归更新子节点
  for (auto child : m_Children) {
    child->Update(force);
  }
}

void SceneNode::UpdateWorldTransform()
{
  if (m_Parent && !m_Parent->IsRoot()) {
    // 有父节点：世界变换 = 父世界变换 × 局部变换
    m_WorldTransform = m_Parent->GetWorldTransform() * m_LocalTransform;
  }
  else {
    // 根节点：世界变换 = 局部变换
    m_WorldTransform = m_LocalTransform;
  }
}

void SceneNode::UpdateWorldBounds()
{
  if (m_LocalBounds.IsValid()) {
    // 变换局部包围盒到世界空间
    m_WorldBounds = m_LocalBounds.Transform(m_WorldTransform);
  }
  else {
    // 无效的局部包围盒，使用默认值
    m_WorldBounds = AABB(glm::vec3(0.0f), glm::vec3(0.0f));
  }
}

int SceneNode::GetDepth() const
{
  int depth = 0;
  SceneNode *current = m_Parent;
  while (current != nullptr) {
    depth++;
    current = current->m_Parent;
  }
  return depth;
}

std::string SceneNode::GetPath() const
{
  std::vector<std::string> pathParts;
  SceneNode *current = const_cast<SceneNode *>(this);

  while (current != nullptr) {
    pathParts.push_back("Entity_" + current->GetEntity().GetUUIDString());
    current = current->m_Parent;
  }

  std::reverse(pathParts.begin(), pathParts.end());

  std::stringstream ss;
  for (size_t i = 0; i < pathParts.size(); ++i) {
    if (i > 0)
      ss << "/";
    ss << pathParts[i];
  }

  return ss.str();
}

bool SceneNode::IsNodeVisible(SceneRegistry &registry,
                                     uint32_t visibilityMask) const
{
  if (!m_Entity.IsValid()) {
    return false;
  }

  if (registry.HasComponent<VisibilityComponent>(m_Entity)) {
    auto &visibilityComp = registry.GetComponent<VisibilityComponent>(m_Entity);
    return visibilityComp.IsVisible() && visibilityComp.MatchesMask(visibilityMask);
  }

  // 没有VisibilityComponent的节点默认可见
  return true;
}

}  // namespace mite
