#include "scene_node.h"
#include "scene_core/scene_registry.h"
#include "scene_core_components/hierarchy_component.h"
#include "scene_core_components/transform_component.h"
#include "visibility_component.h"

namespace mite {

SceneNode::SceneNode(Entity entity) : entity_(entity)
{
  localTransform_ = glm::mat4(1.0f);
  worldTransform_ = glm::mat4(1.0f);
  transformDirty_ = true;
  boundsDirty_ = true;
}

SceneNode::~SceneNode()
{
  // 从父节点中移除自己
  if (parent_) {
    parent_->RemoveChild(this);
  }

  // 清空子节点（子节点会自动设置父节点为nullptr）
  for (auto child : children_) {
    child->parent_ = nullptr;
  }
  children_.clear();
}

void SceneNode::SetParent(SceneNode *parent)
{
  if (parent_ == parent)
    return;

  // 从原父节点移除
  if (parent_) {
    parent_->RemoveChild(this);
  }

  parent_ = parent;

  // 添加到新父节点
  if (parent_) {
    parent_->AddChild(this);
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
  if (std::find(children_.begin(), children_.end(), child) != children_.end()) {
    return;
  }

  children_.push_back(child);
  child->parent_ = this;

  // 标记子节点需要更新
  child->MarkTransformDirty();
  child->MarkBoundsDirty();
}

bool SceneNode::RemoveChild(SceneNode *child)
{
  if (child == nullptr)
    return false;

  auto it = std::find(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    children_.erase(it);
    child->parent_ = nullptr;

    // 标记子节点需要更新
    child->MarkTransformDirty();
    child->MarkBoundsDirty();
    return true;
  }

  return false;
}

void SceneNode::SetLocalTransform(const glm::mat4 &localTransform)
{
  if (localTransform_ != localTransform) {
    localTransform_ = localTransform;
    MarkTransformDirty();
    MarkBoundsDirty();
  }
}

void SceneNode::SetLocalBounds(const AABB &localBounds)
{
  if (localBounds_.min != localBounds.min || localBounds_.max != localBounds.max) {
    localBounds_ = localBounds;
    MarkBoundsDirty();
  }
}

void SceneNode::MarkTransformDirty()
{
  if (!transformDirty_) {
    transformDirty_ = true;
    MarkChildrenTransformDirty();
  }
}

void SceneNode::MarkBoundsDirty()
{
  if (!boundsDirty_) {
    boundsDirty_ = true;
    MarkChildrenBoundsDirty();
  }
}

void SceneNode::MarkChildrenTransformDirty()
{
  for (auto child : children_) {
    child->MarkTransformDirty();
  }
}

void SceneNode::MarkChildrenBoundsDirty()
{
  for (auto child : children_) {
    child->MarkBoundsDirty();
  }
}

void SceneNode::Update(bool force)
{
  if (transformDirty_ || force) {
    UpdateWorldTransform();
    transformDirty_ = false;
  }

  if (boundsDirty_ || force) {
    UpdateWorldBounds();
    boundsDirty_ = false;
  }

  // 递归更新子节点
  for (auto child : children_) {
    child->Update(force);
  }
}

void SceneNode::UpdateWorldTransform()
{
  if (parent_ && !parent_->IsRoot()) {
    // 有父节点：世界变换 = 父世界变换 × 局部变换
    worldTransform_ = parent_->GetWorldTransform() * localTransform_;
  }
  else {
    // 根节点：世界变换 = 局部变换
    worldTransform_ = localTransform_;
  }
}

void SceneNode::UpdateWorldBounds()
{
  if (localBounds_.IsValid()) {
    // 变换局部包围盒到世界空间
    worldBounds_ = localBounds_.Transform(worldTransform_);
  }
  else {
    // 无效的局部包围盒，使用默认值
    worldBounds_ = AABB(glm::vec3(0.0f), glm::vec3(0.0f));
  }
}

int SceneNode::GetDepth() const
{
  int depth = 0;
  SceneNode *current = parent_;
  while (current != nullptr) {
    depth++;
    current = current->parent_;
  }
  return depth;
}

std::string SceneNode::GetPath() const
{
  std::vector<std::string> pathParts;
  SceneNode *current = const_cast<SceneNode *>(this);

  while (current != nullptr) {
    pathParts.push_back("Entity_" + current->GetEntity().GetUUIDString());
    current = current->parent_;
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
  if (!entity_.IsValid()) {
    return false;
  }

  if (registry.HasComponent<VisibilityComponent>(entity_)) {
    auto &visibilityComp = registry.GetComponent<VisibilityComponent>(entity_);
    return visibilityComp.IsVisible() && visibilityComp.MatchesMask(visibilityMask);
  }

  // 没有VisibilityComponent的节点默认可见
  return true;
}

}  // namespace mite
