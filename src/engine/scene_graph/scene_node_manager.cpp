#include "scene_node_manager.h"
#include "scene_core/scene_registry.h"
#include "scene_core_components/transform_component.h"
#include "scene_core_components/bounding_volume_component.h"

namespace mite {
SceneNodeManager::SceneNodeManager(SpatialPartitionManager &spatialPartition)
    : m_SpatialPartition(spatialPartition)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneGraph NodeManager");
}
void SceneNodeManager::Clear()
{
  // 清空所有节点（会自动处理父子关系）
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_EntityToNodeMap.clear();
  m_DirtyNodes.clear();
  m_PathToNodeCache.clear();
  m_PathCacheDirty = false;
}
// ==================== 场景节点生命周期管理 ====================
SceneNode *SceneNodeManager::CreateNode(SceneRegistry &registry, Entity entity)
{
  if (!entity.IsValid()) {
    m_Logger->warn("Attempted to create node for invalid entity");
    return nullptr;
  }
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 检查是否已存在节点
  if (m_EntityToNodeMap.find(entity) != m_EntityToNodeMap.end()) {
    m_Logger->warn("Scene node already exists for entity {}", entity.GetUUIDString());
    return m_EntityToNodeMap[entity].get();
  }

  try {
    // 创建新的场景节点
    auto node = std::make_unique<SceneNode>(entity);
    SceneNode *nodePtr = node.get();

    // 添加到映射表
    m_EntityToNodeMap[entity] = std::move(node);

    // 立即更新节点的世界变换和包围盒
    nodePtr->Update(registry, true);  // force update

    // 添加到空间划分结构
    m_SpatialPartition.AddNodeToSpatialPartition(nodePtr);

    // 标记路径缓存为脏
    m_PathCacheDirty = true;

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
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 检查是否存在被删除的节点
  auto it = m_EntityToNodeMap.find(entity);
  if (it == m_EntityToNodeMap.end()) {
    m_Logger->warn("Scene node not found for entity {}", entity.GetUUIDString());
    return false;
  }
  SceneNode *node = it->second.get();

  // 从空间划分结构中移除
  m_SpatialPartition.RemoveNodeFromSpatialPartition(node);

  // 处理父子关系：将所有子节点提升为根节点
  auto children = node->GetChildren();
  for (SceneNode *child : children) {
    SetParent(child, nullptr);
  }

  // 如果自身有父节点，从父节点中移除
  if (node->GetParent()) {
    node->GetParent()->RemoveChild(node);
  }

  // 从映射表中移除
  m_EntityToNodeMap.erase(it);

  // 从脏节点列表中移除
  m_DirtyNodes.erase(std::remove(m_DirtyNodes.begin(), m_DirtyNodes.end(), entity),
                     m_DirtyNodes.end());

  // 标记路径缓存为脏
  m_PathCacheDirty = true;

  m_Logger->debug("Destroyed scene node for entity {}", entity.GetUUIDString());
  return true;
}

// ==================== 场景节点查询接口 ====================
SceneNode *SceneNodeManager::GetNode(Entity entity) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  auto it = m_EntityToNodeMap.find(entity);
  return it != m_EntityToNodeMap.end() ? it->second.get() : nullptr;
}

bool SceneNodeManager::HasNode(Entity entity) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_EntityToNodeMap.find(entity) != m_EntityToNodeMap.end();
}

std::vector<SceneNode *> SceneNodeManager::GetRootNodes() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  std::vector<SceneNode *> rootNodes;

  // 执行遍历操作，检查Root
  for (const auto &[entity, node] : m_EntityToNodeMap) {
    if (node->IsRoot()) {
      rootNodes.push_back(node.get());
    }
  }
  return rootNodes;
}

std::vector<SceneNode *> SceneNodeManager::GetAllNodes() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  std::vector<SceneNode *> nodes;
  nodes.reserve(m_EntityToNodeMap.size());

  // 遍历赋值
  for (const auto &[entity, node] : m_EntityToNodeMap) {
    nodes.push_back(node.get());
  }
  return nodes;
}

size_t SceneNodeManager::GetNodeCount() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_EntityToNodeMap.size();
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
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 简单的路径查找实现（可根据需要优化）
  for (const auto &[entity, node] : m_EntityToNodeMap) {
    if (GetNodePath(node.get()) == path) {
      return node.get();
    }
  }

  return nullptr;
}

void SceneNodeManager::TraverseTree(std::function<bool(SceneNode *)> callback,
                                    TraversalType traversalType) const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  for (const auto &[entity, node] : m_EntityToNodeMap) {
    if (node->IsRoot()) {
      bool shouldContinue = true;

      switch (traversalType) {
        case TraversalType::DepthFirstPreOrder:
          shouldContinue = TraverseDepthFirstPreOrder(node.get(), callback);
          break;
        case TraversalType::DepthFirstPostOrder:
          shouldContinue = TraverseDepthFirstPostOrder(node.get(), callback);
          break;
        case TraversalType::BreadthFirst:
          shouldContinue = TraverseBreadthFirst(node.get(), callback);
          break;
        case TraversalType::ReverseBreadthFirst:
          shouldContinue = TraverseReverseBreadthFirst(node.get(), callback);
          break;
      }

      if (!shouldContinue) {
        break;  // 回调函数要求中断遍历
      }
    }
  }
}

bool SceneNodeManager::IsEmpty() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_EntityToNodeMap.empty();
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
  std::lock_guard<std::mutex> lock(m_Mutex);

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

  // 标记路径缓存为脏（父子关系变化会影响路径）
  m_PathCacheDirty = true;

  m_Logger->debug("Reparented node {}.", node->GetEntity().GetUUIDString());

  return true;
}

void SceneNodeManager::MarkNodeDirty(Entity entity)
{
  // 避免重复添加
  if (std::find(m_DirtyNodes.begin(), m_DirtyNodes.end(), entity) == m_DirtyNodes.end()) {
    m_DirtyNodes.push_back(entity);
  }
}

void SceneNodeManager::Update(SceneRegistry &registry)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 更新所有脏节点
  if (m_DirtyNodes.empty()) {
    return;
  }

  for (Entity entity : m_DirtyNodes) {
    auto it = m_EntityToNodeMap.find(entity);
    if (it != m_EntityToNodeMap.end()) {
      // 更新节点的世界变换和包围盒
      it->second->Update(registry);

      // 更新空间划分结构中的节点位置
      m_SpatialPartition.Update(it->second.get());
    }
  }

  m_Logger->trace("Updated {} dirty nodes", m_DirtyNodes.size());
  m_DirtyNodes.clear();
}
// ==================== 私有工具方法 ====================
bool SceneNodeManager::TraverseDepthFirstPreOrder(SceneNode *node,
                                                  std::function<bool(SceneNode *)> callback) const
{
  if (!node || !callback) {
    return true;
  }
  // 先处理当前节点
  if (!callback(node)) {
    return false;
  }
  // 递归处理所有子节点
  for (SceneNode *child : node->GetChildren()) {
    if (!TraverseDepthFirstPreOrder(child, callback)) {
      return false;
    }
  }
  return true;
}
bool SceneNodeManager::TraverseDepthFirstPostOrder(SceneNode *node,
                                                   std::function<bool(SceneNode *)> callback) const
{
  if (!node || !callback) {
    return true;
  }
  // 先递归处理所有子节点
  for (SceneNode *child : node->GetChildren()) {
    if (!TraverseDepthFirstPostOrder(child, callback)) {
      return false;
    }
  }
  // 最后处理当前节点
  if (!callback(node)) {
    return false;
  }
  return true;
}
bool SceneNodeManager::TraverseBreadthFirst(SceneNode *node,
                                            std::function<bool(SceneNode *)> callback) const
{
  if (!node || !callback) {
    return true;
  }
  std::queue<SceneNode *> nodeQueue;
  nodeQueue.push(node);
  while (!nodeQueue.empty()) {
    SceneNode *currentNode = nodeQueue.front();
    nodeQueue.pop();
    // 处理当前节点
    if (!callback(currentNode)) {
      return false;
    }
    // 将子节点加入队列
    for (SceneNode *child : currentNode->GetChildren()) {
      nodeQueue.push(child);
    }
  }
  return true;
}
bool SceneNodeManager::TraverseReverseBreadthFirst(SceneNode *node,
                                                   std::function<bool(SceneNode *)> callback) const
{
  if (!node || !callback) {
    return true;
  }
  std::vector<SceneNode *> nodes;
  std::queue<SceneNode *> nodeQueue;
  nodeQueue.push(node);
  // 先收集所有节点（广度优先顺序）
  while (!nodeQueue.empty()) {
    SceneNode *currentNode = nodeQueue.front();
    nodeQueue.pop();

    nodes.push_back(currentNode);
    for (SceneNode *child : currentNode->GetChildren()) {
      nodeQueue.push(child);
    }
  }
  // 反向遍历节点（从底层到根）
  for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
    if (!callback(*it)) {
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
void SceneNodeManager::BuildPathCache() const
{
  if (!m_PathCacheDirty) {
    return;
  }
  m_PathToNodeCache.clear();
  // 使用BFS构建路径缓存，避免递归深度过大
  std::queue<SceneNode *> nodeQueue;

  // 将所有根节点加入队列
  for (const auto &[entity, node] : m_EntityToNodeMap) {
    if (node->IsRoot()) {
      nodeQueue.push(node.get());
    }
  }
  while (!nodeQueue.empty()) {
    SceneNode *current = nodeQueue.front();
    nodeQueue.pop();
    // 计算当前节点路径并加入缓存
    std::string path = CalculateNodePath(current);
    m_PathToNodeCache[path] = current;
    // 将子节点加入队列
    for (SceneNode *child : current->GetChildren()) {
      nodeQueue.push(child);
    }
  }
  // 清理脏标记
  m_PathCacheDirty = false;
  m_Logger->debug("Built path cache with {} entries", m_PathToNodeCache.size());
}

std::string SceneNodeManager::CalculateNodePath(SceneNode *node) const
{
  if (!node) {
    return "Invalid";
  }
  std::vector<std::string> pathSegments;
  SceneNode *current = node;
  // 向上遍历构建路径段
  while (current) {
    pathSegments.push_back("Entity_" + current->GetEntity().GetUUIDString());
    current = current->GetParent();
  }
  // 反转路径段（从根到当前节点）
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

}  // namespace mite