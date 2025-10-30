#include "scene_node_manager.h"

namespace mite {
SceneNodeManager::SceneNodeManager(SpatialPartition &spatialPartition)
    : m_SpatialPartition(spatialPartition)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneGraph NodeManager");

  // 订阅变换、包围盒、可见性更新事件
  // Immediate同步模式：
  // 变换、包围盒、可见性更新是场景图核心操作，需要立即标记脏状态以确保后续渲染正确性
  m_EventSubscriptions.SubscribeImmediate<TransformUpdatedEvent>(
      BIND_DISPATCH_FN(OnTransformComponentUpdated),
      EventPriority::High  // 变换更新优先级高，影响层级关系
  );
  m_EventSubscriptions.SubscribeImmediate<BoundingVolumeChangedEvent>(
      BIND_DISPATCH_FN(OnBoundingVolumeComponentUpdated), EventPriority::Normal);
  m_EventSubscriptions.SubscribeImmediate<VisibilityChangedEvent>(
      BIND_DISPATCH_FN(OnVisibilityComponentUpdated),
      EventPriority::High  // 可见性更新优先级高，影响渲染流程
  );

  // 订阅父子关系变换（延迟处理）
  m_EventSubscriptions.SubscribeDeferred<SceneNodeParentChangeEvent>(
      BIND_DISPATCH_FN(OnSceneNodeParentChange));

  m_Logger->info("SceneGraph NodeManager created with spatial partition type: {}, name: {}",
                 GetSpatialPartitionTypeName(SpatialPartitionType::BVH),
                 m_SpatialPartition.GetTypeName());
}
void SceneNodeManager::Clear()
{
  // 清空所有节点（会自动处理父子关系）
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_EntityToNodeMap.clear();
  m_LightNodes.clear();
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

    // 如果包含光照组件，则被认为是光源节点，需要参与每帧的光照构建环节
    if (registry.HasComponent<LightComponent>(entity)) {
      m_LightNodes.insert(nodePtr);
    }

    // 根据可见性决定是否添加到空间划分结构
    if (nodePtr->IsWorldVisible()) {
      m_SpatialPartition.Insert(nodePtr);
    }

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

  // 从空间划分结构中移除（无论是否可见）
  m_SpatialPartition.Remove(node);

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

  // 从光源节点列表中移除
  if (m_LightNodes.find(GetNode(entity)) != m_LightNodes.end())
    m_LightNodes.erase(GetNode(entity));

  // 从脏节点列表中移除
  if (m_DirtyNodes.find(GetNode(entity)) != m_DirtyNodes.end())
    m_DirtyNodes.erase(GetNode(entity));

  // 标记路径缓存为脏
  m_PathCacheDirty = true;

  m_Logger->debug("Destroyed scene node for entity {}", entity.GetUUIDString());
  return true;
}

// ==================== 场景节点查询接口 ====================
SceneNode *SceneNodeManager::GetNode(Entity entity) const
{
  auto it = m_EntityToNodeMap.find(entity);
  return it != m_EntityToNodeMap.end() ? it->second.get() : nullptr;
}

bool SceneNodeManager::HasNode(Entity entity) const
{
  return m_EntityToNodeMap.find(entity) != m_EntityToNodeMap.end();
}

std::vector<SceneNode *> SceneNodeManager::GetRootNodes() const
{
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
  std::vector<SceneNode *> nodes;
  nodes.reserve(m_EntityToNodeMap.size());

  // 遍历赋值
  for (const auto &[entity, node] : m_EntityToNodeMap) {
    nodes.push_back(node.get());
  }
  return nodes;
}

std::vector<SceneNode *> SceneNodeManager::GetLightNodes() const
{
  std::vector<SceneNode *> nodes;
  nodes.reserve(m_LightNodes.size());

  // 遍历赋值
  for (const auto node : m_LightNodes) {
    nodes.push_back(node);
  }
  return nodes;
}

size_t SceneNodeManager::GetNodeCount() const
{
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

  // 标记路径缓存为脏（父子关系变化会影响路径）
  m_PathCacheDirty = true;

  m_Logger->debug("Reparented node {}.", node->GetEntity().GetUUIDString());

  return true;
}

void SceneNodeManager::MarkNodeDirty(SceneNode *node)
{
  if (!node)
    return;

  // 避免重复添加
  if (m_DirtyNodes.find(node) == m_DirtyNodes.end()) {
    m_DirtyNodes.insert(node);
  }
}
void SceneNodeManager::MarkNodeDirtyRecursive(SceneNode *node)
{
  // 标记当前节点
  MarkNodeDirty(node);

  // 创建递归函数
  std::function<void(SceneNode *)> markChildren = [&](SceneNode *currentNode) {
    for (SceneNode *child : currentNode->GetChildren()) {
      MarkNodeDirty(child);
      markChildren(child);
    }
  };

  // 递归标记所有子节点
  markChildren(node);
}
void SceneNodeManager::Update(SceneRegistry &registry)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  // 更新所有脏节点
  if (m_DirtyNodes.empty()) {
    return;
  }

  // 按节点深度排序，确保父节点先于子节点更新
  std::vector<std::pair<Entity, int>> sortedDirtyNodes;
  sortedDirtyNodes.reserve(m_DirtyNodes.size());

  // 收集脏节点及其深度
  for (SceneNode *dirtyNode : m_DirtyNodes) {
    if (dirtyNode) {
      int depth = dirtyNode->GetDepth();
      sortedDirtyNodes.emplace_back(dirtyNode->GetEntity(), depth);
    }
  }

  // 按深度升序排序（深度小的先更新）
  std::sort(sortedDirtyNodes.begin(), sortedDirtyNodes.end(), [](const auto &a, const auto &b) {
    return a.second < b.second;
  });

  // 按排序后的顺序更新节点
  for (const auto &[entity, depth] : sortedDirtyNodes) {
    auto it = m_EntityToNodeMap.find(entity);
    if (it != m_EntityToNodeMap.end()) {
      // 更新节点的世界变换、包围盒和可见性
      it->second->Update(registry);

      // 根据可见性状态更新空间划分结构
      if (it->second->IsWorldVisible()) {
        // 可见节点：插入或更新到空间划分结构
        if (!m_SpatialPartition.Contains(it->second.get())) {
          m_SpatialPartition.Insert(it->second.get());
        }
        else {
          m_SpatialPartition.Update(it->second.get());
        }
      }
      else {
        // 不可见节点：从空间划分结构中移除
        m_SpatialPartition.Remove(it->second.get());
      }
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

void SceneNodeManager::OnTransformComponentUpdated(TransformUpdatedEvent &e)
{
  // 获取Node并检查可用性
  SceneNode *node = GetNode(e.GetEntity());
  if (!node)
    return;

  // 标记为TransformDirty与BoundingVolumeDirty（变换改变通常伴随着包围盒改变），等待Update阶段执行更新
  node->MarkTransformDirty();
  node->MarkBoundsDirty();

  // 变换更新影响当前节点及其所有子节点的世界变换（Manager独立的脏标记收集器）
  MarkNodeDirtyRecursive(node);

  // 标记已处理但允许传播（其他系统可能需要知道变换更新）
  e.SetResult(EventResult::Handled);
}

void SceneNodeManager::OnBoundingVolumeComponentUpdated(BoundingVolumeChangedEvent &e)
{
  // 获取Node并检查可用性
  SceneNode *node = GetNode(e.GetEntity());
  if (!node)
    return;

  // 标记为BoundingVolumeDirty，等待Update阶段执行更新
  node->MarkBoundsDirty();

  // 包围盒更新只影响当前节点的世界包围盒
  MarkNodeDirty(node);

  // 标记已处理但允许传播（碰撞检测等系统可能需要包围盒更新）
  e.SetResult(EventResult::Handled);
}

void SceneNodeManager::OnVisibilityComponentUpdated(VisibilityChangedEvent &e)
{
  // 获取Node并检查可用性
  SceneNode *node = GetNode(e.GetEntity());
  if (!node)
    return;

  // 标记为VisibilityDirty，等待Update阶段执行更新
  node->MarkVisibilityDirty();

  // 可见性更新影响当前节点及其所有子节点的世界可见性
  MarkNodeDirtyRecursive(node);

  // 标记已处理但允许传播（UI系统等可能需要可见性信息）
  e.SetResult(EventResult::Handled);
}
void SceneNodeManager::OnSceneNodeParentChange(SceneNodeParentChangeEvent &e)
{
  if (e.GetSceneNode()) {
    e.GetSceneNode()->SetParent(e.GetNewParent());
    e.SetResult(EventResult::HandledAndStop);
    return;
  }
}
}  // namespace mite