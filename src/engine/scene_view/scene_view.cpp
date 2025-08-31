#include "scene_view.h"

namespace mite {
SceneView::SceneView(SceneGraph *sceneGraph, SceneRegistry &registry)
    : m_sceneGraph(sceneGraph),
      m_builder(std::make_unique<RenderableItemBuilder>(registry)),
      m_renderQueue(std::make_shared<RenderQueue>()),
      m_visibilityMask(0xFFFFFFFF)  // 默认所有位都可见
      ,
      m_lastVisibleNodeCount(0),
      m_lastRenderItemCount(0),
      m_lastUpdateTime(0.0f)
{
  m_logger = mite::LoggerSystem::CreateModuleLogger("Mite SceneView");
  // 初始化日志
  m_logger->debug("SceneView initialized");
}
SceneView::~SceneView()
{
  m_logger->debug("SceneView destroyed");
}
void SceneView::Update(Camera *camera)
{
  if (!camera || !m_sceneGraph) {
    m_logger->warn("SceneView update called with null camera or sceneGraph");
    return;
  }

  Timer timer;
  ProcessVisibility(camera);
  m_lastUpdateTime = timer.ElapsedMillis();

  m_logger->debug("SceneView updated in {:.3f}ms, visible nodes: {}, render items: {}",
                 m_lastUpdateTime,
                 m_lastVisibleNodeCount,
                 m_lastRenderItemCount);
}
void SceneView::Rebuild(Camera *camera)
{
  Update(camera);  // 当前实现与Update相同
}
std::shared_ptr<RenderQueue> SceneView::GetRenderQueue() const
{
  return m_renderQueue;
}
void SceneView::SetVisibilityMask(uint32_t mask)
{
  m_visibilityMask = mask;
  m_logger->debug("SceneView visibility mask set to: 0x{:08X}", mask);
}
uint32_t SceneView::GetVisibilityMask() const
{
  return m_visibilityMask;
}
void SceneView::SetCustomFilter(std::function<bool(SceneNode *)> filterFunc)
{
  m_customFilterFunc = filterFunc;
  m_logger->debug("SceneView custom filter set");
}
size_t SceneView::GetVisibleNodeCount() const
{
  return m_lastVisibleNodeCount;
}
size_t SceneView::GetRenderItemCount() const
{
  return m_lastRenderItemCount;
}
float SceneView::GetLastUpdateTime() const
{
  return m_lastUpdateTime;
}
void SceneView::ProcessVisibility(Camera *camera)
{
  // 1. 执行视锥体剔除和可见性查询
  std::vector<SceneNode *> visibleNodes = m_sceneGraph->QueryVisibleNodes(
      *camera->GetSceneRegistry(), camera->GetFrustum(), m_visibilityMask);

  m_lastVisibleNodeCount = visibleNodes.size();

  // 2. 应用自定义过滤器（如果设置）
  if (m_customFilterFunc) {
    visibleNodes = ApplyCustomFilter(visibleNodes);
  }

  // 3. 构建渲染项
  std::vector<RenderableItem> renderItems = m_builder->BuildFromSceneNodes(visibleNodes);

  m_lastRenderItemCount = renderItems.size();

  // 4. 更新渲染队列
  m_renderQueue->ClearAll();
  m_renderQueue->AddItems(renderItems);
  m_renderQueue->SortAll();
}
std::vector<SceneNode *> SceneView::ApplyCustomFilter(const std::vector<SceneNode *> &nodes)
{
  std::vector<SceneNode *> filteredNodes;
  filteredNodes.reserve(nodes.size());

  for (SceneNode *node : nodes) {
    if (m_customFilterFunc(node)) {
      filteredNodes.push_back(node);
    }
  }

  return filteredNodes;
}


//SceneView::SceneView(SceneRegistry &registry) : m_Registry(registry)
//{
//  // 订阅SceneCore发出的四类核心事件
//  m_EventSubscriptions.Subscribe<EntityCreatedEvent>(BIND_DISPATCH_FN(OnEntityCreated));
//  m_EventSubscriptions.Subscribe<EntityDestroyedEvent>(BIND_DISPATCH_FN(OnEntityDestroyed));
//  m_EventSubscriptions.Subscribe<TransformChangedEvent>(BIND_DISPATCH_FN(OnTransformChanged));
//  m_EventSubscriptions.Subscribe<MaterialChangedEvent>(BIND_DISPATCH_FN(OnMaterialChanged));
//}
//
//SceneView::~SceneView()
//{
//  // 析构时自动通过SubscriptionGroup取消所有事件订阅
//}
//
//void SceneView::Update()
//{
//  // 每帧处理PendingEntities列表，
//  // 将当前帧（和之前帧）新创建的实体加入到渲染队列
//  // 
//  // 注意：
//  // 此处没有主动清理未能加入渲染队列的Entity，
//  // 是因为遍历成本不高，且考虑到异步加载
//  // Mesh和Material未必能在当前帧内完成，
//  // 甚至无法给出几帧之后移除的判定依据。
//  for (auto it = m_PendingEntities.begin(); it != m_PendingEntities.end();) {
//    if (AddToRenderQueue(*it)) {
//      it = m_PendingEntities.erase(it);  // 成功加入队列后移除
//    }
//    else {
//      ++it;
//    }
//  }
//}
//
//const std::vector<std::shared_ptr<RenderableItem>> &SceneView::GetRenderQueue() const
//{
//  return m_RenderQueue;
//}
//
////=== 私有事件处理函数 ===//
//bool SceneView::OnEntityCreated(EntityCreatedEvent &event)
//{
//  // 加入延迟处理列表
//  m_PendingEntities.insert(event.GetEntity());
//
//  // 不应当标记事件已处理
//  return event.handled;
//}
//
//bool SceneView::OnEntityDestroyed(EntityDestroyedEvent &event)
//{
//  RemoveFromRenderQueue(event.GetEntity());
//
//  // 维护延迟处理列表
//  if (m_PendingEntities.find(event.GetEntity()) != m_PendingEntities.end()) {
//    m_PendingEntities.erase(m_PendingEntities.find(event.GetEntity()));
//  }
//
//  // 不应当标记事件已处理
//  return event.handled;
//}
//
//bool SceneView::OnTransformChanged(TransformChangedEvent &event)
//{
//  auto it = m_EntityToIndexMap.find(event.GetEntity());
//  if (it != m_EntityToIndexMap.end()) {
//    // 更新现有渲染实体的变换矩阵
//    m_RenderQueue[it->second]->worldTransform =
//        m_Registry.GetComponent<TransformComponent>(event.GetEntity()).GetWorldMatrix(m_Registry);
//  }
//
//  // 不应当标记事件已处理
//  return event.handled;
//}
//
//bool SceneView::OnMaterialChanged(MaterialChangedEvent &event)
//{
//  auto it = m_EntityToIndexMap.find(event.GetEntity());
//  if (it != m_EntityToIndexMap.end()) {
//    // 更新现有渲染实体的材质引用
//    m_RenderQueue[it->second]->material =
//        m_Registry.GetComponent<MaterialComponent>(event.GetEntity()).GetMaterial();
//  }
//
//  // 不应当标记事件已处理
//  return event.handled;
//}
//
//bool SceneView::AddToRenderQueue(Entity entity)
//{
//  // 防止重复添加
//  if (m_EntityToIndexMap.count(entity) > 0)
//    return true;
//
//  // 只有同时拥有Transform、Mesh、Material的实体才加入渲染队列
//  if (m_Registry.HasComponentWithAllOf<TransformComponent, MeshComponent, MaterialComponent>(
//          entity))
//  {
//
//    // 构造RenderableEntity
//    std::shared_ptr<RenderableItem> renderable = std::make_shared<RenderableItem>();
//    renderable->entity = entity;
//    renderable->worldTransform =
//        m_Registry.GetComponent<TransformComponent>(entity).GetWorldMatrix(
//        m_Registry);
//    renderable->mesh = m_Registry.GetComponent<MeshComponent>(entity).GetMesh();
//    renderable->material =
//        m_Registry.GetComponent<MaterialComponent>(entity).GetMaterial();
//
//    // 加入队列并记录索引
//    m_EntityToIndexMap[entity] = m_RenderQueue.size();
//    m_RenderQueue.push_back(std::move(renderable));
//
//    return true;
//  }
//  else {
//    return false;
//  }
//}
//
//void SceneView::RemoveFromRenderQueue(Entity entity)
//{
//  auto it = m_EntityToIndexMap.find(entity);
//  if (it == m_EntityToIndexMap.end())
//    return;
//
//  // 将末尾元素移动到被删除元素的位置（保持内存连续）
//  size_t index = it->second;
//  if (index != m_RenderQueue.size() - 1) {
//    m_RenderQueue[index] = std::move(m_RenderQueue.back());
//    m_EntityToIndexMap[m_RenderQueue.back()->entity] = index;
//  }
//
//  // 移除末尾元素
//  m_RenderQueue.pop_back();
//  m_EntityToIndexMap.erase(entity);
//}
//
////=== 私有工具函数 ===//
//
//void SceneView::UpdateRenderableEntity(Entity entity)
//{
//  auto it = m_EntityToIndexMap.find(entity);
//  if (it == m_EntityToIndexMap.end())
//    return;
//
//  // 从队列中获取到eitity，进行综合更新（供未来扩展使用）
//  std::shared_ptr<RenderableItem> &renderable = m_RenderQueue[it->second];
//  renderable->worldTransform = m_Registry.GetComponent<TransformComponent>(entity).GetWorldMatrix(
//      m_Registry);
//  renderable->material = m_Registry.GetComponent<MaterialComponent>(entity).GetMaterial();
//}
}  // namespace mite