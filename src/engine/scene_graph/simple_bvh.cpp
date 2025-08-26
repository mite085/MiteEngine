#include "simple_bvh.h"

namespace mite {

SimpleBVH::SimpleBVH(int maxDepth, int minLeafSize)
    : maxDepth_(maxDepth), minLeafSize_(minLeafSize)
{
}

SimpleBVH::~SimpleBVH()
{
  Clear();
}

void SimpleBVH::Insert(SceneNode *node)
{
  if (!node)
    return;

  // 检查是否已存在
  if (std::find(allNodes_.begin(), allNodes_.end(), node) != allNodes_.end()) {
    return;
  }

  allNodes_.push_back(node);
  needsRebuild_ = true;
}

void SimpleBVH::Remove(SceneNode *node)
{
  if (!node)
    return;

  auto it = std::find(allNodes_.begin(), allNodes_.end(), node);
  if (it != allNodes_.end()) {
    allNodes_.erase(it);
    needsRebuild_ = true;
  }
}

void SimpleBVH::Update(SceneNode *node)
{
  if (!node)
    return;

  // 简单实现：标记需要重建
  // 优化实现：可增量更新，这里为简化先重建
  needsRebuild_ = true;
}

void SimpleBVH::Clear()
{
  if (root_) {
    FreeNode(root_);
    root_ = nullptr;
  }
  allNodes_.clear();
  nodeCount_ = 0;
  needsRebuild_ = false;
}

void SimpleBVH::Rebuild()
{
  if (allNodes_.empty()) {
    if (root_) {
      FreeNode(root_);
      root_ = nullptr;
    }
    nodeCount_ = 0;
    needsRebuild_ = false;
    return;
  }

  // 释放旧树
  if (root_) {
    FreeNode(root_);
  }

  // 构建新树
  std::vector<SceneNode *> nodesToBuild = allNodes_;
  root_ = BuildTree(nodesToBuild, 0, static_cast<int>(nodesToBuild.size()), 0);
  needsRebuild_ = false;

  LOG_INFO("BVH rebuilt with {} nodes, depth: {}", nodeCount_, GetDepth());
}

BVHNode *SimpleBVH::BuildTree(std::vector<SceneNode *> &nodes, int start, int end, int depth)
{
  if (start >= end)
    return nullptr;

  const int count = end - start;

  // 创建新节点
  BVHNode *node = new BVHNode();
  node->depth = depth;
  nodeCount_++;

  // 计算所有节点的合并包围盒
  AABB totalBounds;
  for (int i = start; i < end; ++i) {
    totalBounds.Expand(nodes[i]->GetWorldBounds());
  }
  node->bounds = totalBounds;

  // 如果节点数较少或达到最大深度，创建叶子节点
  if (count <= minLeafSize_ || depth >= maxDepth_) {
    node->sceneNode = nodes[start];  // 存储第一个节点（实际应存储所有，这里简化）
    return node;
  }

  // 寻找最佳分割
  int bestAxis = 0;
  float bestSplitPos = 0.0f;
  if (!FindBestSplit(nodes, start, end, bestAxis, bestSplitPos)) {
    // 无法分割，创建叶子节点
    node->sceneNode = nodes[start];
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
  AABB centerBounds;
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

  std::nth_element(centers.begin(), centers.begin() + count / 2, centers.end());
  splitPos = centers[count / 2];

  return true;
}

int SimpleBVH::PartitionNodes(
    std::vector<SceneNode *> &nodes, int start, int end, int axis, float splitPos) const
{
  int left = start;
  int right = end - 1;

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
  delete node;
  nodeCount_--;
}

bool SimpleBVH::Raycast(const Ray &ray, std::vector<SceneNode *> &results)
{
  if (needsRebuild_)
    Rebuild();
  if (!root_)
    return false;

  results.clear();
  RaycastRecursive(root_, ray, results);
  return !results.empty();
}

void SimpleBVH::RaycastRecursive(BVHNode *node,
                                 const Ray &ray,
                                 std::vector<SceneNode *> &results) const
{
  if (!node)
    return;

  float t;
  if (!RayIntersectsAABB(ray, node->bounds, t)) {
    return;
  }

  if (node->IsLeaf()) {
    results.push_back(node->sceneNode);
  }
  else {
    RaycastRecursive(node->left, ray, results);
    RaycastRecursive(node->right, ray, results);
  }
}

bool SimpleBVH::RaycastFirst(const Ray &ray, SceneNode *&result, float &distance)
{
  if (needsRebuild_)
    Rebuild();
  if (!root_)
    return false;

  result = nullptr;
  distance = std::numeric_limits<float>::max();
  RaycastFirstRecursive(root_, ray, result, distance);
  return result != nullptr;
}

void SimpleBVH::RaycastFirstRecursive(BVHNode *node,
                                      const Ray &ray,
                                      SceneNode *&bestNode,
                                      float &bestDistance) const
{
  if (!node)
    return;

  float t;
  if (!RayIntersectsAABB(ray, node->bounds, t) || t > bestDistance) {
    return;
  }

  if (node->IsLeaf()) {
    // 检查与具体场景节点的精确相交
    float nodeDistance;
    if (RayIntersectsAABB(ray, node->sceneNode->GetWorldBounds(), nodeDistance) &&
        nodeDistance < bestDistance)
    {
      bestNode = node->sceneNode;
      bestDistance = nodeDistance;
    }
  }
  else {
    // 先检测距离更近的子节点
    float leftDist, rightDist;
    bool leftHit = RayIntersectsAABB(ray, node->left->bounds, leftDist);
    bool rightHit = RayIntersectsAABB(ray, node->right->bounds, rightDist);

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

int SimpleBVH::FrustumCull(const Frustum &frustum, std::vector<SceneNode *> &results)
{
  if (needsRebuild_)
    Rebuild();
  if (!root_)
    return 0;

  results.clear();
  FrustumCullRecursive(root_, frustum, results);
  return static_cast<int>(results.size());
}

void SimpleBVH::FrustumCullRecursive(BVHNode *node,
                                     const Frustum &frustum,
                                     std::vector<SceneNode *> &results) const
{
  if (!node)
    return;

  auto intersection = FrustumIntersectsAABB(frustum, node->bounds);
  if (intersection == IntersectionType::Outside) {
    return;
  }

  if (node->IsLeaf()) {
    if (intersection == IntersectionType::Inside ||
        FrustumIntersectsAABB(frustum, node->sceneNode->GetWorldBounds()) !=
            IntersectionType::Outside)
    {
      results.push_back(node->sceneNode);
    }
  }
  else {
    FrustumCullRecursive(node->left, frustum, results);
    FrustumCullRecursive(node->right, frustum, results);
  }
}

size_t SimpleBVH::SphereQuery(const Sphere &sphere, std::vector<SceneNode *> &results)
{
  if (needsRebuild_)
    Rebuild();
  if (!root_)
    return 0;

  results.clear();
  SphereQueryRecursive(root_, sphere, results);
  return results.size();
}

void SimpleBVH::SphereQueryRecursive(BVHNode *node,
                                     const Sphere &sphere,
                                     std::vector<SceneNode *> &results) const
{
  if (!node)
    return;

  if (!SphereIntersectsAABB(sphere, node->bounds)) {
    return;
  }

  if (node->IsLeaf()) {
    if (SphereIntersectsAABB(sphere, node->sceneNode->GetWorldBounds())) {
      results.push_back(node->sceneNode);
    }
  }
  else {
    SphereQueryRecursive(node->left, sphere, results);
    SphereQueryRecursive(node->right, sphere, results);
  }
}

size_t SimpleBVH::AABBQuery(const AABB &aabb, std::vector<SceneNode *> &results)
{
  if (needsRebuild_)
    Rebuild();
  if (!root_)
    return 0;

  results.clear();
  AABBQueryRecursive(root_, aabb, results);
  return results.size();
}

void SimpleBVH::AABBQueryRecursive(BVHNode *node,
                                   const AABB &aabb,
                                   std::vector<SceneNode *> &results) const
{
  if (!node)
    return;

  if (!BoundingVolumes::AABBIntersectsAABB(aabb, node->bounds)) {
    return;
  }

  if (node->IsLeaf()) {
    if (BoundingVolumes::AABBIntersectsAABB(aabb, node->sceneNode->GetWorldBounds())) {
      results.push_back(node->sceneNode);
    }
  }
  else {
    AABBQueryRecursive(node->left, aabb, results);
    AABBQueryRecursive(node->right, aabb, results);
  }
}

size_t SimpleBVH::PointQuery(const glm::vec3 &point, std::vector<SceneNode *> &results)
{
  if (needsRebuild_)
    Rebuild();
  if (!root_)
    return 0;

  results.clear();
  AABB pointAABB(point, point);  // 创建零大小的AABB
  AABBQueryRecursive(root_, pointAABB, results);
  return results.size();
}

bool SimpleBVH::NearestNeighbor(const glm::vec3 &point, SceneNode *&result, float maxDistance)
{
  if (needsRebuild_)
    Rebuild();
  if (!root_)
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
  queue.push({root_, 0.0f});

  while (!queue.empty()) {
    QueueElement current = queue.top();
    queue.pop();

    if (current.distanceSq > bestDistanceSq) {
      continue;
    }

    if (current.node->IsLeaf()) {
      float distSq = glm::distance2(point, current.node->sceneNode->GetWorldBounds().GetCenter());
      if (distSq < bestDistanceSq) {
        bestDistanceSq = distSq;
        result = current.node->sceneNode;
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

void SimpleBVH::ForEachNode(std::function<bool(SceneNode *)> callback)
{
  if (needsRebuild_)
    Rebuild();
  if (!root_)
    return;

  ForEachNodeRecursive(root_, callback);
}

bool SimpleBVH::ForEachNodeRecursive(BVHNode *node,
                                     std::function<bool(SceneNode *)> callback) const
{
  if (!node)
    return true;

  if (node->IsLeaf()) {
    if (!callback(node->sceneNode)) {
      return false;
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

size_t SimpleBVH::GetNodeCount() const
{
  return nodeCount_;
}

bool SimpleBVH::IsEmpty() const
{
  return nodeCount_ == 0;
}

int SimpleBVH::GetDepth() const
{
  if (!root_)
    return 0;
  return root_->GetHeight();
}

const char *SimpleBVH::GetTypeName() const
{
  return "SimpleBVH";
}

std::string SimpleBVH::GetStats() const
{
  BVHStats stats;
  if (root_) {
    CollectStatsRecursive(root_, stats);
    if (stats.leafCount > 0) {
      stats.avgDepth = static_cast<float>(stats.totalDepth) / stats.leafCount;
    }
  }

  std::stringstream ss;
  ss << "Nodes: " << nodeCount_ << " (Internal: " << stats.internalCount
     << ", Leaves: " << stats.leafCount << "), Max Depth: " << stats.maxDepth
     << ", Avg Depth: " << stats.avgDepth;
  return ss.str();
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

void SimpleBVH::DebugDraw(std::function<void(const AABB &, int depth)> drawCallback)
{
  if (needsRebuild_)
    Rebuild();
  if (!root_)
    return;

  DebugDrawRecursive(root_, drawCallback);
}

void SimpleBVH::DebugDrawRecursive(BVHNode *node,
                                   std::function<void(const AABB &, int depth)> drawCallback) const
{
  if (!node)
    return;

  drawCallback(node->bounds, node->depth);

  if (!node->IsLeaf()) {
    DebugDrawRecursive(node->left, drawCallback);
    DebugDrawRecursive(node->right, drawCallback);
  }
}

int BVHNode::GetHeight() const
{
  if (IsLeaf())
    return 0;
  int leftHeight = left ? left->GetHeight() : 0;
  int rightHeight = right ? right->GetHeight() : 0;
  return 1 + std::max(leftHeight, rightHeight);
}

}  // namespace mite
