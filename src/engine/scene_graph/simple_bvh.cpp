#include "simple_bvh.h"

namespace mite {
size_t BVHNode::GetHeight() const {
  if (IsLeaf()) return 0;
  size_t leftHeight = left ? left->GetHeight() : 0;
  size_t rightHeight = right ? right->GetHeight() : 0;
  return 1 + std::max(leftHeight, rightHeight);
}

SimpleBVH::SimpleBVH(size_t maxDepth, size_t minLeafSize)
    : m_MaxDepth(maxDepth), m_MinLeafSize(minLeafSize) {}

SimpleBVH::~SimpleBVH() { Clear(); }
bool SimpleBVH::Contains(std::shared_ptr<SceneNode> node) const {
  return m_AllNodes.find(node) != m_AllNodes.end();
}
// ==================== 空间划分生命周期管理 ====================
void SimpleBVH::Insert(std::shared_ptr<SceneNode> node) {
  if (!node) return;
  // 检查是否已存在
  if (m_AllNodes.find(node) != m_AllNodes.end()) {
    return;
  }
  // 添加到总节点集合
  m_AllNodes.insert(node);

  // 如果是第一个节点，或者树为空，直接重建
  if (!m_Root || m_AllNodes.size() == 1) {
    m_ForceRebuild = true;  // 标记需要重建
    return;
  }

  // 如果有现有的BVH树，尝试增量插入
  if (m_Root && !m_NodeToBVHNodeMap.empty()) {
    // 尝试增量插入到现有树中
    if (TryInsertIntoExistingTree(node)) {
      // 插入成功，标记为脏以便更新包围盒
      m_DirtyNodes.insert(node);
      return;
    }
  }

  // 增量插入失败或没有现有树，添加到待插入集合
  // 这些节点会在下次 UpdateTree 时批量处理
  m_NewNodes.insert(node);

  LOG_DEBUG("Node added to pending insert list, total pending: {}",
            m_NewNodes.size());
}
void SimpleBVH::Remove(std::shared_ptr<SceneNode> node) {
  if (!node) return;
  // 从总节点集合中移除
  auto allIt = m_AllNodes.find(node);
  if (allIt == m_AllNodes.end()) {
    return;
  }
  m_AllNodes.erase(allIt);

  // 从待插入集合中移除（如果存在）
  m_NewNodes.erase(node);

  // 从脏节点集合中移除
  m_DirtyNodes.erase(node);

  // 如果节点已经在BVH树中，需要从树中移除
  auto mapIt = m_NodeToBVHNodeMap.find(node);
  if (mapIt != m_NodeToBVHNodeMap.end()) {
    BVHNode *leafNode = mapIt->second;

    // 从叶子节点的场景节点列表中移除
    auto &sceneNodes = leafNode->sceneNodes;
    auto vecIt = std::find(sceneNodes.begin(), sceneNodes.end(), node);
    if (vecIt != sceneNodes.end()) {
      sceneNodes.erase(vecIt);

      // 标记叶子节点为脏（包围盒可能需要更新）
      MarkDirty(leafNode);

      // 如果叶子节点变空了，标记为待清理
      if (sceneNodes.empty()) {
        m_EmptyLeaves.insert(leafNode);
      }
    }

    // 从映射中移除
    m_NodeToBVHNodeMap.erase(mapIt);
  }

  LOG_DEBUG("Node removed from BVH");
}
void SimpleBVH::Update(std::shared_ptr<SceneNode> node) {
  if (!node) return;

  // 如果节点不在BVH中，忽略更新
  if (m_AllNodes.find(node) == m_AllNodes.end()) {
    return;
  }
  // 检查节点是否已经在BVH树中（通过映射）
  if (m_NodeToBVHNodeMap.find(node) != m_NodeToBVHNodeMap.end()) {
    // 节点在树中，标记为脏以便增量更新
    m_DirtyNodes.insert(node);
  } else {
    // 节点不在树中（可能是新增节点），添加到待插入集合
    m_NewNodes.insert(node);
  }
}
void SimpleBVH::Clear() {
  if (m_Root) {
    FreeNode(m_Root);
    m_Root = nullptr;
  }
  m_AllNodes.clear();
  m_DirtyNodes.clear();
  m_DirtyBVHNodes.clear();
  m_NodeToBVHNodeMap.clear();  // 清空映射
  m_NewNodes.clear();          // 清空待插入节点
  m_EmptyLeaves.clear();       // 清空空叶子节点
  m_NodeCount = 0;
  m_ForceRebuild = false;
}
void SimpleBVH::Rebuild() {
  if (m_AllNodes.empty()) {
    if (m_Root) {
      FreeNode(m_Root);
      m_Root = nullptr;
    }
    m_NodeCount = 0;
    m_ForceRebuild = false;
    ClearDirtyFlags();
    m_NodeToBVHNodeMap.clear();
    m_NewNodes.clear();
    m_EmptyLeaves.clear();
    return;
  }
  // 释放旧树
  if (m_Root) {
    FreeNode(m_Root);
  }
  // 构建新树
  std::vector<std::shared_ptr<SceneNode>> nodesToBuild(m_AllNodes.begin(),
                                                       m_AllNodes.end());
  m_Root = BuildTree(nodesToBuild, 0, nodesToBuild.size(), 0);

  // 重建映射
  m_NodeToBVHNodeMap.clear();
  BuildNodeMapping(m_Root);

  // 清除所有状态
  m_ForceRebuild = false;
  m_NewNodes.clear();
  m_EmptyLeaves.clear();
  ClearDirtyFlags();
  LOG_DEBUG("BVH rebuilt with {} nodes, depth: {}", m_NodeCount, GetDepth());
}

// ==================== 空间结构外部查询接口 ====================
bool SimpleBVH::Raycast(const Ray &ray,
                        std::vector<std::shared_ptr<SceneNode>> &results) {
  // 检查是否需要更新
  if (m_ForceRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_ForceRebuild);
  }
  if (!m_Root) return false;

  results.clear();

  // 使用最佳优先遍历进行射线查询
  RaycastBestFirst(m_Root, ray, results);
  return !results.empty();
}
bool SimpleBVH::RaycastFirst(const Ray &ray, std::shared_ptr<SceneNode> &result,
                             float &distance) {
  // 检查是否需要更新
  if (m_ForceRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_ForceRebuild);
  }
  if (!m_Root) return false;

  result = nullptr;
  distance = std::numeric_limits<float>::max();

  // 使用最佳优先遍历寻找第一个交点
  RaycastFirstBestFirst(m_Root, ray, result, distance);
  return result != nullptr;
}
size_t SimpleBVH::FrustumCull(
    const Frustum &frustum, const uint32_t visibleMask,
    std::vector<std::shared_ptr<SceneNode>> &results) {
  // 检查是否需要更新
  if (m_ForceRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_ForceRebuild);
  }
  if (!m_Root) return 0;

  results.clear();

  // 使用广度优先遍历进行视锥体裁剪
  FrustumCullBFS(m_Root, frustum, visibleMask, results);
  return results.size();
}
size_t SimpleBVH::VolumeQuery(
    const BoundingVolume &volume,
    std::vector<std::shared_ptr<SceneNode>> &results) {
  // 检查是否需要更新
  if (m_ForceRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_ForceRebuild);
  }
  if (!m_Root) return 0;

  results.clear();

  // 使用广度优先遍历进行体积查询
  VolumeQueryBFS(m_Root, volume, results);
  return results.size();
}
size_t SimpleBVH::PointQuery(const glm::vec3 &point,
                             std::vector<std::shared_ptr<SceneNode>> &results) {
  // 检查是否需要更新
  if (m_ForceRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_ForceRebuild);
  }
  if (!m_Root) return 0;

  results.clear();
  BoundingVolume pointAABB = BoundingVolume::CreateFromPoints(
      BoundingVolumeType::AABB, {point});  // 创建零大小的AABB
  return VolumeQuery(pointAABB, results);
}
bool SimpleBVH::NearestNeighbor(const glm::vec3 &point,
                                std::shared_ptr<SceneNode> &result,
                                float maxDistance) {
  // 检查是否需要更新
  if (m_ForceRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_ForceRebuild);
  }
  if (!m_Root) return false;

  result = nullptr;
  float bestDistance = maxDistance;
  float bestDistanceSq = bestDistance * bestDistance;

  // 使用优先队列进行最近邻搜索（最佳优先变种）
  struct QueueElement {
    BVHNode *node;
    float distanceSq;

    bool operator<(const QueueElement &other) const {
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
      for (std::shared_ptr<SceneNode> sceneNode : current.node->sceneNodes) {
        float distSq =
            glm::distance2(point, sceneNode->GetWorldBounds().GetCenter());
        if (distSq < bestDistanceSq) {
          bestDistanceSq = distSq;
          result = sceneNode;
        }
      }
    } else {
      if (current.node->left) {
        float leftDistSq = current.node->left->bounds.DistanceToPointSq(point);
        if (leftDistSq <= bestDistanceSq) {
          queue.push({current.node->left, leftDistSq});
        }
      }

      if (current.node->right) {
        float rightDistSq =
            current.node->right->bounds.DistanceToPointSq(point);
        if (rightDistSq <= bestDistanceSq) {
          queue.push({current.node->right, rightDistSq});
        }
      }
    }
  }

  return result != nullptr;
}

// ==================== 空间结构内部查询接口 ====================
void SimpleBVH::ForEachNode(
    std::function<bool(std::shared_ptr<SceneNode>)> callback) {
  // 检查是否需要更新
  if (m_ForceRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_ForceRebuild);
  }
  if (!m_Root) return;

  // 递归遍历所有Node
  TraverseDFS(m_Root, callback);
}
size_t SimpleBVH::GetNodeCount() const { return m_NodeCount; }

bool SimpleBVH::IsEmpty() const { return m_NodeCount == 0; }

size_t SimpleBVH::GetDepth() const {
  if (!m_Root) return 0;
  return m_Root->GetHeight();
}

const char *SimpleBVH::GetTypeName() const { return "SimpleBVH"; }

std::string SimpleBVH::GetStats() const {
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
void SimpleBVH::DebugDraw(
    std::function<void(const BoundingVolumeAABB &, size_t depth)>
        drawCallback) {
  // 检查是否需要更新
  if (m_ForceRebuild || !m_DirtyNodes.empty()) {
    UpdateTree(m_ForceRebuild);
  }
  if (!m_Root) return;

  DebugDrawRecursive(m_Root, drawCallback);
}

// ==================== 私有方法：BVH树构建 ====================

BVHNode *SimpleBVH::BuildTree(std::vector<std::shared_ptr<SceneNode>> &nodes,
                              size_t start, size_t end, size_t depth) {
  // 边界检查
  if (start >= end) return nullptr;
  const size_t count = end - start;

  // 创建新节点
  BVHNode *node = new BVHNode();
  node->depth = depth;
  m_NodeCount++;

  // 计算所有节点的合并包围盒
  BoundingVolumeAABB totalBounds;
  for (size_t i = start; i < end; ++i) {
    // 获取场景节点的AABB近似
    totalBounds.Expand(nodes[i]->GetWorldBounds().GetAABBApproximation());
  }
  node->bounds = totalBounds;

  // 如果节点数较少或达到最大深度，创建叶子节点
  if (count <= m_MinLeafSize || depth >= m_MaxDepth) {
    // 存储该范围内的所有场景节点
    for (size_t i = start; i < end; ++i) {
      node->sceneNodes.push_back(nodes[i]);
    }
    return node;
  }

  // 寻找最佳分割
  int bestAxis = 0;
  float bestSplitPos = 0.0f;
  if (!FindBestSplit(nodes, start, end, bestAxis, bestSplitPos)) {
    // 无法分割，创建叶子节点
    for (size_t i = start; i < end; ++i) {
      node->sceneNodes.push_back(nodes[i]);
    }
    return node;
  }

  // 寻找分割节点的位置
  size_t splitIndex = PartitionNodes(nodes, start, end, bestAxis, bestSplitPos);

  // 确保分割后的两个区间都非空
  if (splitIndex == start || splitIndex == end) {
    // 分割失败，回退到叶子节点
    for (size_t i = start; i < end; ++i) {
      node->sceneNodes.push_back(nodes[i]);
    }
    return node;
  }

  // 递归构建左右子树
  node->left = BuildTree(nodes, start, splitIndex, depth + 1);
  node->right = BuildTree(nodes, splitIndex, end, depth + 1);

  return node;
}

bool SimpleBVH::FindBestSplit(
    const std::vector<std::shared_ptr<SceneNode>> &nodes, size_t start,
    size_t end, int &axis, float &splitPos) const {
  const size_t count = end - start;
  if (count <= 1) return false;

  // 计算所有节点的中心点包围盒
  BoundingVolumeAABB centerBounds;
  for (size_t i = start; i < end; ++i) {
    centerBounds.Expand(nodes[i]->GetWorldBounds().GetCenter());
  }

  // 选择最长的轴
  glm::vec3 size = centerBounds.GetSize();
  axis = 0;
  if (size.y > size.x) axis = 1;
  if (size.z > size[axis]) axis = 2;

  // 如果所有中心点在同一位置，无法分割，直接返回
  if (size[axis] < 1e-6f) {
    return false;
  }

  // 简单选择中心点中位数
  std::vector<float> centers;
  centers.reserve(count);
  for (size_t i = start; i < end; ++i) {
    centers.push_back(nodes[i]->GetWorldBounds().GetCenter()[axis]);
  }

  // 确保有足够的元素进行分割
  if (centers.size() < 2) {
    return false;
  }

  // nth_element从序列中找到第n小或第n大的元素，并将其移动到第n的位置处
  // centers.begin() + count / 2表示中位数
  std::nth_element(centers.begin(), centers.begin() + count / 2, centers.end());
  splitPos = centers[count / 2];

  // 确保分割位置在有效范围内
  float minVal = *std::min_element(centers.begin(), centers.end());
  float maxVal = *std::max_element(centers.begin(), centers.end());

  // 如果分割位置太接近边界，无法有效分割
  if (splitPos <= minVal + 1e-6f || splitPos >= maxVal - 1e-6f) {
    return false;
  }

  return true;
}

size_t SimpleBVH::PartitionNodes(std::vector<std::shared_ptr<SceneNode>> &nodes,
                                 size_t start, size_t end, int &axis,
                                 float splitPos) const {
  // 添加边界检查 - 防止整数下溢
  if (start >= end) {
    return start;  // 这种情况不应该发生，但作为安全措施
  }

  // 添加断言方便调试
  assert(end <= nodes.size());

  size_t left = start;
  size_t right = end - 1;

  // 使用node包围盒中心点在指定轴的分量，作为node在这个轴上的位置
  // 通过left和right迭代，与swap交换，实现了复杂度O(1)的“排序”，
  // 将node位置在splitPos左侧的点集中在vector的前left个元素
  while (left <= right) {
    while (left <= right &&
           nodes[left]->GetWorldBounds().GetCenter()[axis] < splitPos) {
      left++;
    }
    while (left <= right &&
           nodes[right]->GetWorldBounds().GetCenter()[axis] >= splitPos) {
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

void SimpleBVH::FreeNode(BVHNode *node) {
  if (!node) return;

  // 如果是叶子节点，从映射中移除所有关联的场景节点
  if (node->IsLeaf()) {
    for (auto &sceneNode : node->sceneNodes) {
      m_NodeToBVHNodeMap.erase(sceneNode);
    }
  }
  FreeNode(node->left);
  FreeNode(node->right);

  // 释放 sceneNodes 向量（不需要释放 SceneNode 对象本身，它们由场景管理）
  node->sceneNodes.clear();
  delete node;
  m_NodeCount--;
}

// ==================== 私有方法：BVH树增量更新 ====================
void SimpleBVH::UpdateTree(bool forceFullRebuild) {
  // 检查是否需要完全重建
  if (forceFullRebuild || m_ForceRebuild || ShouldRebuildCompletely()) {
    Rebuild();
    return;
  }

  // 第一步：处理结构变化（新增/删除节点）
  ProcessStructuralChanges();

  // 第二步：如果处理结构变化后标记了需要重建，则重建
  if (m_ForceRebuild) {
    Rebuild();
    return;
  }

  // 第三步：处理位置变化（增量更新）
  if (!m_DirtyNodes.empty()) {
    // 原有的增量更新逻辑
    FindDirtyBVHNodes(m_Root);
    bool needsFullRebuild = UpdateNodeRecursive(m_Root);
    if (needsFullRebuild) {
      // 增量更新过程中发现问题，需要完全重建
      m_ForceRebuild = true;
      Rebuild();
      return;
    }
  }

  // 第四步：清理状态
  ClearDirtyFlags();
}
bool SimpleBVH::UpdateNodeRecursive(BVHNode *node) {
  if (!node) return false;
  if (node->IsLeaf()) {
    // 叶子节点：检查是否需要重构
    if (NeedsRefit(node)) {
      RefitNode(node);
      return false;
    }
  } else {
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
void SimpleBVH::RefitNode(BVHNode *node) {
  if (node->IsLeaf()) {
    // 叶子节点：重新计算所有场景节点的合并包围盒
    BoundingVolumeAABB newBounds;
    for (std::shared_ptr<SceneNode> sceneNode : node->sceneNodes) {
      BoundingVolumeAABB nodeBounds =
          sceneNode->GetWorldBounds().GetAABBApproximation();
      newBounds.Expand(nodeBounds);
    }
    node->bounds = newBounds;
  } else {
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
bool SimpleBVH::NeedsRefit(BVHNode *node) const {
  if (node->IsLeaf()) {
    // 检查叶子节点中的场景节点是否为脏节点
    for (std::shared_ptr<SceneNode> sceneNode : node->sceneNodes) {
      if (m_DirtyNodes.find(sceneNode) != m_DirtyNodes.end()) {
        // 若为脏节点，则返回True
        return true;
      }
    }
  } else {
    // 内部节点
    bool leftDirty =
        node->left && m_DirtyBVHNodes.find(node->left) != m_DirtyBVHNodes.end();
    bool rightDirty = node->right && m_DirtyBVHNodes.find(node->right) !=
                                         m_DirtyBVHNodes.end();

    // 如果左右子节点需要重构，或者当前节点被标记为脏，则返回true
    return leftDirty || rightDirty ||
           m_DirtyBVHNodes.find(node) != m_DirtyBVHNodes.end();
  }

  return false;
}
void SimpleBVH::FindDirtyBVHNodes(BVHNode *node) {
  if (!node) return;
  if (node->IsLeaf()) {
    // 检查叶子节点是否包含脏场景节点
    for (std::shared_ptr<SceneNode> sceneNode : node->sceneNodes) {
      if (m_DirtyNodes.find(sceneNode) != m_DirtyNodes.end()) {
        m_DirtyBVHNodes.insert(node);
        break;
      }
    }
  } else {
    // 递归检查子节点
    FindDirtyBVHNodes(node->left);
    FindDirtyBVHNodes(node->right);

    // 如果子节点是脏的，当前节点也是脏的
    bool leftDirty =
        node->left && m_DirtyBVHNodes.find(node->left) != m_DirtyBVHNodes.end();
    bool rightDirty = node->right && m_DirtyBVHNodes.find(node->right) !=
                                         m_DirtyBVHNodes.end();

    if (leftDirty || rightDirty) {
      m_DirtyBVHNodes.insert(node);
    }
  }
}
void SimpleBVH::MarkDirty(BVHNode *node) {
  if (node) {
    m_DirtyBVHNodes.insert(node);
  }
}
void SimpleBVH::ClearDirtyFlags() {
  m_DirtyNodes.clear();
  m_DirtyBVHNodes.clear();
}

// ==================== 私有方法：BVH树递归查询 ====================
void SimpleBVH::RaycastBestFirst(
    BVHNode *root, const Ray &ray,
    std::vector<std::shared_ptr<SceneNode>> &results) const {
  struct NodeWithDistance {
    BVHNode *node;
    float distance;
    bool operator<(const NodeWithDistance &other) const {
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
      for (std::shared_ptr<SceneNode> sceneNode : current.node->sceneNodes) {
        float hitDistance;
        if (ray.Intersects(sceneNode->GetWorldBounds(), hitDistance)) {
          results.push_back(sceneNode);
        }
      }
    } else {
      // 计算子节点距离并排序
      float leftDist =
          CalculateRayAABBDistance(ray, current.node->left->bounds);
      float rightDist =
          CalculateRayAABBDistance(ray, current.node->right->bounds);

      // 距离近的先处理
      if (leftDist < rightDist) {
        queue.push({current.node->left, leftDist});
        queue.push({current.node->right, rightDist});
      } else {
        queue.push({current.node->right, rightDist});
        queue.push({current.node->left, leftDist});
      }
    }
  }
}

void SimpleBVH::RaycastFirstBestFirst(BVHNode *root, const Ray &ray,
                                      std::shared_ptr<SceneNode> &bestNode,
                                      float &bestDistance) const {
  struct NodeWithDistance {
    BVHNode *node;
    float distance;
    bool operator<(const NodeWithDistance &other) const {
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
      for (std::shared_ptr<SceneNode> sceneNode : current.node->sceneNodes) {
        float nodeDistance;
        if (ray.Intersects(sceneNode->GetWorldBounds(), nodeDistance) &&
            nodeDistance < bestDistance) {
          bestNode = sceneNode;
          bestDistance = nodeDistance;
        }
      }
    } else {
      float leftDist =
          CalculateRayAABBDistance(ray, current.node->left->bounds);
      float rightDist =
          CalculateRayAABBDistance(ray, current.node->right->bounds);

      if (leftDist < rightDist) {
        queue.push({current.node->left, leftDist});
        queue.push({current.node->right, rightDist});
      } else {
        queue.push({current.node->right, rightDist});
        queue.push({current.node->left, leftDist});
      }
    }
  }
}
void SimpleBVH::FrustumCullBFS(
    BVHNode *root, const Frustum &frustum, const uint32_t visibleMask,
    std::vector<std::shared_ptr<SceneNode>> &results) const {
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
      for (std::shared_ptr<SceneNode> sceneNode : current->sceneNodes) {
        // 当前BVHNode不在视锥体外（相交或者在内），或者SceneNode不在视锥体外（相交或者在内）
        if (intersection !=
                BoundingVolumeIntersection::IntersectionType::Outside ||
            frustum.TestBoundingVolume(sceneNode->GetWorldBounds()) !=
                BoundingVolumeIntersection::IntersectionType::Outside) {
          // 确保可见，且可见性掩码匹配
          if (sceneNode->IsWorldVisible() &&
              (sceneNode->GetVisibilityMask() & visibleMask) != 0)
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
void SimpleBVH::VolumeQueryBFS(
    BVHNode *root, const BoundingVolume &volume,
    std::vector<std::shared_ptr<SceneNode>> &results) const {
  std::queue<BVHNode *> queue;
  queue.push(root);

  while (!queue.empty()) {
    BVHNode *current = queue.front();
    queue.pop();

    BoundingVolume nodeBV = BoundingVolume::CreateFromAABB(current->bounds);
    auto intersection = nodeBV.Intersects(volume);

    // 当前BVHNode在包围盒之外，直接退出
    if (intersection == BoundingVolumeIntersection::IntersectionType::Outside) {
      continue;
    }

    if (current->IsLeaf()) {
      // 有SceneNode的BVHNode就是叶子节点
      for (std::shared_ptr<SceneNode> sceneNode : current->sceneNodes) {
        // 当前SceneNode不在volume外（相交或者在内）
        if (sceneNode->GetWorldBounds().Intersects(volume) !=
            BoundingVolumeIntersection::IntersectionType::Outside) {
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

bool SimpleBVH::TraverseDFS(
    BVHNode *node,
    std::function<bool(std::shared_ptr<SceneNode>)> callback) const {
  if (!node) return true;

  if (node->IsLeaf()) {
    for (std::shared_ptr<SceneNode> sceneNode : node->sceneNodes) {
      if (!callback(sceneNode)) {
        return false;
      }
    }
  } else {
    if (!TraverseDFS(node->left, callback)) return false;
    if (!TraverseDFS(node->right, callback)) return false;
  }

  return true;
}
void SimpleBVH::CollectStatsRecursive(BVHNode *node, BVHStats &stats) const {
  if (!node) return;

  if (node->IsLeaf()) {
    stats.leafCount++;
    stats.totalDepth += node->depth;
  } else {
    stats.internalCount++;
    CollectStatsRecursive(node->left, stats);
    CollectStatsRecursive(node->right, stats);
  }

  stats.maxDepth = std::max(stats.maxDepth, node->depth);
}

void SimpleBVH::DebugDrawRecursive(
    BVHNode *node,
    std::function<void(const BoundingVolumeAABB &, size_t depth)> drawCallback)
    const {
  if (!node) return;

  drawCallback(node->bounds, node->depth);

  if (!node->IsLeaf()) {
    DebugDrawRecursive(node->left, drawCallback);
    DebugDrawRecursive(node->right, drawCallback);
  }
}

float SimpleBVH::CalculateRayAABBDistance(
    const Ray &ray, const BoundingVolumeAABB &aabb) const {
  float t;
  if (ray.Intersects(aabb, t)) {
    return t;
  }
  return std::numeric_limits<float>::max();
}
float SimpleBVH::CalculatePointAABBDistanceSq(
    const glm::vec3 &point, const BoundingVolumeAABB &aabb) const {
  return aabb.DistanceToPointSq(point);
}

bool SimpleBVH::ShouldRebuildCompletely() const {
  // 启发式判断是否需要完全重建

  // 0. 强制重建标志
  if (m_ForceRebuild) {
    return true;
  }

  // 1. 如果树为空，但有待插入节点或者有效节点，需要重建
  if (!m_Root && (!m_NewNodes.empty() || !m_AllNodes.empty())) {
    return true;
  }

  // 2. 如果待插入节点太多（超过总节点的30%），重建更高效
  if (!m_NewNodes.empty() && m_NewNodes.size() > m_AllNodes.size() * 0.3f) {
    LOG_DEBUG("Too many new nodes ({} > 30%), triggering rebuild",
              m_NewNodes.size());
    return true;
  }

  // 3. 如果空叶子节点太多（超过总节点的20%），重建以优化结构
  if (!m_EmptyLeaves.empty() && m_EmptyLeaves.size() > m_NodeCount * 0.2f) {
    LOG_DEBUG("Too many empty leaves ({} > 20%), triggering rebuild",
              m_EmptyLeaves.size());
    return true;
  }

  // 4. 如果脏节点太多（超过总节点的40%），重建可能更高效
  if (!m_DirtyNodes.empty() && m_DirtyNodes.size() > m_AllNodes.size() * 0.4f) {
    LOG_DEBUG("Too many dirty nodes ({} > 40%), triggering rebuild",
              m_DirtyNodes.size());
    return true;
  }

  return false;
}

void SimpleBVH::ProcessStructuralChanges() {
  // 处理空叶子节点
  if (!m_EmptyLeaves.empty()) {
    RemoveEmptyLeaves();
  }

  // 处理新增节点
  if (!m_NewNodes.empty()) {
    // 尝试批量插入新增节点
    if (!BatchInsertNodes(m_NewNodes)) {
      // 批量插入失败，标记需要重建
      m_ForceRebuild = true;
      return;
    }
    m_NewNodes.clear();
  }
}

bool SimpleBVH::BatchInsertNodes(
    const std::unordered_set<std::shared_ptr<SceneNode>> &nodes) {
  if (!m_Root || nodes.empty()) {
    return false;
  }

  LOG_DEBUG("Batch inserting {} nodes into existing BVH", nodes.size());

  size_t successCount = 0;
  for (auto &node : nodes) {
    if (TryInsertIntoExistingTree(node)) {
      successCount++;
    } else {
      // 插入失败，返回false
      LOG_DEBUG("Batch insert failed after {} successful insertions",
                successCount);
      return false;
    }
  }

  LOG_DEBUG("Batch insert successful: {} nodes inserted", successCount);
  return true;
}

bool SimpleBVH::TryInsertIntoExistingTree(std::shared_ptr<SceneNode> node) {
  if (!m_Root || !node) {
    return false;
  }

  // 简单的插入策略：找到最适合的叶子节点插入
  // 使用简单的启发式：选择插入后包围盒扩展最小的叶子节点

  BVHNode *bestLeaf = nullptr;
  float bestCost = std::numeric_limits<float>::max();

  // 递归查找最佳叶子节点
  std::function<void(BVHNode *)> findBestLeaf = [&](BVHNode *currentNode) {
    if (!currentNode) return;

    if (currentNode->IsLeaf()) {
      // 计算插入到这个叶子节点的成本
      BoundingVolumeAABB expandedBounds = currentNode->bounds;
      expandedBounds.Expand(node->GetWorldBounds().GetAABBApproximation());

      // 简单成本计算：扩展后的表面积 * 节点数量
      float cost = expandedBounds.GetSurfaceArea() *
                   (currentNode->sceneNodes.size() + 1);

      if (cost < bestCost &&
          currentNode->sceneNodes.size() < m_MinLeafSize * 2) {
        bestCost = cost;
        bestLeaf = currentNode;
      }
    } else {
      // 内部节点：递归检查子节点
      findBestLeaf(currentNode->left);
      findBestLeaf(currentNode->right);
    }
  };

  findBestLeaf(m_Root);

  if (bestLeaf) {
    // 插入到最佳叶子节点
    bestLeaf->sceneNodes.push_back(node);

    // 更新映射
    m_NodeToBVHNodeMap[node] = bestLeaf;

    // 标记叶子节点为脏（包围盒需要更新）
    MarkDirty(bestLeaf);

    return true;
  }

  // 没有找到合适的叶子节点，插入失败
  return false;
}

void SimpleBVH::RemoveEmptyLeaves() {
  if (m_EmptyLeaves.empty()) {
    return;
  }

  LOG_DEBUG("Removing {} empty leaves", m_EmptyLeaves.size());

  // 简单的处理：标记需要重建
  // 更复杂的实现可以尝试合并空叶子节点，
  // 但重建通常更简单
  m_ForceRebuild = true;
  m_EmptyLeaves.clear();
}

void SimpleBVH::BuildNodeMapping(BVHNode *node) {
  if (!node) return;

  if (node->IsLeaf()) {
    // 叶子节点：为每个SceneNode建立映射
    for (auto &sceneNode : node->sceneNodes) {
      m_NodeToBVHNodeMap[sceneNode] = node;
    }
  } else {
    // 内部节点：递归处理子节点
    BuildNodeMapping(node->left);
    BuildNodeMapping(node->right);
  }
}
}  // namespace mite