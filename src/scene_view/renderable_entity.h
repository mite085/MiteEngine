#ifndef MITE_RENDERABLE_ENTITY
#define MITE_RENDERABLE_ENTITY

#include "scene_core/entity.h"

namespace mite {
/**
 * 可渲染实体的数据结构，用于SceneView向Renderer传递渲染数据
 * 注：仅包含渲染所需的最小字段，未来可扩展（如LOD、骨骼动画等）
 */
struct RenderableEntity {
  // TODO: 占位符，后续完善了基本逻辑后替换
  using MeshID = uint32_t;
  using MaterialID = uint32_t;

  Entity entity;             // 对应的ECS实体ID
  glm::mat4 worldTransform;  // 世界空间变换矩阵（从Transform组件计算）
  MeshID meshID;             // 网格资产ID（从Mesh组件获取）
  MaterialID materialID;     // 材质资产ID（从Material组件获取）
  // 注：可在此添加渲染排序所需的附加字段（如与摄像机的距离）
};
};

#endif
