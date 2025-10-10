#include "scene_view.h"
#include "basic_event/instance_event.h"
#include "subscription_group.h"
#include "timer/timer.h"

namespace mite {
SceneView::SceneView()
    : m_Builder(std::make_unique<RenderableItemBuilder>()),
      m_RenderQueue(std::make_shared<RenderQueue>()),
      m_CameraInstance(nullptr),
      m_LastVisibleNodeCount(0),
      m_LastRenderItemCount(0),
      m_LastUpdateTime(0.0f)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneView");
  // 初始化日志
  m_Logger->debug("SceneView initialized");
}
SceneView::~SceneView()
{
  m_Logger->debug("SceneView destroyed");
}
void SceneView::SetCamera(const std::shared_ptr<Camera> &camera)
{
  if (camera) {
    m_CameraInstance = std::make_shared<CameraInstance>(camera);
    m_CameraInstance->InitializeUBO();

    // 初始化UBO后发布事件委托RenderContext注册并执行着色器绑定
    EventBus::Publish<CameraInstanceCreateEvent>(CameraInstanceCreateEvent(m_CameraInstance));

    m_Logger->debug("SceneView camera instance set");
  }
  else {
    m_Logger->warn("SceneView received null camera instance");
  }
}
void SceneView::Update(SceneRegistry &registry,Transform cameraTransform, std::vector<SceneNode *> visibleNodes)
{
  // 计时，确定构建RenderQueue所消耗的时间
  Timer timer;

  // 更新相机实例
  m_CameraInstance->UpdateUBO(cameraTransform);

  // 处理可见节点
  ProcessVisibility(registry, visibleNodes);
  m_LastUpdateTime = timer.ElapsedMillis();

  // m_Logger->debug("SceneView updated in {:.3f}ms, visible nodes: {}, render items: {}",
  //                m_lastUpdateTime,
  //                m_lastVisibleNodeCount,
  //                m_lastRenderItemCount);
}

std::shared_ptr<RenderQueue> SceneView::GetRenderQueue() const
{
  return m_RenderQueue;
}

size_t SceneView::GetVisibleNodeCount() const
{
  return m_LastVisibleNodeCount;
}
size_t SceneView::GetRenderItemCount() const
{
  return m_LastRenderItemCount;
}
float SceneView::GetLastUpdateTime() const
{
  return m_LastUpdateTime;
}
void SceneView::ProcessVisibility(SceneRegistry &registry, std::vector<SceneNode *> visibleNodes)
{
  m_LastVisibleNodeCount = visibleNodes.size();

  // 1. 构建渲染项（使用相机实例辅助选择LOD）
  std::vector<RenderableItem> renderItems = m_Builder->BuildFromSceneNodes(
      registry, m_CameraInstance, visibleNodes);

  // 2. 更新计数
  m_LastRenderItemCount = renderItems.size();

  // 3. 更新渲染队列
  m_RenderQueue->ClearAll();
  m_RenderQueue->AddItems(renderItems);
  m_RenderQueue->SortAll();
}
}  // namespace mite