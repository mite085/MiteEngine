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
    SceneRegistry &registry,
    std::shared_ptr<CameraInstance> camera,
    const std::vector<SceneNode *> &sceneNodes)
{
  std::vector<RenderableItem> items;
  items.reserve(sceneNodes.size());

  // 记录MeshInstance缓存
  size_t cachedCount = 0;
  size_t createdCount = 0;

  for (SceneNode *node : sceneNodes) {
    // 判断是否为可渲染对象
    if (IsRenderable(registry, node->GetEntity())) {
      RenderableItem item = BuildFromSceneNode(registry, camera, node);
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
  // m_Logger->trace("Built {} renderable items ({} cached, {} created)",
  //                items.size(),
  //                cachedCount,
  //                createdCount);

  return items;
}

RenderableItem RenderableItemBuilder::BuildFromSceneNode(SceneRegistry &registry,
                                                         std::shared_ptr<CameraInstance> camera,
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
    std::shared_ptr<MeshInstance> meshInstance = GetOrCreateMeshInstance(
        registry, entity, sceneNode->GetWorldTransform());
    if (!meshInstance) {
      m_Logger->warn("Failed to create MeshInstance for Entity {}", entity.GetUUIDString());
      return RenderableItem();
    }
    // 1.1. 更新MeshInstance的世界变换
    meshInstance->UpdateUBO(sceneNode->GetWorldTransform());

    // 1.2. 更新meshInstance的LOD等级
    uint32_t lodLevel = SelectMeshLODLevel(meshInstance->GetMesh(),
                                           camera->GetCameraTransform().GetPosition(),
                                           sceneNode->GetWorldTransform().GetLocalMatrix());
    meshInstance->SetMeshLODLevel(lodLevel);

    // 2. 提取材质
    std::shared_ptr<MaterialInstance> material = ExtractMaterialComponent(registry, entity);
    if (!material) {
      m_Logger->warn("Entity {} has no valid material", entity.GetUUIDString());
      return RenderableItem();
    }

    // 3. 根据包围盒计算与相机的最短距离
    glm::vec3 closestPoint = glm::clamp(camera->GetCameraTransform().GetPosition(),
                                        meshInstance->GetWorldBoundingBox().first,
                                        meshInstance->GetWorldBoundingBox().second);
    float distanceToCamera = glm::distance(camera->GetCameraTransform().GetPosition(),
                                           closestPoint);

    // 3. 构建RenderableItem
    RenderableItem item;
    item.entity = entity;
    item.worldTransform = sceneNode->GetWorldTransform();
    item.mesh = meshInstance;
    item.material = material;
    item.distanceToCamera = distanceToCamera;

    // 基于材质参数的透明性判断
    if (material->GetAlphaMode() == AlphaMode::OPAQUE) {
      item.itemType = RenderableItemType::Opaque;
    }
    else if (material->GetAlphaMode() == AlphaMode::MASK) {
      item.itemType = RenderableItemType::AlphaTest;
    }
    else if (material->GetAlphaMode() == AlphaMode::BLEND) {
      item.itemType = RenderableItemType::Transparent;
    }
    else {
      item.itemType = RenderableItemType::Opaque; // 默认按照不透明来进行
    }

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

uint32_t RenderableItemBuilder::SelectMeshLODLevel(std::shared_ptr<Mesh> mesh,
                                                   const glm::vec3 &cameraPosition,
                                                   const glm::mat4 &worldTransform,
                                                   float lodBias)
{
  if (!mesh || !mesh->GetVertexCount()) {
    LOG_ERROR("Invalid Mesh in selecting mesh lod by renderable item builder.");
    return 0;
  }

  // 获取网格的世界空间包围盒
  auto localBBox = mesh->GetBoundingBox(0);
  glm::vec3 localMin = localBBox.first;
  glm::vec3 localMax = localBBox.second;

  // 转换到世界空间
  glm::vec3 worldMin = glm::vec3(worldTransform * glm::vec4(localMin, 1.0f));
  glm::vec3 worldMax = glm::vec3(worldTransform * glm::vec4(localMax, 1.0f));
  glm::vec3 worldCenter = (worldMin + worldMax) * 0.5f;

  // 计算屏幕空间覆盖率
  //
  // 假设一个网格：
  // 原始大小：10米 × 10米（包围盒投影面积）
  // 距离相机：100米
  // 屏幕宽度：1920像素
  // screenCoverage = (10.0f / 100.0f) * 1920.0f = 192像素
  float distance = glm::distance(cameraPosition, worldCenter);
  glm::vec3 bboxSize = worldMax - worldMin;
  float objectSize = glm::max(bboxSize.x, glm::max(bboxSize.y, bboxSize.z));
  float screenCoverage = (objectSize / distance) * 1920 * lodBias;

  // 基于“1920像素宽度的”屏幕覆盖率的LOD选择
  //
  // 200.0f: 当网格在屏幕上覆盖宽度小于200像素时，切换到LOD 1
  // 100.0f: 当网格在屏幕上覆盖宽度小于100像素时，切换到LOD 2
  //  50.0f: 当网格在屏幕上覆盖宽度小于 50像素时，切换到LOD 3
  //  25.0f: 当网格在屏幕上覆盖宽度小于 25像素时，切换到LOD 4
  //  10.0f: 当网格在屏幕上覆盖宽度小于 10像素时，切换到LOD 5
  //   5.0f: 当网格在屏幕上覆盖宽度小于  5像素时，切换到LOD 6
  //
  // 注意：可以将该选择方案作为配置项，针对不同情况修改配置
  // 如：
  // 高质量场景（近处细节重要）：
  //    {300.0f, 150.0f, 75.0f, 30.0f, 15.0f, 5.0f};
  // 性能优先场景：
  //    {100.0f, 50.0f, 20.0f, 8.0f, 3.0f};
  // 环境网格（可以更早降级）：
  //    {80.0f, 40.0f, 15.0f, 5.0f};
  uint32_t selectedLOD = 0;
  constexpr float lodThresholds[] = {200.0f, 100.0f, 50.0f, 25.0f, 10.0f, 5.0f};
  for (uint32_t i = 0; i < sizeof(lodThresholds) / sizeof(lodThresholds[0]); ++i) {
    if (screenCoverage < lodThresholds[i]) {
      selectedLOD = i + 1;
    }
    else {
      break;
    }
  }

  // 获取可用的LOD级别
  std::set<uint32_t> availableLODs;
  availableLODs.insert(mesh->GetBaseSection().lodLevel);
  for (const auto &lodSection : mesh->GetSubLODSections()) {
    availableLODs.insert(lodSection.lodLevel);
  }

  // 确保选择的LOD级别实际存在
  if (!availableLODs.empty()) {
    auto it = availableLODs.lower_bound(selectedLOD);
    if (it != availableLODs.end()) {
      selectedLOD = *it;
    }
    else {
      selectedLOD = *availableLODs.rbegin();
    }
  }
  return selectedLOD;
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
  std::shared_ptr<MeshInstance> meshInstance = CreateMeshInstance(mesh, worldTransform);
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