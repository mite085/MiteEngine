#include "scene_node_manager.h"
#include "scene_core/scene_registry.h"
#include "visibility_component.h"

namespace mite {
SceneNodeManager::SceneNodeManager(SpatialPartitionManager &spatialPartition)
    : m_spatialPartition(spatialPartition)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneGraph NodeManager");
}
void SceneNodeManager::Clear()
{
  // 清空所有节点（会自动处理父子关系）
  m_entityToNodeMap.clear();
  m_dirtyNodes.clear();
}
// ==================== 场景节点生命周期管理 ====================
SceneNode *SceneNodeManager::CreateNode(SceneRegistry &registry, Entity entity)
{
  if (!entity.IsValid()) {
    m_Logger->warn("Attempted to create node for invalid entity");
    return nullptr;
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  // 检查是否已存在节点
  if (m_entityToNodeMap.find(entity) != m_entityToNodeMap.end()) {
    m_Logger->warn("Scene node already exists for entity {}", entity.GetUUIDString());
    return m_entityToNodeMap[entity].get();
  }

  try {
    // 创建新的场景节点
    auto node = std::make_unique<SceneNode>(entity);
    SceneNode *nodePtr = node.get();

    // 添加到映射表
    m_entityToNodeMap[entity] = std::move(node);

    // 如果实体有VisibilityComponent，初始化其局部包围盒
    if (registry.HasComponent<VisibilityComponent>(entity)) {
      auto &visibilityComp = registry.GetComponent<VisibilityComponent>(entity);
      visibilityComp.SetLocalAABB(nodePtr->GetLocalBounds());
      visibilityComp.MarkBoundsDirty();
    }

    // 添加到空间划分结构
    m_spatialPartition.AddNodeToSpatialPartition(nodePtr);

    m_Logger->debug("Created scene node for entity {}", entity.GetUUIDString());
    return nodePtr;
  }
  catch (const std::exception &e) {
    m_Logger->error(
        "Failed to create scene node for entity {}: {}", entity.GetUUIDString(), e.what());
    return nullptr;
  }
}

bool SceneNodeManager::DestroyNode(SceneRegistry &registry, Entity entity)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_entityToNodeMap.find(entity);
  if (it == m_entityToNodeMap.end()) {
    m_Logger->warn("Scene node not found for entity {}", entity.GetUUIDString());
    return false;
  }

  SceneNode *node = it->second.get();

  // 从空间划分结构中移除
  m_spatialPartition.RemoveNodeFromSpatialPartition(node);

  // 处理父子关系：将所有子节点提升为根节点
  auto children = node->GetChildren();
  for (SceneNode *child : children) {
    SetParent(child, nullptr);
  }

  // 如果自身有父节点，从父节点中移除
  if (node->GetParent()) {
    node->GetParent()->RemoveChild(node);
  }

  // 清理VisibilityComponent相关状态
  if (registry.HasComponent<VisibilityComponent>(entity)) {
    auto &visibilityComp = registry.GetComponent<VisibilityComponent>(entity);
    visibilityComp.SetVisible(false);  // 标记为不可见
  }

  // 从映射表中移除
  m_entityToNodeMap.erase(it);

  // 从脏节点列表中移除
  m_dirtyNodes.erase(std::remove(m_dirtyNodes.begin(), m_dirtyNodes.end(), entity),
                     m_dirtyNodes.end());

  m_Logger->debug("Destroyed scene node for entity {}", entity.GetUUIDString());
  return true;
}

// ==================== 场景节点查询接口 ====================
SceneNode *SceneNodeManager::GetNode(Entity entity) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_entityToNodeMap.find(entity);
  return it != m_entityToNodeMap.end() ? it->second.get() : nullptr;
}

bool SceneNodeManager::HasNode(Entity entity) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_entityToNodeMap.find(entity) != m_entityToNodeMap.end();
}

std::vector<SceneNode *> SceneNodeManager::GetRootNodes() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<SceneNode *> rootNodes;

  for (const auto &[entity, node] : m_entityToNodeMap) {
    if (node->IsRoot()) {
      rootNodes.push_back(node.get());
    }
  }

  return rootNodes;
}

std::vector<SceneNode *> SceneNodeManager::GetAllNodes() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<SceneNode *> nodes;
  nodes.reserve(m_entityToNodeMap.size());

  for (const auto &[entity, node] : m_entityToNodeMap) {
    nodes.push_back(node.get());
  }

  return nodes;
}

size_t SceneNodeManager::GetNodeCount() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_entityToNodeMap.size();
}

std::string SceneNodeManager::GetNodePath(SceneNode *node) const
{
  if (!node) {
    return "Invalid";
  }

  std::vector<std::string> pathSegments;
  SceneNode *current = node;

  // 向上遍历构建路径
  while (current) {
    std::stringstream ss;
    ss << "Entity_" << current->GetEntity().GetUUIDString();
    pathSegments.push_back(ss.str());
    current = current->GetParent();
  }

  // 反转路径（从根到当前节点）
  std::reverse(pathSegments.begin(), pathSegments.end());

  // 拼接路径字符串
  std::string path;
  for (size_t i = 0; i < pathSegments.size(); ++i) {
    if (i > 0)
      path += "/";
    path += pathSegments[i];
  }

  return path;
}

SceneNode *SceneNodeManager::FindNodeByPath(const std::string &path) const
{
  std::lock_guard<std::mutex> lock(m_mutex);

  // 简单的路径查找实现（可根据需要优化）
  for (const auto &[entity, node] : m_entityToNodeMap) {
    if (GetNodePath(node.get()) == path) {
      return node.get();
    }
  }

  return nullptr;
}

void SceneNodeManager::TraverseTree(std::function<bool(SceneNode *)> callback) const
{
  std::lock_guard<std::mutex> lock(m_mutex);

  // 从所有根节点开始遍历
  for (const auto &[entity, node] : m_entityToNodeMap) {
    if (node->IsRoot()) {
      if (!TraverseRecursive(node.get(), callback)) {
        break;  // 回调函数要求中断遍历
      }
    }
  }
}

bool SceneNodeManager::IsEmpty() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_entityToNodeMap.empty();
}

// ==================== 节点更新接口 ====================
bool SceneNodeManager::SetParent(SceneNode *node, SceneNode *newParent)
{
  if (!node) {
    m_Logger->warn("Attempted to set parent for null node");
    return false;
  }

  // 检查循环引用
  if (!ValidateParenting(node, newParent)) {
    m_Logger->warn("Invalid parenting operation: cyclic reference detected");
    return false;
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  // 从原父节点移除
  SceneNode *oldParent = node->GetParent();
  if (oldParent) {
    oldParent->RemoveChild(node);
  }

  // 设置新父节点
  if (newParent) {
    newParent->AddChild(node);
  }

  // 设置节点的父节点引用
  node->SetParent(newParent);

  // 标记节点需要更新（父子关系变化影响世界变换）
  MarkNodeDirty(node->GetEntity());

  m_Logger->debug("Reparented node {}.", node->GetEntity().GetUUIDString());

  return true;
}

void SceneNodeManager::UpdateNodeBounds(SceneRegistry &registry,
                                        Entity entity,
                                        const AABB &localBounds)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_entityToNodeMap.find(entity);
  if (it != m_entityToNodeMap.end()) {
    it->second->SetLocalBounds(localBounds);
    MarkNodeDirty(entity);
  }
}

void SceneNodeManager::MarkNodeDirty(Entity entity)
{
  // 避免重复添加
  if (std::find(m_dirtyNodes.begin(), m_dirtyNodes.end(), entity) == m_dirtyNodes.end()) {
    m_dirtyNodes.push_back(entity);
  }
}

void SceneNodeManager::Update(SceneRegistry &registry)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  // 更新主相机的视锥体和可见性掩码

  // 更新所有脏节点
  if (m_dirtyNodes.empty()) {
    return;
  }

  for (Entity entity : m_dirtyNodes) {
    auto it = m_entityToNodeMap.find(entity);
    if (it != m_entityToNodeMap.end()) {
      it->second->Update();

      // 更新空间划分结构中的节点位置
      m_spatialPartition.Update(it->second.get());
    }
  }

  m_Logger->trace("Updated {} dirty nodes", m_dirtyNodes.size());
  m_dirtyNodes.clear();
}
// ==================== 私有工具方法 ====================

bool SceneNodeManager::TraverseRecursive(SceneNode *node,
                                         std::function<bool(SceneNode *)> callback) const
{
  if (!node || !callback) {
    return true;
  }

  // 先处理当前节点
  if (!callback(node)) {
    return false;  // 回调要求中断遍历
  }

  // 递归处理所有子节点
  for (SceneNode *child : node->GetChildren()) {
    if (!TraverseRecursive(child, callback)) {
      return false;
    }
  }

  return true;
}

bool SceneNodeManager::ValidateParenting(SceneNode *node, SceneNode *newParent) const
{
  if (!node || node == newParent) {
    return false;  // 不能设置自己为父节点
  }

  // 检查循环引用：确保newParent不是node的子孙
  SceneNode *current = newParent;
  while (current) {
    if (current == node) {
      return false;  // 循环引用检测
    }
    current = current->GetParent();
  }

  return true;
}
}  // namespace mite