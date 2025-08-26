#ifndef MITE_SCENE_GRAPH_H
#define MITE_SCENE_GRAPH_H

#include "scene_core/entity.h"
#include "scene_core/scene_registry.h"
#include "spatial_partition.h"

namespace mite {
// 前向声明
class ComponentSystemManager;
class SceneRegistry;
class Entity;

/**
 * @class SceneGraph
 * @brief 场景图独立服务 - 负责场景节点层级管理和空间查询优化
 *
 * 核心职责：
 * 1. 管理场景节点的层级树结构
 * 2. 维护空间划分数据结构（BVH、Octree等）
 * 3. 提供高效的空间查询接口
 * 4. 作为编辑器场景树的唯一数据源
 * 5. 协助SceneView进行视锥体裁剪优化
 *
 * 设计原则：
 * - 独立于ECS架构，作为纯服务类存在
 * - 专注于空间数据结构和算法优化
 * - 提供稳定、高效的查询接口
 * - 支持编辑器的场景树操作
 */
class SceneGraph {
 public:
  /**
   * @brief 构造函数
   * @param spatialPartitionType 空间划分类型（默认BVH）
   */
  explicit SceneGraph(SpatialPartitionType spatialPartitionType = SpatialPartitionType::BVH);
  ~SceneGraph();

  // 禁止拷贝和移动
  SceneGraph(const SceneGraph &) = delete;
  SceneGraph &operator=(const SceneGraph &) = delete;
  SceneGraph(SceneGraph &&) = delete;
  SceneGraph &operator=(SceneGraph &&) = delete;

  // 初始化与清理均需要依赖SceneCore模块的组件系统管理器注册组件
  void Initialize(ComponentSystemManager &manager);
  void CleanUp(ComponentSystemManager &manager);

  // ==================== 视锥体与可见性设定 ====================

  /**
   * @brief 设定主相机的视锥体
   * @param frustum 视锥体对象引用
   */
  void SceneGraph::SetMainCameraFrustum(const Frustum &frustum);

  /**
   * @brief 设定主相机的可见性
   * @param mask 可见性掩码
   */
  void SceneGraph::SetCameraVisibilityMask(uint32_t mask);

  // ==================== 场景节点生命周期管理 ====================

  /**
   * @brief 为实体创建场景节点
   * @param entity 目标实体
   * @return 创建的场景节点指针，失败返回nullptr
   */
  SceneNode *CreateNode(SceneRegistry &registry, Entity entity);

  /**
   * @brief 销毁实体的场景节点
   * @param entity 目标实体
   * @return 是否成功销毁
   */
  bool DestroyNode(SceneRegistry &registry, Entity entity);

  // ==================== 场景节点查询接口 ====================

  /**
   * @brief 获取实体对应的场景节点
   * @param entity 实体句柄
   * @return 场景节点指针，不存在时返回nullptr
   */
  SceneNode *GetNode(Entity entity) const;

  /**
   * @brief 检查实体是否有对应的场景节点
   * @param entity 实体句柄
   * @return 是否存在场景节点
   */
  bool HasNode(Entity entity) const;

  /**
   * @brief 获取根节点列表（没有父节点的节点）
   * @return 根节点指针列表
   */
  std::vector<SceneNode *> GetRootNodes() const;

  /**
   * @brief 获取所有场景节点
   * @return 所有场景节点指针列表
   */
  std::vector<SceneNode *> GetAllNodes() const;

  /**
   * @brief 获取场景节点数量
   * @return 节点总数
   */
  size_t GetNodeCount() const;

  /**
   * @brief 判断场景图是否为空
   * @return 是否为空
   */
  bool IsEmpty() const;

  // ==================== 场景树操作接口（编辑器支持） ====================

  /**
   * @brief 设置节点的父节点
   * @param node 目标节点
   * @param newParent 新的父节点（nullptr表示设为根节点）
   * @return 是否成功设置
   */
  bool SetParent(SceneNode *node, SceneNode *newParent);

  /**
   * @brief 获取节点的完整路径（用于编辑器序列化）
   * @param node 目标节点
   * @return 节点路径字符串（如"Root/Camera/Light"）
   */
  std::string GetNodePath(SceneNode *node) const;

  /**
   * @brief 通过路径查找场景节点
   * @param path 节点路径
   * @return 场景节点指针，找不到返回nullptr
   */
  SceneNode *FindNodeByPath(const std::string &path) const;

  /**
   * @brief 遍历场景树执行回调函数
   * @param callback 回调函数，返回false可中断遍历
   */
  void TraverseTree(std::function<bool(SceneNode *)> callback) const;

  // ==================== 空间划分管理接口 ====================

  /**
   * @brief 设置空间划分类型
   * @param type 空间划分类型
   */
  void SetSpatialPartitionType(SpatialPartitionType type);

  /**
   * @brief 获取当前空间划分类型
   * @return 空间划分类型
   */
  SpatialPartitionType GetSpatialPartitionType() const;

  /**
   * @brief 重新构建空间划分结构（优化性能）
   */
  void RebuildSpatialPartition();

  /**
   * @brief 获取空间划分统计信息
   * @return 统计信息字符串
   */
  std::string GetSpatialPartitionStats() const;

  /**
   * @brief 调试绘制接口
   * @param drawCallback 绘制回调函数
   */
  void DebugDraw(std::function<void(const AABB &, int depth)> drawCallback);

  // ==================== 空间查询接口（为SceneView提供优化） ====================

  /**
   * @brief 查询可见节点（使用内部保存的视锥体）
   * @return 可见节点列表
   */
  std::vector<SceneNode *> QueryVisibleNodes(SceneRegistry &registry);

  /**
   * @brief 快速可见性检查（不返回具体节点，只计数）
   * @return 可见节点数量
   */
  size_t QueryVisibleCount(SceneRegistry &registry);

  /**
   * @brief 获取可见节点数量（不执行可见性检查，只获取上次检查结果）
   * @return 可见节点数量
   */
  size_t GetVisibleNodeCount() const;

  /**
   * @brief 视锥体裁剪查询 - 主要给SceneView使用
   * @param frustum 视锥体
   * @return 可见节点列表
   */
  std::vector<SceneNode *> QueryVisibleNodes(SceneRegistry &registry, const Frustum &frustum);

  /**
   * @brief 射线检测查询
   * @param ray 检测射线
   * @return 相交节点列表
   */
  std::vector<SceneNode *> QueryRaycast(SceneRegistry &registry, const Ray &ray);

  /**
   * @brief 射线检测查询（第一个命中）
   * @param ray 检测射线
   * @param result 命中的节点（输出参数）
   * @param distance 相交距离（输出参数）
   * @return 是否命中
   */
  bool QueryRaycastFirst(SceneRegistry &registry,
                         const Ray &ray,
                         SceneNode *&result,
                         float &distance);

  /**
   * @brief 球体查询
   * @param sphere 查询球体
   * @return 结果节点列表
   */
  std::vector<SceneNode *> QuerySphere(SceneRegistry &registry, const Sphere &sphere);

  /**
   * @brief AABB查询
   * @param aabb 查询AABB
   * @return 结果节点列表
   */
  std::vector<SceneNode *> QueryAABB(SceneRegistry &registry, const AABB &aabb);

  // ==================== 节点更新接口（由SceneGraphSystem调用） ====================

  /**
   * @brief 更新场景节点的变换数据
   * @param entity 目标实体
   * @param localTransform 局部变换矩阵
   */
  void UpdateNodeTransform(SceneRegistry &registry,
                           Entity entity,
                           const glm::mat4 &localTransform);

  /**
   * @brief 更新场景节点的包围盒数据
   * @param entity 目标实体
   * @param localBounds 局部包围盒
   */
  void UpdateNodeBounds(SceneRegistry &registry, Entity entity, const AABB &localBounds);

  /**
   * @brief 标记节点需要更新（变换或包围盒变化）
   * @param entity 目标实体
   */
  void MarkNodeDirty(Entity entity);

  /**
   * @brief 批量更新所有脏节点
   */
  void UpdateDirtyNodes(SceneRegistry &registry);

  // ==================== 序列化支持（为编辑器保存/加载） ====================

  /**
   * @brief 序列化场景图数据
   * @param output 输出流
   * @return 是否成功序列化
   */
  bool Serialize(std::ostream &output) const;

  /**
   * @brief 反序列化场景图数据
   * @param input 输入流
   * @return 是否成功反序列化
   */
  bool Deserialize(std::istream &input);

 private:
  // ==================== 内部工具方法 ====================

  /**
   * @brief 初始化空间划分结构
   */
  void InitializeSpatialPartition();

  /**
   * @brief 递归遍历场景树辅助函数
   */
  bool TraverseRecursive(SceneNode *node, std::function<bool(SceneNode *)> callback) const;

  /**
   * @brief 验证父子关系是否有效（防止循环引用）
   */
  bool ValidateParenting(SceneNode *node, SceneNode *newParent) const;

  /**
   * @brief 从空间划分结构中移除节点
   */
  void RemoveNodeFromSpatialPartition(SceneNode *node);

  /**
   * @brief 添加节点到空间划分结构
   */
  void AddNodeToSpatialPartition(SceneNode *node);

  /**
   * @brief 检查节点是否可见
   * @param registry 场景注册表
   * @param entity 实体
   * @return 是否可见
   */
  bool IsNodeVisible(SceneRegistry &registry, Entity entity) const;

  /**
   * @brief 清空空间划分结构,清空所有节点
   */
  void Clear();

 private:
  // 实体到场景节点的映射表
  std::unordered_map<Entity, std::unique_ptr<SceneNode>> m_entityToNodeMap;

  // 空间划分结构
  std::unique_ptr<SpatialPartition> m_spatialPartition;

  // 当前空间划分类型
  SpatialPartitionType m_spatialPartitionType;

  // 需要更新的脏节点列表
  std::vector<Entity> m_dirtyNodes;

  // 状态存储
  Frustum m_mainCameraFrustum;      // 主相机的视锥体
  uint32_t m_cameraVisibilityMask;  // 主相机的可见性（通过掩码判断，支持不同通道渲染）
  size_t m_visibleNodeCount;        // 可见节点数量

  // 线程安全保护
  mutable std::mutex m_mutex;

  // 日志器
  Logger m_logger;
};
}  // namespace mite

#endif  // MITE_SCENE_GRAPH_H
