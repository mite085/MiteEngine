#include "hierarchy_scene_node_system.h"
#include "scene_core/scene_registry.h"

namespace mite {

HierarchySceneNodeSystem::HierarchySceneNodeSystem()
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite HierarchySceneNodeSystem");
  m_Logger->trace("HierarchySceneNodeSystem created");
}

Component::Family HierarchySceneNodeSystem::GetExecutionOrder() const
{
  return Component::Family::Hierarchy;
}

void HierarchySceneNodeSystem::Initialize()
{
  m_Logger->info("Initializing HierarchySceneNodeSystem");

  m_eventSubscriptions.Subscribe<ComponentAddedEvent<HierarchyComponent>>(
      BIND_DISPATCH_FN(OnHierarchyComponentAdded));
  m_eventSubscriptions.Subscribe<ComponentRemovedEvent<HierarchyComponent>>(
      BIND_DISPATCH_FN(OnHierarchyComponentRemoved));
  m_eventSubscriptions.Subscribe<ParentChangedEvent>(BIND_DISPATCH_FN(OnParentChanged));

  m_Logger->debug("HierarchySceneNodeSystem initialized");
}

void HierarchySceneNodeSystem::Update(float deltaTime, SceneRegistry &registry)
{
  ProcessPendingHierarchyChanges(registry);
}

void HierarchySceneNodeSystem::Shutdown()
{
  m_Logger->info("Shutting down HierarchySceneNodeSystem");
  m_eventSubscriptions.UnsubscribeAll();
  m_pendingHierarchyChanges.clear();
}

std::vector<std::type_index> HierarchySceneNodeSystem::GetComponentTypes() const
{
  return {typeid(HierarchyComponent)};
}

std::vector<std::type_index> HierarchySceneNodeSystem::GetSystemDependencies() const
{
  return {typeid(HierarchyComponentSystem)};
}

void HierarchySceneNodeSystem::SetSceneGraph(SceneGraph *sceneGraph)
{
  m_sceneGraph = sceneGraph;
}

bool HierarchySceneNodeSystem::OnHierarchyComponentAdded(
    ComponentAddedEvent<HierarchyComponent> &e)
{
  m_pendingHierarchyChanges.push_back(e.GetEntity());
  e.Handled();
  return true;
}

bool HierarchySceneNodeSystem::OnHierarchyComponentRemoved(
    ComponentRemovedEvent<HierarchyComponent> &e)
{
  // 移除层级组件时，将节点设为根节点
  if (m_sceneGraph) {
    SceneNode *node = m_sceneGraph->GetNode(e.GetEntity());
    if (node) {
      m_sceneGraph->SetParent(node, nullptr);
    }
  }
  e.Handled();
  return true;
}

bool HierarchySceneNodeSystem::OnParentChanged(ParentChangedEvent &e)
{
  m_pendingHierarchyChanges.push_back(e.GetEntity());
  e.Handled();
  return true;
}

void HierarchySceneNodeSystem::UpdateSceneNodeParent(SceneRegistry &registry, Entity entity)
{
  if (!m_sceneGraph || !m_sceneGraph->HasNode(entity)) {
    return;
  }

  SceneNode *node = m_sceneGraph->GetNode(entity);
  SceneNode *parentNode = nullptr;

  if (registry.HasComponent<HierarchyComponent>(entity)) {
    auto &hierarchy = registry.GetComponent<HierarchyComponent>(entity);
    if (hierarchy.GetParent().IsValid() && m_sceneGraph->HasNode(hierarchy.GetParent())) {
      parentNode = m_sceneGraph->GetNode(hierarchy.GetParent());
    }
  }

  m_sceneGraph->SetParent(node, parentNode);
  m_Logger->debug("Updated parent for entity {}", entity.GetUUIDString());
}

void HierarchySceneNodeSystem::ProcessPendingHierarchyChanges(SceneRegistry &registry)
{
  if (m_pendingHierarchyChanges.empty()) {
    return;
  }

  std::vector<Entity> processingEntities;
  processingEntities.swap(m_pendingHierarchyChanges);

  for (Entity entity : processingEntities) {
    if (registry.IsValid(entity)) {
      UpdateSceneNodeParent(registry, entity);
    }
  }
}

}  // namespace mite
