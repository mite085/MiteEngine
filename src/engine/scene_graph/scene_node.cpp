#include "scene_node.h"
#include "scene_core/scene_registry.h"
#include "scene_core_components/bounding_volume_component.h"
#include "scene_core_components/transform_component.h"
#include "scene_core_components/visibility_component.h"

namespace mite {
SceneNode::SceneNode(Entity entity)
    : m_Entity(entity), m_WorldBounds(BoundingVolume(BoundingVolumeType::None))
{
  // 初始化世界包围盒为无效状态，实际应当通过包围盒组件获取
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
// ==================== 实体和关系操作 ====================
Entity SceneNode::GetEntity() const
{
  return m_Entity;
}
void SceneNode::SetParent(SceneNode *parent)
{
  if (m_Parent == parent)
    return;

  // 递归检查parent是否为当前node的child，避免循环依赖
  if (IsChild(parent)) {
    return;
  }

  // 记录原父节点的变换，从原父节点移除
  Transform oldParentWorldTransform;
  if (m_Parent) {
    oldParentWorldTransform = m_Parent->GetWorldTransform();
    m_Parent->RemoveChild(this);
  }

  // 记录新父节点的变换，添加到新父节点
  m_Parent = parent;
  Transform newParentWorldTransform;
  if (m_Parent) {
    newParentWorldTransform = m_Parent->GetWorldTransform();
    m_Parent->AddChild(this);
  }

  // world = oldParentWorld * local = newParentWorld * newLocal
  // newLocal = inv(newParentWorld) * oldParentWorld * local
  Transform biasTransform = Transform(glm::inverse(newParentWorldTransform.GetLocalMatrix()) *
                                      oldParentWorldTransform.GetLocalMatrix());
  // 标记需要更新变换
  MarkTransformDirty(biasTransform);
  MarkBoundsDirty();
  MarkVisibilityDirty();
}
SceneNode *SceneNode::GetParent() const
{
  return m_Parent;
}
void SceneNode::AddChild(SceneNode *child)
{
  if (child == nullptr || child == this)
    return;

  // 检查是否已经是子节点
  if (std::find(m_Children.begin(), m_Children.end(), child) != m_Children.end()) {
    return;
  }

  // 添加子节点
  m_Children.push_back(child);
  child->m_Parent = this;

  // 标记子节点需要更新
  child->MarkTransformDirty();
  child->MarkBoundsDirty();
  child->MarkVisibilityDirty();
}
bool SceneNode::RemoveChild(SceneNode *child)
{
  if (child == nullptr)
    return false;

  // 检查是否存在该子节点
  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it != m_Children.end()) {
    // 执行移除操作
    m_Children.erase(it);
    child->m_Parent = nullptr;

    // 标记子节点需要更新
    child->MarkTransformDirty();
    child->MarkBoundsDirty();
    child->MarkVisibilityDirty();
    return true;
  }

  return false;
}
bool SceneNode::IsChild(SceneNode *child)
{
  // 遍历children
  for (SceneNode *node : GetChildren()) {
    // 仅当node有效时
    if (node) {
      if (child == node)
        return true;
      else if (node->IsChild(child))
        return true;
    }
  }
  // 不存在指定child节点，可以作为新的child，不会循环依赖
  return false;
}
const std::vector<SceneNode *> &SceneNode::GetChildren() const
{
  return m_Children;
}

// ==================== 状态查询 ====================
bool SceneNode::IsRoot() const
{
  return m_Parent == nullptr;
}
bool SceneNode::IsLeaf() const
{
  return m_Children.empty();
}
int SceneNode::GetDepth() const
{
  // 向上追溯的同时计数
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
  // 向上追溯的同时，累积pathParts
  std::vector<std::string> pathParts;
  SceneNode *current = const_cast<SceneNode *>(this);

  while (current != nullptr) {
    pathParts.push_back("Entity_" + current->GetEntity().GetUUIDString());
    current = current->m_Parent;
  }

  // 反转容器中元素的顺序，确保顺序是从最顶部向下的
  std::reverse(pathParts.begin(), pathParts.end());

  // 合并字符串
  std::stringstream ss;
  for (size_t i = 0; i < pathParts.size(); ++i) {
    if (i > 0)
      ss << "/";
    ss << pathParts[i];
  }

  return ss.str();
}

// ==================== 变换相关 ====================
const Transform &SceneNode::GetWorldTransform() const
{
  return m_WorldTransform;
}
bool SceneNode::IsTransformDirty() const
{
  return m_TransformDirty;
}

void SceneNode::MarkTransformDirty(Transform transformBias)
{
  if (!m_TransformDirty) {
    m_TransformDirty = true;
    m_TransformBias = transformBias;

    // 向下传递Dirty标记
    MarkChildrenTransformDirty();
  }
}
void SceneNode::ClearTransformDirty()
{
  m_TransformDirty = false;
  m_TransformBias = Transform(1.0f);
}
// ==================== 包围盒相关 ====================

BoundingVolume SceneNode::GetWorldBounds() const
{
  return m_WorldBounds;
}

bool SceneNode::IsBoundsDirty() const
{
  return m_BoundsDirty;
}
void SceneNode::MarkBoundsDirty()
{
  if (!m_BoundsDirty) {
    m_BoundsDirty = true;

    // 向下传递Dirty标记
    MarkChildrenBoundsDirty();
  }
}
// ==================== 可见性相关 ====================
bool SceneNode::IsWorldVisible() const
{
  return m_WorldVisible;
}
uint32_t SceneNode::GetVisibilityMask() const
{
  return m_VisibilityMask;
}
bool SceneNode::IsVisibilityDirty() const
{
  return m_VisibilityDirty;
}
void SceneNode::MarkVisibilityDirty()
{
  if (!m_VisibilityDirty) {
    m_VisibilityDirty = true;

    // 向下传递Dirty标记
    MarkChildrenVisibilityDirty();
  }
}
// ==================== 更新操作 ====================
void SceneNode::UpdateWorldTransform(const SceneRegistry &registry)
{
  // 从TransformComponent获取局部变换矩阵
  if (registry.HasComponent<TransformComponent>(m_Entity)) {
    const auto &transformComp = registry.GetComponent<TransformComponent>(m_Entity);
    glm::mat4 localMatrix = transformComp.GetLocalMatrix();
    // 计算世界变换（先作用Bias修改，再作用新的Parent，以确保Parent修改时世界坐标不变）
    if (m_Parent && !m_Parent->IsRoot()) {
      m_WorldTransform.SetLocalMatrix(m_Parent->GetWorldTransform().GetLocalMatrix() *
                                      m_TransformBias.GetLocalMatrix() * localMatrix);
    }
    else {
      m_WorldTransform.SetLocalMatrix(m_TransformBias.GetLocalMatrix() * localMatrix);
    }
  }
  else {
    // 没有TransformComponent，使用单位矩阵
    m_WorldTransform.SetLocalMatrix(glm::mat4(1.0f));
  }
  ClearTransformDirty();
}
void SceneNode::UpdateWorldBounds(const SceneRegistry &registry)
{
  // 从BoundingVolumeComponent获取局部包围盒
  if (registry.HasComponent<BoundingVolumeComponent>(m_Entity)) {
    const auto &boundsComp = registry.GetComponent<BoundingVolumeComponent>(m_Entity);
    const BoundingVolume &localBounds = boundsComp.GetVolume();
    // 变换到世界空间
    if (localBounds.IsValid()) {
      m_WorldBounds = localBounds.Transform(m_WorldTransform.GetLocalMatrix());
    }
    else {
      // 无效的局部包围盒
      m_WorldBounds = BoundingVolume(BoundingVolumeType::None);
    }
  }
  else {
    // 没有BoundingVolumeComponent
    m_WorldBounds = BoundingVolume(BoundingVolumeType::None);
  }
  m_BoundsDirty = false;
}
void SceneNode::UpdateVisibility(const SceneRegistry &registry)
{
  // 从VisibilityComponent获取本地可见性BOOL与可见性掩码
  if (registry.HasComponent<VisibilityComponent>(m_Entity)) {
    const auto &visibilityComp = registry.GetComponent<VisibilityComponent>(m_Entity);

    // 掩码直接使用本地值
    m_VisibilityMask = visibilityComp.GetVisibilityMask();

    // 计算世界可见性：本地可见且父世界可见
    bool localVisible = visibilityComp.IsVisible();
    bool parentWorldVisible = (m_Parent == nullptr || m_Parent->IsWorldVisible());
    m_WorldVisible = localVisible && parentWorldVisible;
  }
  else {
    // 没有VisibilityComponent时的默认行为
    bool parentWorldVisible = (m_Parent == nullptr || m_Parent->IsWorldVisible());
    m_WorldVisible = parentWorldVisible;  // 默认本地可见
    m_VisibilityMask = 0xFFFFFFFF;        // 默认全掩码
  }
  m_VisibilityDirty = false;
}
void SceneNode::Update(const SceneRegistry &registry, bool force)
{
  if (m_VisibilityDirty || force) {
    UpdateVisibility(registry);
  }
  if (m_TransformDirty || force) {
    UpdateWorldTransform(registry);
  }
  if (m_BoundsDirty || force) {
    UpdateWorldBounds(registry);
  }
  // 递归更新子节点
  for (auto child : m_Children) {
    child->Update(registry, force);
  }
}

void SceneNode::MarkChildrenTransformDirty()
{
  for (auto child : m_Children) {
    child->MarkTransformDirty(m_TransformBias);
  }
}

void SceneNode::MarkChildrenBoundsDirty()
{
  for (auto child : m_Children) {
    child->MarkBoundsDirty();
  }
}
void SceneNode::MarkChildrenVisibilityDirty()
{
  for (auto child : m_Children) {
    child->MarkVisibilityDirty();
  }
}
}  // namespace mite