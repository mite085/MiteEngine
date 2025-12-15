#ifndef MITE_RENDERABLE_ITEM_BUILDER_H
#define MITE_RENDERABLE_ITEM_BUILDER_H

#include "basic_instance/camera_instance.h"
#include "renderable_item.h"
#include "scene_core/entity.h"
#include "scene_core/scene_registry.h"
#include "scene_node.h"

namespace mite {
// 前向声明
class MeshComponent;
class MaterialComponent;
class TransformComponent;

/**
 * @brief RenderableItem构建器
 * @note 职责：将SceneNode转换为RenderableItem，处理数据提取和转换逻辑
 * @note 单一职责：专注于SceneNode到渲染数据的转换，不涉及渲染状态管理
 */
class RenderableItemBuilder {
 public:
  /**
   * @brief 构造函数
   * @param registry 场景注册表引用（用于组件查询）
   */
  explicit RenderableItemBuilder();

  /**
   * @brief 析构函数
   */
  ~RenderableItemBuilder();

  // ==================== 与SceneGraph对接的接口 ====================
  /**
   * @brief 批量构建RenderableItem
   * @param sceneNodes 场景节点列表
   * @return 构建成功的RenderableItem列表
   *
   * 开发前期，每帧对所有SceneNode构建RenderableItem
   * RenderableItem构建成本仅有智能指针构建的开销，短期内不会成为较为严重的瓶颈
   *
   * 优点：
   * 1. 架构简单，便于调试
   * 2. 功能正确性优先于性能优化
   * 3. 便于后续添加更复杂的优化策略
   *
   * 当性能成为瓶颈时，再逐步引入：
   * 1. 脏标记系统
   * 2. 对象池
   * 3. 缓存机制
   * 4. 增量更新
   *
   * 注意：
   * 由于MeshInstance涉及UBO的创建、绑定和更新，所以必须优先实现缓存机制，
   * 以确保创建和绑定仅执行一次，但Update操作每帧更新
   */
  std::vector<RenderableItem> BuildFromSceneNodes(SceneRegistry &registry,
                                                  std::shared_ptr<CameraInstance> camera,
                                                  const std::vector<std::shared_ptr<SceneNode> > &sceneNodes);

  // ==================== 核心构建接口 ====================
  /**
   * @brief 从SceneNode构建RenderableItem
   * @param sceneNode 场景节点
   * @return 构建成功的RenderableItem，如果构建失败返回空对象
   */
  RenderableItem BuildFromSceneNode(SceneRegistry &registry,
                                    std::shared_ptr<CameraInstance> camera,
                                    std::shared_ptr<SceneNode> sceneNode);

  // ==================== 缓存管理接口 ====================
  /**
   * @brief 清空MeshInstance缓存
   * @note 在场景切换或需要强制刷新时调用
   */
  void ClearMeshInstanceCache();
  /**
   * @brief 获取缓存统计信息
   */
  size_t GetMeshInstanceCacheSize() const { return m_MeshInstanceCache.size(); }

  // ==================== 工具接口 ====================
  /**
   * @brief 检查SceneNode是否可渲染
   * @param sceneNode 场景节点
   * @return 是否包含渲染所需的组件
   */
  bool IsRenderable(SceneRegistry &registry, std::shared_ptr<SceneNode> sceneNode) const;
  /**
   * @brief 检查Entity是否可渲染
   * @param entity ECS实体
   * @return 是否包含渲染所需的组件
   */
  bool IsRenderable(SceneRegistry &registry, Entity entity) const;
  /**
   * @brief SelectMeshLODLevel 根据输入LOD偏差，选择单个Mesh的LOD层级
   * @param mesh 网格体对象
   * @param cameraPosition 相机距离
   * @param worldTransform 局部空间到世界空间的旋转矩阵
   * @param lodBias LOD层级偏差值(偏差值越高，越倾向于高精度，如10代表距离为默认值10倍时才切换下一个LOD)
   * @return LOD层级
   *
   * 针对超大Model（如地形）可以逐Mesh划分LOD，降低渲染压力
   */
  static uint32_t SelectMeshLODLevel(std::shared_ptr<Mesh> mesh,
                                     const glm::vec3 &cameraPosition,
                                     const glm::mat4 &worldTransform,
                                     float lodBias = 10.0f);

 private:
  /**
   * @brief 从实体提取网格组件并创建MeshInstance
   * @param entity ECS实体
   * @param worldTransform 世界变换
   * @return MeshInstance共享指针
   */
  std::shared_ptr<MeshInstance> GetOrCreateMeshInstance(SceneRegistry &registry,
                                                        Entity entity,
                                                        const Transform &worldTransform);
  /**
   * @brief 从实体提取网格组件
   * @param entity ECS实体
   * @return 网格组件共享指针，如果不存在返回nullptr
   */
  std::shared_ptr<Mesh> ExtractMeshComponent(SceneRegistry &registry, Entity entity);
  /**
   * @brief 从实体提取材质组件
   * @param entity ECS实体
   * @return 材质实例Handle
   */
  std::shared_ptr<MaterialInstance> ExtractMaterialComponent(SceneRegistry &registry,
                                                             Entity entity);
  /**
   * @brief 创建新的MeshInstance
   */
  std::shared_ptr<MeshInstance> CreateMeshInstance(std::shared_ptr<Mesh> mesh,
                                                   const Transform &worldTransform);

  // 禁用拷贝构造和赋值
  RenderableItemBuilder(const RenderableItemBuilder &) = delete;
  RenderableItemBuilder &operator=(const RenderableItemBuilder &) = delete;

  // MeshInstance缓存管理：Entity ID -> MeshInstance 映射
  std::unordered_map<Entity, std::shared_ptr<MeshInstance>> m_MeshInstanceCache;

  // 日志器
  Logger m_Logger;
};
}  // namespace mite

#endif  // MITE_RENDERABLE_ITEM_BUILDER_H
