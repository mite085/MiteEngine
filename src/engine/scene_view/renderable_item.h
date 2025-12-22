#ifndef MITE_RENDERABLE_ENTITY
#define MITE_RENDERABLE_ENTITY

#include "basic_data/transform.h"
#include "basic_instance/material_instance.h"
#include "basic_instance/mesh_instance.h"
#include "scene_core/entity.h"

namespace mite {
/**
 * @brief 可渲染项类型枚举（主要用于RenderQueue分类）
 */
enum class RenderableItemType {
  Opaque,       // 不透明物体队列
  Transparent,  // 透明物体队列
  AlphaTest,    // Alpha测试物体队列
  Custom        // 自定义队列（预留）
};

/**
 * 可渲染实体的数据结构，用于SceneView向Renderer传递渲染数据
 * 注：仅包含渲染所需的最小字段，未来可扩展（如LOD、骨骼动画等）
 */
struct RenderableItem {
  Entity entity;                       // 对应的ECS实体ID
  Transform worldTransform;            // 世界空间变换矩阵（从SceneNode获取）
  std::shared_ptr<MeshInstance> mesh;  // 网格实例
  std::shared_ptr<MaterialInstance> material;  // 材质实例

  // 渲染排序相关字段
  float distanceToCamera;       // 与摄像机的距离（用于透明物体排序）
  uint32_t renderLayer;         // 渲染层级（用于自定义渲染顺序）
  RenderableItemType itemType;  // 可渲染项类型（用于区分前向渲染/延迟光照）

  /**
   * @brief 默认构造函数
   */
  RenderableItem()
      : entity(Entity()),
        worldTransform(Transform()),
        distanceToCamera(0.0f),
        renderLayer(0),
        itemType(RenderableItemType::Opaque) {}
  /**
   * @brief 参数化构造函数
   */
  RenderableItem(Entity ent, const Transform &transform,
                 std::shared_ptr<MeshInstance> mesh,
                 std::shared_ptr<MaterialInstance> material,
                 RenderableItemType itemType)
      : entity(ent),
        worldTransform(transform),
        mesh(mesh),
        material(material),
        distanceToCamera(0.0f),
        renderLayer(0),
        itemType(itemType) {}
};
};  // namespace mite

#endif
