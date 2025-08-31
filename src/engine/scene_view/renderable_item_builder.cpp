#include "renderable_item_builder.h"
#include "logger/logger.h"
#include "scene_core_components/material_component.h"
#include "scene_core_components/mesh_component.h"
#include "scene_core_components/transform_component.h"
#include "scene_node.h"

namespace mite {

RenderableItemBuilder::RenderableItemBuilder()
{
  // 初始化日志
  LOG_DEBUG("RenderableItemBuilder initialized");
}

RenderableItemBuilder::~RenderableItemBuilder()
{
  LOG_DEBUG("RenderableItemBuilder destroyed");
}

RenderableItem RenderableItemBuilder::BuildFromSceneNode(SceneNode *sceneNode)
{
  if (!sceneNode) {
    LOG_WARN("Attempted to build from null SceneNode");
    return RenderableItem();
  }

  Entity entity = sceneNode->GetEntity();
  if (!entity.IsValid()) {
    LOG_WARN("SceneNode has no associated Entity");
    return RenderableItem();
  }

  return BuildFromEntity(entity);
}

RenderableItem RenderableItemBuilder::BuildFromEntity(Entity entity)
{
  if (!entity.IsValid()) {
    LOG_WARN("Attempted to build from null Entity");
    return RenderableItem();
  }

  if (!IsRenderable(entity)) {
    LOG_DEBUG("Entity {} is not renderable", entity.GetUUIDString());
    return RenderableItem();
  }

  try {
    // 提取渲染所需组件数据
    auto mesh = ExtractMeshComponent(entity);
    auto material = ExtractMaterialComponent(entity);
    glm::mat4 transform = ExtractTransformComponent(entity);

    if (!mesh || !material) {
      LOG_WARN("Entity {} missing mesh or material component", entity.GetUUIDString());
      return RenderableItem();
    }

    // 应用自定义覆盖函数（如果设置）
    if (m_materialOverrideFunc) {
      material = m_materialOverrideFunc(entity, material);
    }

    if (m_transformOverrideFunc) {
      transform = m_transformOverrideFunc(entity, transform);
    }

    // 构建RenderableItem
    RenderableItem item;
    item.entity = entity;
    item.worldTransform = transform;
    item.mesh = mesh;
    item.material = material;

    // 设置LOD级别（如果设置了LOD选择器）
    if (m_lodSelectorFunc) {
      // 这里可以设置LOD相关数据，具体实现取决于LOD系统设计
      // item.lodLevel = m_lodSelectorFunc(entity, mesh);
    }

    LOG_DEBUG("Successfully built RenderableItem for Entity {}", entity.GetUUIDString());
    return item;
  }
  catch (const std::exception &e) {
    LOG_ERROR(
        "Failed to build RenderableItem for Entity {}: {}", entity.GetUUIDString(), e.what());
    return RenderableItem();
  }
}

std::vector<RenderableItem> RenderableItemBuilder::BuildFromSceneNodes(
    const std::vector<SceneNode *> &sceneNodes)
{

  std::vector<RenderableItem> items;
  items.reserve(sceneNodes.size());

  for (SceneNode *node : sceneNodes) {
    RenderableItem item = BuildFromSceneNode(node);
    if (item.entity.IsValid()) {  // 检查是否构建成功
      items.push_back(std::move(item));
    }
  }

  return items;
}

std::vector<RenderableItem> RenderableItemBuilder::BuildFromEntities(
    const std::vector<Entity> &entities)
{

  std::vector<RenderableItem> items;
  items.reserve(entities.size());

  for (Entity entity : entities) {
    RenderableItem item = BuildFromEntity(entity);
    if (item.entity.IsValid()) {  // 检查是否构建成功
      items.push_back(std::move(item));
    }
  }

  return items;
}

void RenderableItemBuilder::SetMaterialOverrideFunction(
    std::function<std::shared_ptr<MaterialInstance>(Entity, std::shared_ptr<MaterialInstance>)>
        func)
{

  m_materialOverrideFunc = func;
}

void RenderableItemBuilder::SetTransformOverrideFunction(
    std::function<glm::mat4(Entity, const glm::mat4 &)> func)
{

  m_transformOverrideFunc = func;
}

void RenderableItemBuilder::SetLODSelectorFunction(
    std::function<uint32_t(Entity, const std::shared_ptr<Mesh> &)> func)
{

  m_lodSelectorFunc = func;
}

bool RenderableItemBuilder::IsRenderable(SceneNode *sceneNode) const
{
  if (!sceneNode)
    return false;
  Entity entity = sceneNode->GetEntity();
  return IsRenderable(entity);
}

bool RenderableItemBuilder::IsRenderable(Entity entity) const
{
  if (!entity.IsValid())
    return false;

  // 检查是否包含渲染所需的组件
  bool hasMesh = m_registry.HasComponent<MeshComponent>(entity);
  bool hasMaterial = m_registry.HasComponent<MaterialComponent>(entity);
  bool hasTransform = m_registry.HasComponent<TransformComponent>(entity);

  return hasMesh && hasMaterial && hasTransform;
}

std::shared_ptr<Mesh> RenderableItemBuilder::ExtractMeshComponent(Entity entity)
{
  if (m_registry.HasComponent<MeshComponent>(entity)) {
    auto &meshComp = m_registry.GetComponent<MeshComponent>(entity);
    return meshComp.GetMesh();
  }
  return nullptr;
}

std::shared_ptr<MaterialInstance> RenderableItemBuilder::ExtractMaterialComponent(Entity entity)
{
  if (m_registry.HasComponent<MaterialComponent>(entity)) {
    auto &materialComp = m_registry.GetComponent<MaterialComponent>(entity);
    return materialComp.GetMaterial();
  }
  return nullptr;
}

glm::mat4 RenderableItemBuilder::ExtractTransformComponent(Entity entity)
{
  if (m_registry.HasComponent<TransformComponent>(entity)) {
    auto &transformComp = m_registry.GetComponent<TransformComponent>(entity);
    return transformComp.GetWorldMatrix();
  }
  return glm::mat4(1.0f);  // 返回单位矩阵作为默认值
}

}  // namespace mite
