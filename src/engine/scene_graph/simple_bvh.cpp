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
// ==================== 空间划分生命周期管理 ====================
void SimpleBVH::Insert(SceneNode *node)
{
  if (!node)
    return;

  // 检查是否已存在
  if (std::find(m_AllNodes.begin(), m_AllNodes.end(), node) != m_AllNodes.end()) {
    return;
  }

  m_AllNodes.push_back(node);
  m_NeedsRebuild = true;
}
void SimpleBVH::Remove(SceneNode *node)
{
  if (!node)
    return;

  auto it = std::find(m_AllNodes.begin(), m_AllNodes.end(), node);
  if (it != m_AllNodes.end()) {
    m_AllNodes.erase(it);
    m_NeedsRebuild = true;
  }
}
void SimpleBVH::Update(SceneNode *node)
{
  if (!node)
    return;

  // 更新场景节点在空间结构中的位置
  // 简单实现：直接重建
  // 优化实现：可增量更新，这里为简化先重建
  m_NeedsRebuild = true;
}
void SimpleBVH::Clear()
{
  if (m_Root) {
    FreeNode(m_Root);
    m_Root = nullptr;
  }
  m_AllNodes.clear();
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
    return;
  }

  // 释放旧树
  if (m_Root) {
    FreeNode(m_Root);
  }

  // 构建新树
  std::vector<SceneNode *> nodesToBuild = m_AllNodes;
  m_Root = BuildTree(nodesToBuild, 0, static_cast<int>(nodesToBuild.size()), 0);
  m_NeedsRebuild = false;

  LOG_INFO("BVH rebuilt with {} nodes, depth: {}", m_NodeCount, GetDepth());
}

// ==================== 空间结构外部查询接口 ====================
bool SimpleBVH::Raycast(const Ray &ray, std::vector<SceneNode *> &results)
{
  if (m_NeedsRebuild)
    Rebuild();
  if (!m_Root)
    return false;

  results.clear();
  RaycastRecursive(m_Root, ray, results);
  return !results.empty();
}
bool SimpleBVH::RaycastFirst(const Ray &ray, SceneNode *&result, float &distance)
{
  if (m_NeedsRebuild)
    Rebuild();
  if (!m_Root)
    return false;

  result = nullptr;
  distance = std::numeric_limits<float>::max();
  RaycastFirstRecursive(m_Root, ray, result, distance);
  return result != nullptr;
}
int SimpleBVH::FrustumCull(const Frustum &frustum, std::vector<SceneNode *> &results)
{
  if (m_NeedsRebuild)
    Rebuild();
  if (!m_Root)
    return 0;

  results.clear();
  FrustumCullRecursive(m_Root, frustum, results);
  return static_cast<int>(results.size());
}
size_t SimpleBVH::VolumeQuery(const BoundingVolume &volume, std::vector<SceneNode *> &results)
{
  if (m_NeedsRebuild)
    Rebuild();
  if (!m_Root)
    return 0;

  results.clear();
  VolumeQueryRecursive(m_Root, volume, results);
  return results.size();
}
size_t SimpleBVH::PointQuery(const glm::vec3 &point, std::vector<SceneNode *> &results)
{
  if (m_NeedsRebuild)
    Rebuild();
  if (!m_Root)
    return 0;

  results.clear();
  BoundingVolume pointAABB(BoundingVolumeAABB(point, point));  // 创建零大小的AABB
  VolumeQueryRecursive(m_Root, pointAABB, results);
  return results.size();
}
bool SimpleBVH::NearestNeighbor(const glm::vec3 &point, SceneNode *&result, float maxDistance)
{
  if (m_NeedsRebuild)
    Rebuild();
  if (!m_Root)
    return false;

  result = nullptr;
  float bestDistance = maxDistance;
  float bestDistanceSq = bestDistance * bestDistance;

  // 使用优先队列进行最近邻搜索
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
  if (m_NeedsRebuild)
    Rebuild();
  if (!m_Root)
    return;

  // 递归遍历所有Node
  ForEachNodeRecursive(m_Root, callback);
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
  ss << "Nodes: " << m_NodeCount << " (Internal: " << stats.internalCount
     << ", Leaves: " << stats.leafCount << "), Max Depth: " << stats.maxDepth
     << ", Avg Depth: " << stats.avgDepth;
  return ss.str();
}
void SimpleBVH::DebugDraw(std::function<void(const BoundingVolumeAABB &, int depth)> drawCallback)
{
  if (m_NeedsRebuild)
    Rebuild();
  if (!m_Root)
    return;

  DebugDrawRecursive(m_Root, drawCallback);
}


// ==================== 私有方法 ====================

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

void SimpleBVH::RaycastRecursive(BVHNode *node,
                                 const Ray &ray,
                                 std::vector<SceneNode *> &results) const
{
  if (!node)
    return;

  float t;
  if (!ray.Intersects(node->bounds, t)) {
    return;
  }

  if (node->IsLeaf()) {
    for (SceneNode *sceneNode : node->sceneNodes) {
      results.push_back(sceneNode);
    }
  }
  else {
    RaycastRecursive(node->left, ray, results);
    RaycastRecursive(node->right, ray, results);
  }
}

void SimpleBVH::RaycastFirstRecursive(BVHNode *node,
                                      const Ray &ray,
                                      SceneNode *&bestNode,
                                      float &bestDistance) const
{
  if (!node)
    return;

  float t;
  if (!ray.Intersects(node->bounds, t) || t > bestDistance) {
    return;
  }

  if (node->IsLeaf()) {
    for (SceneNode *sceneNode : node->sceneNodes) {
      float nodeDistance;
      if (sceneNode->GetWorldBounds().IntersectsRay(ray, nodeDistance) &&
          nodeDistance < bestDistance)
      {
        bestNode = sceneNode;
        bestDistance = nodeDistance;
      }
    }
  }
  else {
    // 先检测距离更近的子节点
    float leftDist, rightDist;
    bool leftHit = ray.Intersects(node->left->bounds, leftDist);
    bool rightHit = ray.Intersects(node->right->bounds, rightDist);

    if (leftHit && rightHit) {
      if (leftDist < rightDist) {
        RaycastFirstRecursive(node->left, ray, bestNode, bestDistance);
        RaycastFirstRecursive(node->right, ray, bestNode, bestDistance);
      }
      else {
        RaycastFirstRecursive(node->right, ray, bestNode, bestDistance);
        RaycastFirstRecursive(node->left, ray, bestNode, bestDistance);
      }
    }
    else if (leftHit) {
      RaycastFirstRecursive(node->left, ray, bestNode, bestDistance);
    }
    else if (rightHit) {
      RaycastFirstRecursive(node->right, ray, bestNode, bestDistance);
    }
  }
}

void SimpleBVH::FrustumCullRecursive(BVHNode *node,
                                     const Frustum &frustum,
                                     std::vector<SceneNode *> &results) const
{
  if (!node)
    return;

  // 当前BVHNode在视锥体之外，执行裁剪，直接退出
  auto intersection = frustum.TestBoundingVolume(node->bounds);
  if (intersection == BoundingVolumeIntersection::IntersectionType::Outside) {
    return;
  }

  // 有SceneNode的BVHNode就是叶子节点
  if (node->IsLeaf()) {
    for (SceneNode *sceneNode : node->sceneNodes) {
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
  // 非叶子节点，必然存在左右子树，执行递归
  else {
    FrustumCullRecursive(node->left, frustum, results);
    FrustumCullRecursive(node->right, frustum, results);
  }
}

void SimpleBVH::VolumeQueryRecursive(BVHNode *node,
                                     const BoundingVolume &volume,
                                     std::vector<SceneNode *> &results) const
{
  if (!node)
    return;

  // 当前BVHNode在包围盒之外，直接退出
  if (volume.Intersects(node->bounds) == BoundingVolumeIntersection::IntersectionType::Outside) {
    return;
  }
  // 有SceneNode的BVHNode就是叶子节点
  if (node->IsLeaf()) {
    for (SceneNode *sceneNode : node->sceneNodes) {
      // 当前SceneNode不在volume外（相交或者在内）
      if (volume.Intersects(sceneNode->GetWorldBounds()) !=
          BoundingVolumeIntersection::IntersectionType::Outside)
      {
        // 记录SceneNode
        results.push_back(sceneNode);
      }
    }
  }
  // 非叶子节点，必然存在左右子树，执行递归
  else {
    VolumeQueryRecursive(node->left, volume, results);
    VolumeQueryRecursive(node->right, volume, results);
  }
}

bool SimpleBVH::ForEachNodeRecursive(BVHNode *node,
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
    if (!ForEachNodeRecursive(node->left, callback))
      return false;
    if (!ForEachNodeRecursive(node->right, callback))
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
}  // namespace mite