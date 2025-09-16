#include "simple_bvh.h"

namespace mite {
int BVHNode::GetHeight() const
{
  if (IsLeaf())
    return 0;
  int leftHeight = left ? left->GetHeight() : 0;
  int rightHeight = right ? right->GetHeight() : 0;
  return 1 + std::max(leftHeight, rightHeight);
}

SimpleBVH::SimpleBVH(int maxDepth, int minLeafSize)
    : m_MaxDepth(maxDepth), m_MinLeafSize(minLeafSize)
{
}

SimpleBVH::~SimpleBVH()
{
  Clear();
}
bool SimpleBVH::Contains(SceneNode *node) const
{
  return m_AllNodes.find(node) != m_AllNodes.end();
}
// ==================== 空间划分生命周期管理 ====================
void SimpleBVH::Insert(SceneNode *node)
{
  if (!node)
    return;

  // 检查是否已存在
  if (m_AllNodes.find(node) != m_AllNodes.end()) {
    return;
  }

  m_AllNodes.insert(node);
  m_NeedsRebuild = true;
}
void SimpleBVH::Remove(SceneNode *node)
{
  if (!node)
    return;

  if (m_AllNodes.find(node) != m_AllNodes.end()) {
    m_AllNodes.erase(node);
    m_NeedsRebuild = true;
  }
}
void SimpleBVH::Update(SceneNode *node)
{
  if (!node)
    return;

  // 如果节点不在BVH中，忽略更新
  if (m_AllNodes.find(node) == m_AllNodes.end()) {
    return;
  }

  // 标记节点为脏
  m_DirtyNodes.insert(node);

  // 如果脏节点数量超过阈值，或者这是第一个脏节点，设置需要更新标志
  if (m_DirtyNodes.size() > m_AllNodes.size() / 4) {
    // 脏节点太多，直接标记需要完全重建
    m_NeedsRebuild = true;
  }
  else {
    // 标记需要增量更新
    m_NeedsRebuild = false;
  }
}
void SimpleBVH::Clear()
{
  if (m_Root) {
    FreeNode(m_Root);
    m_Root = nullptr;
  }
  m_AllNodes.clear();
  m_DirtyNodes.clear();
  m_DirtyBVHNodes.clear();
  m_NodeCount = 0;
  m_NeedsRebuild = false;
}
void SimpleBVH::Rebuild()
{
  if (m_AllNodes.empty()) {
    if (m_Root) {
      FreeNode(m_Root);
      m_Root = nullptr;
    }
    m_NodeCount = 0;
    m_NeedsRebuild = false;
    ClearDirtyFlags();
    return;
  }

  // 释放旧树
  if (m_Root) {
    FreeNode(m_Root);
  }

  // 构建新树
  std::vector<SceneNode *> nodesToBuild(m_AllNodes.begin(), m_AllNodes.end());
  m_Root = BuildTree(nodesToBuild, 0, static_cast<int>(nodesToBuild.size()), 0);
  m_NeedsRebuild = false;
  ClearDirtyFlags();

  LOG_INFO("BVH rebuilt with {} nodes, depth: {}", m_NodeCount, GetDepth());
}

// ==================== 空间结构外部查询接口 ====================
bool SimpleBVH::Raycast(const Ray &ray, std::vector<SceneNode *> &results)
{
  // 检查是否需要更新
  if (m_NeedsRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_NeedsRebuild);
  }
  if (!m_Root)
    return false;

  results.clear();  
  
  // 使用最佳优先遍历进行射线查询
  RaycastBestFirst(m_Root, ray, results);
  return !results.empty();
}
bool SimpleBVH::RaycastFirst(const Ray &ray, SceneNode *&result, float &distance)
{
  // 检查是否需要更新
  if (m_NeedsRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_NeedsRebuild);
  }
  if (!m_Root)
    return false;

  result = nullptr;
  distance = std::numeric_limits<float>::max();

  // 使用最佳优先遍历寻找第一个交点
  RaycastFirstBestFirst(m_Root, ray, result, distance);
  return result != nullptr;
}
size_t SimpleBVH::FrustumCull(const Frustum &frustum, std::vector<SceneNode *> &results)
{
  // 检查是否需要更新
  if (m_NeedsRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_NeedsRebuild);
  }
  if (!m_Root)
    return 0;

  results.clear();

  // 使用广度优先遍历进行视锥体裁剪
  FrustumCullBFS(m_Root, frustum, results);
  return results.size();
}
size_t SimpleBVH::VolumeQuery(const BoundingVolume &volume, std::vector<SceneNode *> &results)
{
  // 检查是否需要更新
  if (m_NeedsRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_NeedsRebuild);
  }
  if (!m_Root)
    return 0;

  results.clear();

  // 使用广度优先遍历进行体积查询
  VolumeQueryBFS(m_Root, volume, results);
  return results.size();
}
size_t SimpleBVH::PointQuery(const glm::vec3 &point, std::vector<SceneNode *> &results)
{
  // 检查是否需要更新
  if (m_NeedsRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_NeedsRebuild);
  }
  if (!m_Root)
    return 0;

  results.clear();
  BoundingVolume pointAABB(BoundingVolumeAABB(point, point));  // 创建零大小的AABB
  return VolumeQuery(pointAABB, results);
}
bool SimpleBVH::NearestNeighbor(const glm::vec3 &point, SceneNode *&result, float maxDistance)
{
  // 检查是否需要更新
  if (m_NeedsRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_NeedsRebuild);
  }
  if (!m_Root)
    return false;

  result = nullptr;
  float bestDistance = maxDistance;
  float bestDistanceSq = bestDistance * bestDistance;

  // 使用优先队列进行最近邻搜索（最佳优先变种）
  struct QueueElement {
    BVHNode *node;
    float distanceSq;

    bool operator<(const QueueElement &other) const
    {
      return distanceSq > other.distanceSq;  // 最小堆
    }
  };

  std::priority_queue<QueueElement> queue;
  queue.push({m_Root, 0.0f});

  while (!queue.empty()) {
    QueueElement current = queue.top();
    queue.pop();

    if (current.distanceSq > bestDistanceSq) {
      continue;
    }

    if (current.node->IsLeaf()) {
      // 遍历叶子节点中的所有场景节点
      for (SceneNode *sceneNode : current.node->sceneNodes) {
        float distSq = glm::distance2(point, sceneNode->GetWorldBounds().GetCenter());
        if (distSq < bestDistanceSq) {
          bestDistanceSq = distSq;
          result = sceneNode;
        }
      }
    }
    else {
      if (current.node->left) {
        float leftDistSq = current.node->left->bounds.DistanceToPointSq(point);
        if (leftDistSq <= bestDistanceSq) {
          queue.push({current.node->left, leftDistSq});
        }
      }

      if (current.node->right) {
        float rightDistSq = current.node->right->bounds.DistanceToPointSq(point);
        if (rightDistSq <= bestDistanceSq) {
          queue.push({current.node->right, rightDistSq});
        }
      }
    }
  }

  return result != nullptr;
}

// ==================== 空间结构内部查询接口 ====================
void SimpleBVH::ForEachNode(std::function<bool(SceneNode *)> callback)
{
  // 检查是否需要更新
  if (m_NeedsRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_NeedsRebuild);
  }
  if (!m_Root)
    return;

  // 递归遍历所有Node
  TraverseDFS(m_Root, callback);
}
size_t SimpleBVH::GetNodeCount() const
{
  return m_NodeCount;
}

bool SimpleBVH::IsEmpty() const
{
  return m_NodeCount == 0;
}

int SimpleBVH::GetDepth() const
{
  if (!m_Root)
    return 0;
  return m_Root->GetHeight();
}

const char *SimpleBVH::GetTypeName() const
{
  return "SimpleBVH";
}

std::string SimpleBVH::GetStats() const
{
  BVHStats stats;
  if (m_Root) {
    CollectStatsRecursive(m_Root, stats);
    if (stats.leafCount > 0) {
      stats.avgDepth = static_cast<float>(stats.totalDepth) / stats.leafCount;
    }
  }

  std::stringstream ss;
  ss << "BVHNodes: " << m_NodeCount << " (Internal: " << stats.internalCount
     << ", Leaves: " << stats.leafCount << "), Max Depth: " << stats.maxDepth
     << ", Avg Depth: " << stats.avgDepth;
  return ss.str();
}
void SimpleBVH::DebugDraw(std::function<void(const BoundingVolumeAABB &, int depth)> drawCallback)
{
  // 检查是否需要更新
  if (m_NeedsRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_NeedsRebuild);
  }
  if (!m_Root)
    return;

  DebugDrawRecursive(m_Root, drawCallback);
}


// ==================== 私有方法：BVH树构建 ====================

BVHNode *SimpleBVH::BuildTree(std::vector<SceneNode *> &nodes, int start, int end, int depth)
{
  if (start >= end)
    return nullptr;

  const int count = end - start;

  // 创建新节点
  BVHNode *node = new BVHNode();
  node->depth = depth;
  m_NodeCount++;

  // 计算所有节点的合并包围盒
  BoundingVolumeAABB totalBounds;
  for (int i = start; i < end; ++i) {
    // 获取场景节点的AABB近似
    totalBounds.Expand(nodes[i]->GetWorldBounds().GetAABBApproximation());
  }
  node->bounds = totalBounds;

  // 如果节点数较少或达到最大深度，创建叶子节点
  if (count <= m_MinLeafSize || depth >= m_MaxDepth) {
    // 存储该范围内的所有场景节点
    for (int i = start; i < end; ++i) {
      node->sceneNodes.push_back(nodes[i]);
    }
    return node;
  }

  // 寻找最佳分割
  int bestAxis = 0;
  float bestSplitPos = 0.0f;
  if (!FindBestSplit(nodes, start, end, bestAxis, bestSplitPos)) {
    // 无法分割，创建叶子节点
    for (int i = start; i < end; ++i) {
      node->sceneNodes.push_back(nodes[i]);
    }
    return node;
  }

  // 分割节点
  int splitIndex = PartitionNodes(nodes, start, end, bestAxis, bestSplitPos);

  // 递归构建左右子树
  node->left = BuildTree(nodes, start, splitIndex, depth + 1);
  node->right = BuildTree(nodes, splitIndex, end, depth + 1);

  return node;
}

bool SimpleBVH::FindBestSplit(
    const std::vector<SceneNode *> &nodes, int start, int end, int &axis, float &splitPos) const
{
  const int count = end - start;
  if (count <= 1)
    return false;

  // 计算所有节点的中心点包围盒
  BoundingVolumeAABB centerBounds;
  for (int i = start; i < end; ++i) {
    centerBounds.Expand(nodes[i]->GetWorldBounds().GetCenter());
  }

  // 选择最长的轴
  glm::vec3 size = centerBounds.GetSize();
  axis = 0;
  if (size.y > size.x)
    axis = 1;
  if (size.z > size[axis])
    axis = 2;

  // 简单选择中心点中位数
  std::vector<float> centers;
  centers.reserve(count);
  for (int i = start; i < end; ++i) {
    centers.push_back(nodes[i]->GetWorldBounds().GetCenter()[axis]);
  }

  // nth_element从序列中找到第n小或第n大的元素，并将其移动到第n的位置处
  // centers.begin() + count / 2表示中位数
  std::nth_element(centers.begin(), centers.begin() + count / 2, centers.end());
  splitPos = centers[count / 2];

  return true;
}

int SimpleBVH::PartitionNodes(
    std::vector<SceneNode *> &nodes, int start, int end, int axis, float splitPos) const
{
  int left = start;
  int right = end - 1;

  // 使用node包围盒中心点在指定轴的分量，作为node在这个轴上的位置
  // 通过left和right迭代，与swap交换，实现了复杂度O(1)的“排序”，
  // 将node位置在splitPos左侧的点集中在vector的前left个元素
  while (left <= right) {
    while (left <= right && nodes[left]->GetWorldBounds().GetCenter()[axis] < splitPos) {
      left++;
    }
    while (left <= right && nodes[right]->GetWorldBounds().GetCenter()[axis] >= splitPos) {
      right--;
    }
    if (left < right) {
      std::swap(nodes[left], nodes[right]);
      left++;
      right--;
    }
  }

  return left;
}

void SimpleBVH::FreeNode(BVHNode *node)
{
  if (!node)
    return;

  FreeNode(node->left);
  FreeNode(node->right);

  // 释放 sceneNodes 向量（不需要释放 SceneNode 对象本身，它们由场景管理）
  node->sceneNodes.clear();

  delete node;
  m_NodeCount--;
}

// ==================== 私有方法：BVH树增量更新 ====================
void SimpleBVH::UpdateTree(bool forceFullRebuild)
{
  if (forceFullRebuild) {
    Rebuild();
    return;
  }
  if (m_DirtyNodes.empty()) {
    return;  // 没有脏节点，不需要更新
  }
  // 首先找到所有包含脏节点的BVH节点
  FindDirtyBVHNodes(m_Root);

  // 递归更新所有脏节点
  bool needsFullRebuild = UpdateNodeRecursive(m_Root);
  if (needsFullRebuild) {
    // 如果更新过程中发现需要完全重建
    Rebuild();
  }
  else {
    // 清除脏标记
    ClearDirtyFlags();
  }
}
bool SimpleBVH::UpdateNodeRecursive(BVHNode *node)
{
  if (!node)
    return false;
  if (node->IsLeaf()) {
    // 叶子节点：检查是否需要重构
    if (NeedsRefit(node)) {
      RefitNode(node);
      return false;
    }
  }
  else {
    // 使用后序遍历（左子树 → 右子树 → 当前节点）
    // 
    // 由于叶子节点变化会导致父节点包围盒失效，所以需要：
    // 1. 先更新所有受影响的叶子节点
    // 2. 然后更新直接父节点
    // 3. 最后更新更高层节点
    
    // 内部节点：先更新子节点
    bool leftNeedsRebuild = UpdateNodeRecursive(node->left);
    bool rightNeedsRebuild = UpdateNodeRecursive(node->right);

    if (leftNeedsRebuild || rightNeedsRebuild) {
      // 子节点需要重建，当前节点也需要重建
      return true;
    }

    // 更新当前节点的包围盒
    if (NeedsRefit(node)) {
      RefitNode(node);
    }
  }

  return false;
}
void SimpleBVH::RefitNode(BVHNode *node)
{
  if (node->IsLeaf()) {
    // 叶子节点：重新计算所有场景节点的合并包围盒
    BoundingVolumeAABB newBounds;
    for (SceneNode *sceneNode : node->sceneNodes) {
      BoundingVolumeAABB nodeBounds = sceneNode->GetWorldBounds().GetAABBApproximation();
      newBounds.Expand(nodeBounds);
    }
    node->bounds = newBounds;
  }
  else {
    // 内部节点：合并子节点的包围盒
    BoundingVolumeAABB newBounds;
    if (node->left) {
      newBounds.Expand(node->left->bounds);
    }
    if (node->right) {
      newBounds.Expand(node->right->bounds);
    }
    node->bounds = newBounds;
  }
}
bool SimpleBVH::NeedsRefit(BVHNode *node) const
{
  if (node->IsLeaf()) {
    // 检查叶子节点中的场景节点是否为脏节点
    for (SceneNode *sceneNode : node->sceneNodes) {
      if (m_DirtyNodes.find(sceneNode) != m_DirtyNodes.end()) {
        // 若为脏节点，则返回True
        return true;
      }
    }
  }
  else {
    // 内部节点
    bool leftDirty = node->left && m_DirtyBVHNodes.find(node->left) != m_DirtyBVHNodes.end();
    bool rightDirty = node->right && m_DirtyBVHNodes.find(node->right) != m_DirtyBVHNodes.end();

    // 如果左右子节点需要重构，或者当前节点被标记为脏，则返回true
    return leftDirty || rightDirty || m_DirtyBVHNodes.find(node) != m_DirtyBVHNodes.end();
  }

  return false;
}
void SimpleBVH::FindDirtyBVHNodes(BVHNode *node)
{
  if (!node)
    return;
  if (node->IsLeaf()) {
    // 检查叶子节点是否包含脏场景节点
    for (SceneNode *sceneNode : node->sceneNodes) {
      if (m_DirtyNodes.find(sceneNode) != m_DirtyNodes.end()) {
        m_DirtyBVHNodes.insert(node);
        break;
      }
    }
  }
  else {
    // 递归检查子节点
    FindDirtyBVHNodes(node->left);
    FindDirtyBVHNodes(node->right);

    // 如果子节点是脏的，当前节点也是脏的
    bool leftDirty = node->left && m_DirtyBVHNodes.find(node->left) != m_DirtyBVHNodes.end();
    bool rightDirty = node->right && m_DirtyBVHNodes.find(node->right) != m_DirtyBVHNodes.end();

    if (leftDirty || rightDirty) {
      m_DirtyBVHNodes.insert(node);
    }
  }
}
void SimpleBVH::MarkDirty(BVHNode *node)
{
  if (node) {
    m_DirtyBVHNodes.insert(node);
  }
}
void SimpleBVH::ClearDirtyFlags()
{
  m_DirtyNodes.clear();
  m_DirtyBVHNodes.clear();
}

// ==================== 私有方法：BVH树递归查询 ====================
void SimpleBVH::RaycastBestFirst(BVHNode *root,
                                 const Ray &ray,
                                 std::vector<SceneNode *> &results) const
{
  struct NodeWithDistance {
    BVHNode *node;
    float distance;
    bool operator<(const NodeWithDistance &other) const
    {
      return distance > other.distance;  // 最小堆，距离小的优先
    }
  };

  std::priority_queue<NodeWithDistance> queue;
  queue.push({root, 0.0f});

  while (!queue.empty()) {
    auto current = queue.top();
    queue.pop();

    // 检查射线与包围盒相交
    float t;
    if (!ray.Intersects(current.node->bounds, t)) {
      continue;
    }

    if (current.node->IsLeaf()) {
      // 处理叶子节点
      for (SceneNode *sceneNode : current.node->sceneNodes) {
        float hitDistance;
        if (ray.Intersects(sceneNode->GetWorldBounds(), t)) {
          results.push_back(sceneNode);
        }
      }
    }
    else {
      // 计算子节点距离并排序
      float leftDist = CalculateRayAABBDistance(ray, current.node->left->bounds);
      float rightDist = CalculateRayAABBDistance(ray, current.node->right->bounds);

      // 距离近的先处理
      if (leftDist < rightDist) {
        queue.push({current.node->left, leftDist});
        queue.push({current.node->right, rightDist});
      }
      else {
        queue.push({current.node->right, rightDist});
        queue.push({current.node->left, leftDist});
      }
    }
  }
}

void SimpleBVH::RaycastFirstBestFirst(BVHNode *root,
                                      const Ray &ray,
                                      SceneNode *&bestNode,
                                      float &bestDistance) const
{
  struct NodeWithDistance {
    BVHNode *node;
    float distance;
    bool operator<(const NodeWithDistance &other) const
    {
      return distance > other.distance;
    }
  };

  std::priority_queue<NodeWithDistance> queue;
  queue.push({root, 0.0f});

  while (!queue.empty()) {
    auto current = queue.top();
    queue.pop();

    // 提前终止：当前最小距离已经大于最佳距离
    if (current.distance > bestDistance) {
      continue;
    }
    float t;
    if (!ray.Intersects(current.node->bounds, t) || t > bestDistance) {
      continue;
    }

    if (current.node->IsLeaf()) {
      for (SceneNode *sceneNode : current.node->sceneNodes) {
        float nodeDistance;
        if (ray.Intersects(sceneNode->GetWorldBounds(), nodeDistance) &&
            nodeDistance < bestDistance)
        {
          bestNode = sceneNode;
          bestDistance = nodeDistance;
        }
      }
    }
    else {
      float leftDist = CalculateRayAABBDistance(ray, current.node->left->bounds);
      float rightDist = CalculateRayAABBDistance(ray, current.node->right->bounds);

      if (leftDist < rightDist) {
        queue.push({current.node->left, leftDist});
        queue.push({current.node->right, rightDist});
      }
      else {
        queue.push({current.node->right, rightDist});
        queue.push({current.node->left, leftDist});
      }
    }
  }
}
void SimpleBVH::FrustumCullBFS(BVHNode *root,
                               const Frustum &frustum,
                               std::vector<SceneNode *> &results) const
{
  std::queue<BVHNode *> queue;
  queue.push(root);

  while (!queue.empty()) {
    BVHNode *current = queue.front();
    queue.pop();

    // 使用BoundingVolumeIntersection进行相交测试
    auto intersection = frustum.TestAABB(current->bounds);

    // 当前BVHNode在视锥体之外，执行裁剪，直接退出
    if (intersection == BoundingVolumeIntersection::IntersectionType::Outside) {
      continue;
    }

    // 有SceneNode的BVHNode就是叶子节点
    if (current->IsLeaf()) {
      for (SceneNode *sceneNode : current->sceneNodes) {
        // 当前BVHNode在视锥体内，或者SceneNode不在视锥体外（相交或者在内）
        if (intersection == BoundingVolumeIntersection::IntersectionType::Inside ||
                frustum.TestBoundingVolume(sceneNode->GetWorldBounds()) !=
                BoundingVolumeIntersection::IntersectionType::Outside)
        {
          // 记录SceneNode，不被裁剪
          results.push_back(sceneNode);
        }
      }
    }
    // 非叶子节点，必然存在左右子树，记录留待下一层执行
    else {
      queue.push(current->left);
      queue.push(current->right);
    }
  }
}
void SimpleBVH::VolumeQueryBFS(BVHNode *root,
                               const BoundingVolume &volume,
                               std::vector<SceneNode *> &results) const
{
  std::queue<BVHNode *> queue;
  queue.push(root);

  while (!queue.empty()) {
    BVHNode *current = queue.front();
    queue.pop();

    BoundingVolume nodeBV(current->bounds);
    auto intersection = nodeBV.Intersects(volume);

    // 当前BVHNode在包围盒之外，直接退出
    if (intersection == BoundingVolumeIntersection::IntersectionType::Outside) {
      continue;
    }

    if (current->IsLeaf()) {
      // 有SceneNode的BVHNode就是叶子节点
      for (SceneNode *sceneNode : current->sceneNodes) {
        // 当前SceneNode不在volume外（相交或者在内）
        if (sceneNode->GetWorldBounds().Intersects(volume) !=
            BoundingVolumeIntersection::IntersectionType::Outside)
        {
          // 记录SceneNode
          results.push_back(sceneNode);
        }
      }
    }
    // 非叶子节点，必然存在左右子树，记录留待下一层执行
    else {
      queue.push(current->left);
      queue.push(current->right);
    }
  }
}

bool SimpleBVH::TraverseDFS(BVHNode *node,
                                     std::function<bool(SceneNode *)> callback) const
{
  if (!node)
    return true;

  if (node->IsLeaf()) {
    for (SceneNode *sceneNode : node->sceneNodes) {
      if (!callback(sceneNode)) {
        return false;
      }
    }
  }
  else {
    if (!TraverseDFS(node->left, callback))
      return false;
    if (!TraverseDFS(node->right, callback))
      return false;
  }

  return true;
}
void SimpleBVH::CollectStatsRecursive(BVHNode *node, BVHStats &stats) const
{
  if (!node)
    return;

  if (node->IsLeaf()) {
    stats.leafCount++;
    stats.totalDepth += node->depth;
  }
  else {
    stats.internalCount++;
    CollectStatsRecursive(node->left, stats);
    CollectStatsRecursive(node->right, stats);
  }

  stats.maxDepth = std::max(stats.maxDepth, node->depth);
}

void SimpleBVH::DebugDrawRecursive(
    BVHNode *node, std::function<void(const BoundingVolumeAABB &, int depth)> drawCallback) const
{
  if (!node)
    return;

  drawCallback(node->bounds, node->depth);

  if (!node->IsLeaf()) {
    DebugDrawRecursive(node->left, drawCallback);
    DebugDrawRecursive(node->right, drawCallback);
  }
}

float SimpleBVH::CalculateRayAABBDistance(const Ray &ray, const BoundingVolumeAABB &aabb) const
{
  float t;
  if (ray.Intersects(aabb, t)) {
    return t;
  }
  return std::numeric_limits<float>::max();
}
float SimpleBVH::CalculatePointAABBDistanceSq(const glm::vec3 &point,
                                              const BoundingVolumeAABB &aabb) const
{
  return aabb.DistanceToPointSq(point);
}
}  // namespace mite