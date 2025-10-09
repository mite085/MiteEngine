#include "renderable_item_builder.h"
#include "basic_event/instance_event.h"
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
  ClearMeshInstanceCache();
  m_Logger->debug("RenderableItemBuilder destroyed");
}

std::vector<RenderableItem> RenderableItemBuilder::BuildFromSceneNodes(
    SceneRegistry &registry, const std::vector<SceneNode *> &sceneNodes)
{
  std::vector<RenderableItem> items;
  items.reserve(sceneNodes.size());

  // 记录MeshInstance缓存
  size_t cachedCount = 0;
  size_t createdCount = 0;

  for (SceneNode *node : sceneNodes) {
    // 判断是否为可渲染对象
    if (IsRenderable(registry, node->GetEntity())) {
      RenderableItem item = BuildFromSceneNode(registry, node);
      if (item.entity.IsValid()) {  // 检查是否构建成功
        items.push_back(std::move(item));

        // 统计缓存命中情况
        if (m_MeshInstanceCache.find(node->GetEntity()) != m_MeshInstanceCache.end()) {
          cachedCount++;
        }
        else {
          createdCount++;
        }
      }
    }
  }

  // 日志记录
  m_Logger->trace("Built {} renderable items ({} cached, {} created)",
                  items.size(),
                  cachedCount,
                  createdCount);

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

  try {
    // 1. 获取或创建MeshInstance
    std::shared_ptr<MeshInstance> meshInstance = GetOrCreateMeshInstance(registry, entity, sceneNode->GetWorldTransform());
    if (!meshInstance) {
      m_Logger->warn("Failed to create MeshInstance for Entity {}", entity.GetUUIDString());
      return RenderableItem();
    }
    // 1.1. 更新MeshInstance的世界变换
    meshInstance->UpdateUBO(sceneNode->GetWorldTransform());

    // 2. 提取材质
    std::shared_ptr<MaterialInstance> material = ExtractMaterialComponent(registry, entity);
    if (!material) {
      m_Logger->warn("Entity {} has no valid material", entity.GetUUIDString());
      return RenderableItem();
    }

    // 3. 构建RenderableItem
    RenderableItem item;
    item.entity = entity;
    item.worldTransform = sceneNode->GetWorldTransform();
    item.mesh = meshInstance;
    item.material = material;

    // m_Logger->debug("Successfully built RenderableItem for Entity {}", entity.GetUUIDString());
    return item;
  }
  catch (const std::exception &e) {
    m_Logger->error(
        "Failed to build RenderableItem for Entity {}: {}", entity.GetUUIDString(), e.what());
    return RenderableItem();
  }
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

std::shared_ptr<MeshInstance> RenderableItemBuilder::GetOrCreateMeshInstance(
    SceneRegistry &registry, Entity entity, const Transform &worldTransform)
{
  // 检查缓存
  auto it = m_MeshInstanceCache.find(entity);
  if (it != m_MeshInstanceCache.end()) {
    // 缓存命中，直接返回
    return it->second;
  }

  // 缓存未命中，创建新的MeshInstance
  std::shared_ptr<Mesh> mesh = ExtractMeshComponent(registry, entity);
  if (!mesh || !mesh->GetModelHandle().vertexArray) {
    m_Logger->warn("Entity {} has invalid mesh", entity.GetUUIDString());
    return nullptr;
  }
  std::shared_ptr<MeshInstance> meshInstance = CreateMeshInstance(std::make_shared<Mesh>(mesh),
                                                                  worldTransform);
  if (!meshInstance) {
    m_Logger->error("Failed to create MeshInstance for Entity {}", entity.GetUUIDString());
    return nullptr;
  }

  // 加入缓存
  m_MeshInstanceCache[entity] = meshInstance;
  m_Logger->debug("Created and cached MeshInstance for Entity {}", entity.GetUUIDString());
  return meshInstance;
}

std::shared_ptr<Mesh> RenderableItemBuilder::ExtractMeshComponent(SceneRegistry &registry,
                                                                  Entity entity)
{
  // 直接从组件中提取Mesh
  if (registry.HasComponent<MeshComponent>(entity)) {
    auto &meshComp = registry.GetComponent<MeshComponent>(entity);
    return meshComp.GetMesh();
  }
  return nullptr;
}

std::shared_ptr<MaterialInstance> RenderableItemBuilder::ExtractMaterialComponent(
    SceneRegistry &registry, Entity entity)
{
  // 直接从组件中提取
  if (registry.HasComponent<MaterialComponent>(entity)) {
    auto &materialComp = registry.GetComponent<MaterialComponent>(entity);
    return materialComp.GetMaterialInstance();
  }
  return std::shared_ptr<MaterialInstance>();
}
std::shared_ptr<MeshInstance> RenderableItemBuilder::CreateMeshInstance(
    std::shared_ptr<Mesh> mesh, const Transform &worldTransform)
{
  try {
    // 创建MeshInstance
    auto meshInstance = std::make_shared<MeshInstance>(mesh);

    // 初始化UBO
    if (!meshInstance->InitializeUBO()) {
      m_Logger->error("Failed to initialize MeshInstance UBO");
      return nullptr;
    }
    // 初始更新变换
    meshInstance->UpdateUBO(worldTransform);

    // 发布MeshInstance创建事件，委托RenderContext注册和绑定着色器
    EventBus::Publish<MeshInstanceCreateEvent>(MeshInstanceCreateEvent(meshInstance));

    return meshInstance;
  }
  catch (const std::exception &e) {
    m_Logger->error("Failed to create MeshInstance: {}", e.what());
    return nullptr;
  }
}
void RenderableItemBuilder::ClearMeshInstanceCache()
{
  size_t cacheSize = m_MeshInstanceCache.size();
  m_MeshInstanceCache.clear();

  if (cacheSize > 0) {
    m_Logger->debug("Cleared {} MeshInstances from cache", cacheSize);
  }
}

}  // namespace mite