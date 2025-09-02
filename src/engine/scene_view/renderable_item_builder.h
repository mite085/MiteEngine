#ifndef MITE_RENDERABLE_ITEM_BUILDER_H
#define MITE_RENDERABLE_ITEM_BUILDER_H

#include "material_instance.h"
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
   * 优点：
   * 1. 实现简单，便于调试
   * 2. 功能正确性优先于性能优化
   * 3. 便于后续添加更复杂的优化策略
   * 
   * 当性能成为瓶颈时，再逐步引入：
   * 1. 脏标记系统
   * 2. 对象池
   * 3. 缓存机制
   * 4. 增量更新
   * 
   * （RenderableItem构建成本仅有智能指针构建的开销，短期内不会成为较为严重的瓶颈）
   */
  std::vector<RenderableItem> BuildFromSceneNodes(SceneRegistry& registry, const std::vector<SceneNode *> &sceneNodes);

  // ==================== 核心构建接口 ====================
  /**
   * @brief 从SceneNode构建RenderableItem
   * @param sceneNode 场景节点
   * @return 构建成功的RenderableItem，如果构建失败返回空对象
   */
  RenderableItem BuildFromSceneNode(SceneRegistry &registry, SceneNode *sceneNode);

  /**
   * @brief 从Entity构建RenderableItem
   * @param entity ECS实体
   * @return 构建成功的RenderableItem，如果构建失败返回空对象
   */
  RenderableItem BuildFromEntity(SceneRegistry &registry, Entity entity);

  /**
   * @brief 批量构建RenderableItem
   * @param entities ECS实体列表
   * @return 构建成功的RenderableItem列表
   */
  std::vector<RenderableItem> BuildFromEntities(SceneRegistry &registry,
                                                const std::vector<Entity> &entities);

  // ==================== 配置接口 ====================
  /**
   * @brief 设置自定义材质覆盖函数
   * @param func 材质覆盖回调函数
   * @note 可用于特殊渲染效果或调试目的
   */
  void SetMaterialOverrideFunction(
      std::function<std::shared_ptr<MaterialInstance>(Entity, std::shared_ptr<MaterialInstance>)>
          func);

  /**
   * @brief 设置自定义变换覆盖函数
   * @param func 变换覆盖回调函数
   * @note 可用于特殊变换效果或调试目的
   */
  void SetTransformOverrideFunction(std::function<glm::mat4(Entity, const glm::mat4 &)> func);

  /**
   * @brief 设置LOD选择函数
   * @param func LOD选择回调函数
   * @note 可用于自定义LOD选择策略
   */
  void SetLODSelectorFunction(std::function<uint32_t(Entity, const std::shared_ptr<Mesh> &)> func);

  // ==================== 工具接口 ====================
  /**
   * @brief 检查SceneNode是否可渲染
   * @param sceneNode 场景节点
   * @return 是否包含渲染所需的组件
   */
  bool IsRenderable(SceneRegistry &registry, SceneNode *sceneNode) const;

  /**
   * @brief 检查Entity是否可渲染
   * @param entity ECS实体
   * @return 是否包含渲染所需的组件
   */
  bool IsRenderable(SceneRegistry &registry, Entity entity) const;

 private:
  /**
   * @brief 从实体提取网格组件
   * @param entity ECS实体
   * @return 网格组件共享指针，如果不存在返回nullptr
   */
  std::shared_ptr<Mesh> ExtractMeshComponent(SceneRegistry &registry, Entity entity);

  /**
   * @brief 从实体提取材质组件
   * @param entity ECS实体
   * @return 材质实例共享指针，如果不存在返回nullptr
   */
  std::shared_ptr<MaterialInstance> ExtractMaterialComponent(SceneRegistry &registry,
                                                             Entity entity);

  /**
   * @brief 从实体提取变换组件
   * @param entity ECS实体
   * @return 世界变换矩阵
   */
  glm::mat4 ExtractTransformComponent(SceneRegistry &registry, Entity entity);

  // 自定义回调函数（用于扩展功能）
  std::function<std::shared_ptr<MaterialInstance>(Entity, std::shared_ptr<MaterialInstance>)>
      m_materialOverrideFunc;
  std::function<glm::mat4(Entity, const glm::mat4 &)> m_transformOverrideFunc;
  std::function<uint32_t(Entity, const std::shared_ptr<Mesh> &)> m_lodSelectorFunc;

  // 禁用拷贝构造和赋值
  RenderableItemBuilder(const RenderableItemBuilder &) = delete;
  RenderableItemBuilder &operator=(const RenderableItemBuilder &) = delete;

  
  // 日志器
  Logger m_Logger;
};

}  // namespace mite

#endif  // MITE_RENDERABLE_ITEM_BUILDER_H
