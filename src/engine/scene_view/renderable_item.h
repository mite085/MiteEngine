#ifndef MITE_RENDERABLE_ENTITY
#define MITE_RENDERABLE_ENTITY

#include "basic_data/mesh.h"
#include "material_system.h"
#include "scene_core/entity.h"

namespace mite {
/**
 * 可渲染实体的数据结构，用于SceneView向Renderer传递渲染数据
 * 注：仅包含渲染所需的最小字段，未来可扩展（如LOD、骨骼动画等）
 */
struct RenderableItem {
  Entity entity;             // 对应的ECS实体ID
  glm::mat4 worldTransform;  // 世界空间变换矩阵（从Transform组件计算）
  Mesh mesh;                 // 网格GPU句柄（从Mesh组件获取）
  MaterialInstanceHandle material;  // 材质句柄（从Material组件获取）

  // 渲染排序相关字段
  float distanceToCamera;  // 与摄像机的距离（用于透明物体排序）
  uint32_t renderLayer;    // 渲染层级（用于自定义渲染顺序）

  /**
   * @brief 默认构造函数
   */
  RenderableItem()
      : entity(Entity()), worldTransform(glm::mat4(1.0f)), distanceToCamera(0.0f), renderLayer(0)
  {
  }
  /**
   * @brief 参数化构造函数
   */
  RenderableItem(Entity ent,
                 const glm::mat4 &transform,
                 Mesh mesh,
                 MaterialInstanceHandle material)
      : entity(ent),
        worldTransform(transform),
        mesh(mesh),
        material(material),
        distanceToCamera(0.0f),
        renderLayer(0)
  {
  }
};
};  // namespace mite

#endif
