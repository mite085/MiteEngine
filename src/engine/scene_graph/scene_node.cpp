#include "scene_node.h"

#include "scene_core/scene_registry.h"
#include "scene_core_components/bounding_volume_component.h"
#include "scene_core_components/transform_component.h"
#include "scene_core_components/visibility_component.h"

namespace mite {
SceneNode::SceneNode(Entity entity)
    : m_Entity(entity),
      m_WorldBounds(BoundingVolume(BoundingVolumeType::None)) {
  // 初始化世界包围盒为无效状态，实际应当通过包围盒组件获取
}

SceneNode::~SceneNode() {
  // 从父节点中移除自己
  if (m_Parent) {
    m_Parent->RemoveChild(shared_from_this());
  }

  // 清空子节点（子节点会自动设置父节点为nullptr）
  for (auto child : m_Children) {
    child->m_Parent = nullptr;
  }
  m_Children.clear();
}
// ==================== 实体和关系操作 ====================
Entity SceneNode::GetEntity() { return m_Entity; }

void SceneNode::SetParent(std::shared_ptr<SceneNode> parent) {
  if (m_Parent == parent) return;

  // 递归检查parent是否为当前node的child，避免循环依赖
  if (IsChild(parent)) {
    return;
  }

  // 记录原父节点的变换，从原父节点移除
  Transform oldParentWorldTransform;
  if (m_Parent) {
    oldParentWorldTransform = m_Parent->GetWorldTransform();
    m_Parent->RemoveChild(shared_from_this());
  }

  // 记录新父节点的变换，添加到新父节点
  m_Parent = parent;
  Transform newParentWorldTransform;
  if (m_Parent) {
    newParentWorldTransform = m_Parent->GetWorldTransform();
    m_Parent->AddChild(shared_from_this());
  }

  // 确保world不变，计算bias偏差
  // world = oldParentWorld * local = newParentWorld * newLocal
  // newLocal = inv(newParentWorld) * oldParentWorld * local
  m_transformBias = glm::inverse(newParentWorldTransform.GetLocalMatrix()) *
                    oldParentWorldTransform.GetLocalMatrix();
  // 标记需要更新变换
  MarkTransformDirty();
  MarkBoundsDirty();
  MarkVisibilityDirty();
}
std::shared_ptr<SceneNode> SceneNode::GetParent() const { return m_Parent; }
void SceneNode::AddChild(std::shared_ptr<SceneNode> child) {
  if (child == nullptr || child == shared_from_this()) return;

  // 检查是否已经是子节点
  if (std::find(m_Children.begin(), m_Children.end(), child) !=
      m_Children.end()) {
    return;
  }

  // 添加子节点
  m_Children.push_back(child);
  child->m_Parent = shared_from_this();
}
bool SceneNode::RemoveChild(std::shared_ptr<SceneNode> child) {
  if (child == nullptr) return false;

  // 检查是否存在该子节点
  auto it = std::find(m_Children.begin(), m_Children.end(), child);
  if (it != m_Children.end()) {
    // 执行移除操作
    m_Children.erase(it);
    child->m_Parent = nullptr;
    return true;
  }

  return false;
}
bool SceneNode::IsChild(std::shared_ptr<SceneNode> child) {
  // 遍历children
  for (std::shared_ptr<SceneNode> node : GetChildren()) {
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
const std::vector<std::shared_ptr<SceneNode> > &SceneNode::GetChildren() const {
  return m_Children;
}

// ==================== 状态查询 ====================
bool SceneNode::IsRoot() const { return m_Parent == nullptr; }
bool SceneNode::IsLeaf() const { return m_Children.empty(); }
int SceneNode::GetDepth() const {
  // 向上追溯的同时计数
  int depth = 0;
  std::shared_ptr<SceneNode> current = m_Parent;
  while (current != nullptr) {
    depth++;
    current = current->m_Parent;
  }
  return depth;
}

std::string SceneNode::GetPath() {
  // 向上追溯的同时，累积pathParts
  std::vector<std::string> pathParts;
  std::shared_ptr<SceneNode> current = shared_from_this();

  while (current != nullptr) {
    pathParts.push_back("Entity_" + current->GetEntity().GetUUIDString());
    current = current->m_Parent;
  }

  // 反转容器中元素的顺序，确保顺序是从最顶部向下的
  std::reverse(pathParts.begin(), pathParts.end());

  // 合并字符串
  std::stringstream ss;
  for (size_t i = 0; i < pathParts.size(); ++i) {
    if (i > 0) ss << "/";
    ss << pathParts[i];
  }

  return ss.str();
}

// ==================== 变换相关 ====================
const Transform &SceneNode::GetWorldTransform() const {
  return m_WorldTransform;
}
bool SceneNode::IsTransformDirty() const { return m_TransformDirty; }

void SceneNode::MarkTransformDirty() {
  if (!m_TransformDirty) {
    m_TransformDirty = true;

    // 向下传递Dirty标记
    for (auto child : m_Children) {
      child->MarkTransformDirty();
    }
  }
}
// ==================== 包围盒相关 ====================

BoundingVolume SceneNode::GetWorldBounds() const { return m_WorldBounds; }

bool SceneNode::IsBoundsDirty() const { return m_BoundsDirty; }
void SceneNode::MarkBoundsDirty() {
  if (!m_BoundsDirty) {
    m_BoundsDirty = true;

    // 向下传递Dirty标记
    for (auto child : m_Children) {
      child->MarkBoundsDirty();
    }
  }
}
// ==================== 可见性相关 ====================
bool SceneNode::IsWorldVisible() const { return m_WorldVisible; }
uint32_t SceneNode::GetVisibilityMask() const { return m_VisibilityMask; }
bool SceneNode::IsVisibilityDirty() const { return m_VisibilityDirty; }
void SceneNode::MarkVisibilityDirty() {
  if (!m_VisibilityDirty) {
    m_VisibilityDirty = true;

    // 向下传递Dirty标记
    for (auto child : m_Children) {
      child->MarkVisibilityDirty();
    }
  }
}
// ==================== 更新操作 ====================
void SceneNode::UpdateWorldTransform(const SceneRegistry &registry) {
  // 从TransformComponent获取局部变换矩阵
  if (registry.HasComponent<TransformComponent>(m_Entity)) {
    TransformComponent &transformComp =
        registry.GetComponent<TransformComponent>(m_Entity);

    // 更新之前先作用父子关系修改导致的bias到局部坐标，并清空bias。
    transformComp.SetLocalTransform(m_transformBias *
                                    transformComp.GetLocalTransform());
    m_transformBias = glm::mat4(1.0f);

    // 计算世界变换
    const Transform &localTransform = transformComp.GetLocalTransform();
    if (m_Parent) {
      m_WorldTransform =
          m_Parent->GetWorldTransform().GetLocalMatrix() * localTransform;
    } else {
      m_WorldTransform = localTransform;
    }
  } else {
    // 没有TransformComponent，使用单位矩阵
    m_WorldTransform.SetLocalMatrix(glm::mat4(1.0f));
  }
  m_TransformDirty = false;
}
void SceneNode::UpdateWorldBounds(const SceneRegistry &registry) {
  // 从BoundingVolumeComponent获取局部包围盒
  if (registry.HasComponent<BoundingVolumeComponent>(m_Entity)) {
    const auto &boundsComp =
        registry.GetComponent<BoundingVolumeComponent>(m_Entity);
    const BoundingVolume &localBounds = *boundsComp.GetVolume();
    // 变换到世界空间
    if (localBounds.IsValid()) {
      m_WorldBounds = localBounds.Transform(m_WorldTransform.GetLocalMatrix());
    } else {
      // 无效的局部包围盒
      m_WorldBounds = BoundingVolume(BoundingVolumeType::None);
    }
  } else {
    // 没有BoundingVolumeComponent
    m_WorldBounds = BoundingVolume(BoundingVolumeType::None);
  }
  m_BoundsDirty = false;
}
void SceneNode::UpdateVisibility(const SceneRegistry &registry) {
  // 从VisibilityComponent获取本地可见性BOOL与可见性掩码
  if (registry.HasComponent<VisibilityComponent>(m_Entity)) {
    const auto &visibilityComp =
        registry.GetComponent<VisibilityComponent>(m_Entity);

    // 掩码直接使用本地值
    m_VisibilityMask = visibilityComp.GetVisibilityMask();

    // 计算世界可见性：本地可见且父世界可见
    bool localVisible = visibilityComp.IsVisible();
    bool parentWorldVisible =
        (m_Parent == nullptr || m_Parent->IsWorldVisible());
    m_WorldVisible = localVisible && parentWorldVisible;
  } else {
    // 没有VisibilityComponent时的默认行为
    bool parentWorldVisible =
        (m_Parent == nullptr || m_Parent->IsWorldVisible());
    m_WorldVisible = parentWorldVisible;  // 默认本地可见
    m_VisibilityMask = 0xFFFFFFFF;        // 默认全掩码
  }
  m_VisibilityDirty = false;
}
void SceneNode::Update(const SceneRegistry &registry, bool force) {
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
}  // namespace mite