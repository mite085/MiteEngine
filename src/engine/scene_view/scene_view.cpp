#include "scene_view.h"
#include "timer/timer.h"

namespace mite {
SceneView::SceneView()
    : m_Builder(std::make_unique<RenderableItemBuilder>()),
      m_RenderQueue(std::make_shared<RenderQueue>()),
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
void SceneView::Update(SceneRegistry &registry, std::vector<SceneNode *> visibleNodes)
{
  // 计时，确定构建RenderQueue所消耗的时间
  Timer timer;
  ProcessVisibility(registry, visibleNodes);
  m_LastUpdateTime = timer.ElapsedMillis();

  //m_Logger->debug("SceneView updated in {:.3f}ms, visible nodes: {}, render items: {}",
  //               m_lastUpdateTime,
  //               m_lastVisibleNodeCount,
  //               m_lastRenderItemCount);
}
void SceneView::Rebuild(SceneRegistry &registry,
                        std::vector<SceneNode *> visibleNodes)
{
  Update(registry, visibleNodes);  // 当前实现与Update相同
}
std::shared_ptr<RenderQueue> SceneView::GetRenderQueue() const
{
  return m_RenderQueue;
}

void SceneView::SetCustomFilter(std::function<bool(SceneNode *)> filterFunc)
{
  m_CustomFilterFunc = filterFunc;
  m_Logger->debug("SceneView custom filter set");
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

  // 1. 应用自定义过滤器（如果设置）
  if (m_CustomFilterFunc) {
    visibleNodes = ApplyCustomFilter(visibleNodes);
  }

  // 2. 构建渲染项
  std::vector<RenderableItem> renderItems = m_Builder->BuildFromSceneNodes(registry, visibleNodes);

  m_LastRenderItemCount = renderItems.size();

  // 3. 更新渲染队列
  m_RenderQueue->ClearAll();
  m_RenderQueue->AddItems(renderItems);
  m_RenderQueue->SortAll();
}
std::vector<SceneNode *> SceneView::ApplyCustomFilter(const std::vector<SceneNode *> &nodes)
{
  std::vector<SceneNode *> filteredNodes;
  filteredNodes.reserve(nodes.size());

  for (SceneNode *node : nodes) {
    if (m_CustomFilterFunc(node)) {
      filteredNodes.push_back(node);
    }
  }

  return filteredNodes;
}

}  // namespace mite