#include "scene_view.h"
#include "scene_core_components/component_headers.h"

namespace mite {
SceneView::SceneView(SceneCore &sceneCore, SceneGraph &sceneGraph)
    : m_SceneCore(sceneCore),
      m_SceneGraph(sceneGraph),
      m_Builder(std::make_unique<RenderableItemBuilder>()),
      m_RenderQueue(std::make_shared<RenderQueue>()),
      m_CameraInstance(nullptr),
      m_LastVisibleNodeCount(0),
      m_LastRenderItemCount(0)
{
  // 初始化日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneView");

  // Viewport事件订阅
  m_EventSubscriptions.SubscribeImmediate<ViewportResizeEvent>(BIND_DISPATCH_FN(OnViewportResize));
  m_EventSubscriptions.SubscribeImmediate<ViewportPickedEvent>(BIND_DISPATCH_FN(OnViewportPicked));
  m_EventSubscriptions.SubscribeImmediate<ViewportCameraUpdateEvent>(
      BIND_DISPATCH_FN(OnViewportCameraUpdated));
  m_EventSubscriptions.SubscribeImmediate<ViewportPickedUpdateEvent>(
      BIND_DISPATCH_FN(OnViewportPickedUpdated));
}
SceneView::~SceneView()
{
  m_Logger->debug("SceneView destroyed");
}
void SceneView::Initialize()
{
  // 1. 创建相机实体（每个SceneView持有唯一的相机实体，若考虑多视口则创建多个SceneView）
  m_CameraEntity = m_SceneCore.CreateEntity("camera");
  // 1.1. 主相机的相机组件
  CameraComponent &cameraComponent = m_SceneCore.GetRegistry().AddComponent<CameraComponent>(
      m_CameraEntity);
  // 1.2. 主相机的变换组件
  TransformComponent &cameraTransform = m_SceneCore.GetRegistry().AddComponent<TransformComponent>(
      m_CameraEntity);
  cameraTransform.SetLocalPosition(glm::vec3(5.0f, 5.0f, 5.0f));
  cameraTransform.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
  // 1.3. 主相机的可见性组件
  VisibilityComponent &cameraVisibility =
      m_SceneCore.GetRegistry().AddComponent<VisibilityComponent>(m_CameraEntity);
  // 1.4. 主相机包围盒组件（仅当创建了包围盒才会纳入SceneGraph的空间加速结构管理）
  BoundingVolumeComponent &cameraBoundingVolume =
      m_SceneCore.GetRegistry().AddComponent<BoundingVolumeComponent>(m_CameraEntity);
  cameraBoundingVolume.SetVolume(BoundingVolume::CreateFromPoints(
      BoundingVolumeType::AABB,
      {glm::vec3(0.0f)}));  // 包含本地空间原点即可。Camera不存在实际意义上的包围盒

  // 2. 创建相机实例
  m_CameraInstance = std::make_shared<CameraInstance>(cameraComponent.GetCamera());
  m_CameraInstance->InitializeUBO();
  // 2.1. 初始化UBO后发布事件委托RenderContext注册并执行着色器绑定
  EventBus::Publish<CameraInstanceCreateEvent>(CameraInstanceCreateEvent(m_CameraInstance));

  m_Logger->debug("SceneView initialized");
}

void SceneView::Update()
{
  // 1. 使用相机世界坐标更新相机实例
  const Transform cameraWorldTransform = m_SceneGraph.GetNode(m_CameraEntity)->GetWorldTransform();
  m_CameraInstance->UpdateUBO(cameraWorldTransform);

  // 2. 获取ViewProjection矩阵，构造视锥体
  glm::mat4 cameraView = cameraWorldTransform.GetViewMatrix();
  glm::mat4 cameraProjection = m_SceneCore.GetRegistry()
                                   .GetComponent<CameraComponent>(m_CameraEntity)
                                   .GetProjectionMatrix();
  Frustum cameraFrustum(cameraProjection * cameraView);

  // 3. 执行视锥体裁剪查询，获取可见节点列表
  uint32_t cameraVisibilityMask = m_SceneCore.GetRegistry()
                                      .GetComponent<VisibilityComponent>(m_CameraEntity)
                                      .GetVisibilityMask();
  std::vector<SceneNode *> visibleNodes = m_SceneGraph.FrustumCull(cameraFrustum,
                                                                   cameraVisibilityMask);

  // 4. 处理可见节点，构建RenderQueue
  ProcessVisibility(visibleNodes);

  // m_Logger->debug("SceneView updated in {:.3f}ms, visible nodes: {}, render items: {}",
  //                m_lastUpdateTime,
  //                m_lastVisibleNodeCount,
  //                m_lastRenderItemCount);
}

std::shared_ptr<RenderQueue> SceneView::GetRenderQueue() const
{
  return m_RenderQueue;
}

bool SceneView::Pick(glm::vec2 screenPosUV)
{
  // 获取相机ViewProjection矩阵
  const Transform cameraWorldTransform = m_SceneGraph.GetNode(m_CameraEntity)->GetWorldTransform();
  glm::mat4 cameraView = cameraWorldTransform.GetViewMatrix();
  glm::mat4 cameraProjection = m_SceneCore.GetRegistry()
                                   .GetComponent<CameraComponent>(m_CameraEntity)
                                   .GetProjectionMatrix();

  // 构建Ray
  Ray ray = Ray::GenerateRayFromScreenUV(screenPosUV, cameraView, cameraProjection);

  // 执行RayCast
  SceneNode *node = m_SceneGraph.RaycastFirst(ray);
  if (node) {
    // 查询到节点，更新PickedEntity
    m_PickedEntity = node->GetEntity();
    return true;
  }
  else {
    // 未命中节点，清空当前pick对象
    m_PickedEntity = Entity();
    return false;
  }
}

void SceneView::SetPickedWorldTransform(const Transform &worldTransform)
{
  // 检查实体可用性与在SceneGraph中是否存在对应节点
  if (!m_PickedEntity.IsValid() || !m_SceneGraph.GetNode(m_PickedEntity))
    return;

  // 通过SceneCore获取Picked的变换组件
  TransformComponent &pickedTransformComponent =
      m_SceneCore.GetRegistry().GetComponent<TransformComponent>(m_PickedEntity);

  // 通过SceneGraph获取Picked的Parent
  SceneNode *pickedParent = m_SceneGraph.GetNode(m_PickedEntity)->GetParent();
  if (pickedParent) {
    // 若Parent存在，则根据Parent的WorldTransform更新picked本地坐标
    // World = Parent * Local，可知Local = inv(Parent) * World（等式两边均左乘inv(Parent)）
    const Transform parentWorld = pickedParent->GetWorldTransform();
    pickedTransformComponent.SetLocalMatrix(glm::inverse(parentWorld.GetLocalMatrix()) *
                                            worldTransform.GetLocalMatrix());
  }
  else {
    // 若不存在，则直接更新本地坐标
    pickedTransformComponent.SetLocalMatrix(worldTransform.GetLocalMatrix());
  }
}
Transform SceneView::GetPickedWorldTransform() const
{
  // 检查实体可用性与在SceneGraph中是否存在对应节点
  if (!m_PickedEntity.IsValid() || !m_SceneGraph.GetNode(m_PickedEntity))
    return Transform(1.0f);

  SceneNode *pickedNode = m_SceneGraph.GetNode(m_PickedEntity);

  return pickedNode->GetWorldTransform();
}
void SceneView::SetCameraWorldTransform(const Transform &worldTransform)
{
  // 检查实体可用性
  if (!m_CameraEntity.IsValid() || !m_SceneGraph.GetNode(m_CameraEntity))
    return;

  // 通过SceneCore获取变换组件
  TransformComponent &cameraTransformComponent =
      m_SceneCore.GetRegistry().GetComponent<TransformComponent>(m_CameraEntity);

  // 通过SceneGraph获取相机的Parent
  SceneNode *cameraParent = m_SceneGraph.GetNode(m_CameraEntity)->GetParent();
  if (cameraParent) {
    // 若Parent存在，则根据Parent的WorldTransform更新相机本地坐标
    // World = Parent * Local，可知Local = inv(Parent) * World（等式两边均左乘inv(Parent)）
    const Transform parentWorld = cameraParent->GetWorldTransform();
    cameraTransformComponent.SetLocalMatrix(glm::inverse(parentWorld.GetLocalMatrix()) *
                                            worldTransform.GetLocalMatrix());
  }
  else {
    // 若不存在，则直接更新本地坐标
    cameraTransformComponent.SetLocalMatrix(worldTransform.GetLocalMatrix());
  }
}

void SceneView::SetCameraZoom(float zoom)
{
  // 检查实体可用性
  if (!m_CameraEntity.IsValid() || !m_SceneGraph.GetNode(m_CameraEntity))
    return;

  // 通过SceneCore获取相机组件
  CameraComponent &cameraComponent = m_SceneCore.GetRegistry().GetComponent<CameraComponent>(
      m_CameraEntity);

  // 执行zoom
  cameraComponent.Zoom(zoom);
}

size_t SceneView::GetVisibleNodeCount() const
{
  return m_LastVisibleNodeCount;
}
size_t SceneView::GetRenderItemCount() const
{
  return m_LastRenderItemCount;
}
void SceneView::ProcessVisibility(std::vector<SceneNode *> visibleNodes)
{
  // 1. 构建渲染项（使用相机实例辅助选择LOD）
  std::vector<RenderableItem> renderItems = m_Builder->BuildFromSceneNodes(
      m_SceneCore.GetRegistry(), m_CameraInstance, visibleNodes);

  // 2. 更新计数
  m_LastVisibleNodeCount = visibleNodes.size();
  m_LastRenderItemCount = renderItems.size();

  // 3. 更新渲染队列
  m_RenderQueue->ClearAll();
  m_RenderQueue->AddItems(renderItems);
  m_RenderQueue->SortAll();
}
void SceneView::OnViewportResize(ViewportResizeEvent &event)
{
  glm::vec2 currentSize = event.GetSize();

  if (currentSize.y > 0) {
    // 设置相机宽高比（避免画面拉伸）
    float aspectRatio = static_cast<float>(currentSize.x) / static_cast<float>(currentSize.y);
    m_CameraInstance->GetCamera()->SetAspectRatio(aspectRatio);

    event.SetResult(EventResult::Failed);
    return;
  }

  event.SetResult(EventResult::Failed);
  return;
}
void SceneView::OnViewportPicked(ViewportPickedEvent &event)
{
  // 尝试执行Pick
  if (Pick(event.GetUV())) {
    event.SetResult(EventResult::HandledAndStop);
    return;
  }

  event.SetResult(EventResult::Failed);
  return;
}
void SceneView::OnViewportCameraUpdated(ViewportCameraUpdateEvent &event)
{
  SetCameraWorldTransform(event.GetCameraTransform());
  SetCameraZoom(event.GetCameraZoom());

  event.SetResult(EventResult::HandledAndStop);
  return;
}
void SceneView::OnViewportPickedUpdated(ViewportPickedUpdateEvent &event)
{
  SetPickedWorldTransform(event.GetTransform());

  event.SetResult(EventResult::HandledAndStop);
  return;
}
}  // namespace mite