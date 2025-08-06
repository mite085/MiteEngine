#include "scene_view.h"

namespace mite {
SceneView::SceneView(SceneRegistry &registry) : m_Registry(registry)
{
  // 订阅SceneCore发出的四类核心事件
  m_EventSubscriptions.Subscribe<EntityCreatedEvent>(BIND_DISPATCH_FN(OnEntityCreated));
  m_EventSubscriptions.Subscribe<EntityDestroyedEvent>(BIND_DISPATCH_FN(OnEntityDestroyed));
  m_EventSubscriptions.Subscribe<TransformChangedEvent>(BIND_DISPATCH_FN(OnTransformChanged));
  m_EventSubscriptions.Subscribe<MaterialChangedEvent>(BIND_DISPATCH_FN(OnMaterialChanged));
}

SceneView::~SceneView()
{
  // 析构时自动通过SubscriptionGroup取消所有事件订阅
}

void SceneView::Update()
{
  // 当前最小化实现无需每帧主动处理数据（完全事件驱动）
  // 未来可在此添加帧级批量优化（如脏标记合并）
}

const std::vector<RenderableEntity> &SceneView::GetRenderQueue() const
{
  return m_RenderQueue;
}

//=== 私有事件处理函数 ===//
void SceneView::OnEntityCreated(EntityCreatedEvent &event)
{
  Entity entity = event.GetEntity();
}

void SceneView::OnEntityDestroyed(EntityDestroyedEvent &event)
{
  RemoveFromRenderQueue(event.GetEntity());
}

void SceneView::OnTransformChanged(TransformChangedEvent &event)
{
  auto it = m_EntityToIndexMap.find(event.GetEntity());
  if (it != m_EntityToIndexMap.end()) {
    // 更新现有渲染实体的变换矩阵
    m_RenderQueue[it->second].worldTransform =
        m_Registry.GetComponent<TransformComponent>(event.GetEntity()).GetWorldMatrix(m_Registry);
  }
}

void SceneView::OnMaterialChanged(MaterialChangedEvent &event)
{
  auto it = m_EntityToIndexMap.find(event.GetEntity());
  if (it != m_EntityToIndexMap.end()) {
    // 更新现有渲染实体的材质引用
    m_RenderQueue[it->second].materialInstance =
        m_Registry.GetComponent<MaterialComponent>(event.GetEntity()).GetMaterial();
  }
}

void SceneView::AddToRenderQueue(Entity entity)
{
  // 防止重复添加
  if (m_EntityToIndexMap.count(entity) > 0)
    return;

  // 只有同时拥有Transform、Mesh、Material的实体才加入渲染队列
  if (m_Registry.HasComponentWithAllOf<TransformComponent, MeshComponent, MaterialComponent>(
          entity))
  {

    // 构造RenderableEntity
    RenderableEntity renderable;
    renderable.entity = entity;
    renderable.worldTransform = m_Registry.GetComponent<TransformComponent>(entity).GetWorldMatrix(
        m_Registry);
    renderable.meshHandle = m_Registry.GetComponent<MeshComponent>(entity).GetMesh()->GetHandle();
    renderable.materialInstance = m_Registry.GetComponent<MaterialComponent>(entity).GetMaterial();

    // 加入队列并记录索引
    m_EntityToIndexMap[entity] = m_RenderQueue.size();
    m_RenderQueue.push_back(std::move(renderable));
  }
}

void SceneView::RemoveFromRenderQueue(Entity entity)
{
  auto it = m_EntityToIndexMap.find(entity);
  if (it == m_EntityToIndexMap.end())
    return;

  // 将末尾元素移动到被删除元素的位置（保持内存连续）
  size_t index = it->second;
  if (index != m_RenderQueue.size() - 1) {
    m_RenderQueue[index] = std::move(m_RenderQueue.back());
    m_EntityToIndexMap[m_RenderQueue.back().entity] = index;
  }

  // 移除末尾元素
  m_RenderQueue.pop_back();
  m_EntityToIndexMap.erase(entity);
}

//=== 私有工具函数 ===//

void SceneView::UpdateRenderableEntity(Entity entity)
{
  auto it = m_EntityToIndexMap.find(entity);
  if (it == m_EntityToIndexMap.end())
    return;

  // 从队列中获取到eitity，进行综合更新（供未来扩展使用）
  RenderableEntity &renderable = m_RenderQueue[it->second];
  renderable.worldTransform = m_Registry.GetComponent<TransformComponent>(entity).GetWorldMatrix(
      m_Registry);
  renderable.materialInstance = m_Registry.GetComponent<MaterialComponent>(entity).GetMaterial();
}
}  // namespace mite