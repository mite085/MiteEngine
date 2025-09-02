#include "renderable_item_builder.h"
#include "logger/logger.h"
#include "scene_core_components/material_component.h"
#include "scene_core_components/mesh_component.h"
#include "scene_core_components/transform_component.h"
#include "scene_node.h"

namespace mite {

RenderableItemBuilder::RenderableItemBuilder()
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneView RenderableItem Builder");
  // 初始化日志
  m_Logger->debug("RenderableItem Builder initialized");
}

RenderableItemBuilder::~RenderableItemBuilder()
{
  m_Logger->debug("RenderableItemBuilder destroyed");
}

std::vector<RenderableItem> RenderableItemBuilder::BuildFromSceneNodes(
    SceneRegistry &registry, const std::vector<SceneNode *> &sceneNodes)
{

  std::vector<RenderableItem> items;
  items.reserve(sceneNodes.size());

  for (SceneNode *node : sceneNodes) {
    RenderableItem item = BuildFromSceneNode(registry, node);
    if (item.entity.IsValid()) {  // 检查是否构建成功
      items.push_back(std::move(item));
    }
  }

  return items;
}

RenderableItem RenderableItemBuilder::BuildFromSceneNode(SceneRegistry &registry,
                                                         SceneNode *sceneNode)
{
  if (!sceneNode) {
    m_Logger->warn("Attempted to build from null SceneNode");
    return RenderableItem();
  }

  Entity entity = sceneNode->GetEntity();
  if (!entity.IsValid()) {
    m_Logger->warn("SceneNode has no associated Entity");
    return RenderableItem();
  }

  return BuildFromEntity(registry, entity);
}

RenderableItem RenderableItemBuilder::BuildFromEntity(SceneRegistry &registry, Entity entity)
{
  if (!entity.IsValid()) {
    m_Logger->warn("Attempted to build from null Entity");
    return RenderableItem();
  }

  if (!IsRenderable(registry, entity)) {
    //m_Logger->debug("Entity {} is not renderable", entity.GetUUIDString());
    return RenderableItem();
  }

  try {
    // 提取渲染所需组件数据
    auto mesh = ExtractMeshComponent(registry, entity);
    auto material = ExtractMaterialComponent(registry, entity);
    glm::mat4 transform = ExtractTransformComponent(registry, entity);

    if (!mesh || !material) {
      m_Logger->warn("Entity {} missing mesh or material component", entity.GetUUIDString());
      return RenderableItem();
    }

    // 应用自定义覆盖函数（如果设置）
    if (m_MaterialOverrideFunc) {
      material = m_MaterialOverrideFunc(entity, material);
    }

    if (m_TransformOverrideFunc) {
      transform = m_TransformOverrideFunc(entity, transform);
    }

    // 构建RenderableItem
    RenderableItem item;
    item.entity = entity;
    item.worldTransform = transform;
    item.mesh = mesh;
    item.material = material;

     //m_Logger->debug("Successfully built RenderableItem for Entity {}", entity.GetUUIDString());
    return item;
  }
  catch (const std::exception &e) {
    m_Logger->error(
        "Failed to build RenderableItem for Entity {}: {}", entity.GetUUIDString(), e.what());
    return RenderableItem();
  }
}

std::vector<RenderableItem> RenderableItemBuilder::BuildFromEntities(
    SceneRegistry &registry, 
    const std::vector<Entity> &entities)
{

  std::vector<RenderableItem> items;
  items.reserve(entities.size());

  for (Entity entity : entities) {
    RenderableItem item = BuildFromEntity(registry, entity);
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

  m_MaterialOverrideFunc = func;
}

void RenderableItemBuilder::SetTransformOverrideFunction(
    std::function<glm::mat4(Entity, const glm::mat4 &)> func)
{

  m_TransformOverrideFunc = func;
}

void RenderableItemBuilder::SetLODSelectorFunction(
    std::function<uint32_t(Entity, const std::shared_ptr<Mesh> &)> func)
{

  m_LODSelectorFunc = func;
}

bool RenderableItemBuilder::IsRenderable(SceneRegistry &registry, SceneNode *sceneNode) const
{
  if (!sceneNode)
    return false;
  Entity entity = sceneNode->GetEntity();
  return IsRenderable(registry, entity);
}

bool RenderableItemBuilder::IsRenderable(SceneRegistry &registry, Entity entity) const
{
  if (!entity.IsValid())
    return false;

  // 检查是否包含渲染所需的组件
  bool hasMesh = registry.HasComponent<MeshComponent>(entity);
  bool hasMaterial = registry.HasComponent<MaterialComponent>(entity);
  bool hasTransform = registry.HasComponent<TransformComponent>(entity);

  return hasMesh && hasMaterial && hasTransform;
}

std::shared_ptr<Mesh> RenderableItemBuilder::ExtractMeshComponent(SceneRegistry &registry,
                                                                  Entity entity)
{
  if (registry.HasComponent<MeshComponent>(entity)) {
    auto &meshComp = registry.GetComponent<MeshComponent>(entity);
    return meshComp.GetMesh();
  }
  return nullptr;
}

std::shared_ptr<MaterialInstance> RenderableItemBuilder::ExtractMaterialComponent(
    SceneRegistry &registry, Entity entity)
{
  if (registry.HasComponent<MaterialComponent>(entity)) {
    auto &materialComp = registry.GetComponent<MaterialComponent>(entity);
    return materialComp.GetMaterial();
  }
  return nullptr;
}

glm::mat4 RenderableItemBuilder::ExtractTransformComponent(SceneRegistry &registry, Entity entity)
{
  if (registry.HasComponent<TransformComponent>(entity)) {
    auto &transformComp = registry.GetComponent<TransformComponent>(entity);
    return transformComp.GetWorldMatrix(registry);
  }
  return glm::mat4(1.0f);  // 返回单位矩阵作为默认值
}

}  // namespace mite
