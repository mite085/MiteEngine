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

  m_EventSubscriptions.Subscribe<ComponentAddedEvent<HierarchyComponent>>(
      BIND_DISPATCH_FN(OnHierarchyComponentAdded));
  m_EventSubscriptions.Subscribe<ComponentRemovedEvent<HierarchyComponent>>(
      BIND_DISPATCH_FN(OnHierarchyComponentRemoved));
  m_EventSubscriptions.Subscribe<ParentChangedEvent>(BIND_DISPATCH_FN(OnParentChanged));

  m_Logger->debug("HierarchySceneNodeSystem initialized");
}

void HierarchySceneNodeSystem::Update(float deltaTime, SceneRegistry &registry)
{
  ProcessPendingHierarchyChanges(registry);
}

void HierarchySceneNodeSystem::Shutdown()
{
  m_Logger->info("Shutting down HierarchySceneNodeSystem");
  m_EventSubscriptions.UnsubscribeAll();
  m_PendingHierarchyChanges.clear();
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
  m_SceneGraph = sceneGraph;
}

bool HierarchySceneNodeSystem::OnHierarchyComponentAdded(
    ComponentAddedEvent<HierarchyComponent> &e)
{
  m_PendingHierarchyChanges.push_back(e.GetEntity());
  e.Handled();
  return true;
}

bool HierarchySceneNodeSystem::OnHierarchyComponentRemoved(
    ComponentRemovedEvent<HierarchyComponent> &e)
{
  // 移除层级组件时，将节点设为根节点
  if (m_SceneGraph) {
    SceneNode *node = m_SceneGraph->GetNode(e.GetEntity());
    if (node) {
      m_SceneGraph->SetParent(node, nullptr);
    }
  }
  e.Handled();
  return true;
}

bool HierarchySceneNodeSystem::OnParentChanged(ParentChangedEvent &e)
{
  m_PendingHierarchyChanges.push_back(e.GetEntity());
  e.Handled();
  return true;
}

void HierarchySceneNodeSystem::UpdateSceneNodeParent(SceneRegistry &registry, Entity entity)
{
  if (!m_SceneGraph || !m_SceneGraph->HasNode(entity)) {
    return;
  }

  SceneNode *node = m_SceneGraph->GetNode(entity);
  SceneNode *parentNode = nullptr;

  if (registry.HasComponent<HierarchyComponent>(entity)) {
    auto &hierarchy = registry.GetComponent<HierarchyComponent>(entity);
    if (hierarchy.GetParent().IsValid() && m_SceneGraph->HasNode(hierarchy.GetParent())) {
      parentNode = m_SceneGraph->GetNode(hierarchy.GetParent());
    }
  }

  m_SceneGraph->SetParent(node, parentNode);
  m_Logger->debug("Updated parent for entity {}", entity.GetUUIDString());
}

void HierarchySceneNodeSystem::ProcessPendingHierarchyChanges(SceneRegistry &registry)
{
  if (m_PendingHierarchyChanges.empty()) {
    return;
  }

  std::vector<Entity> processingEntities;
  processingEntities.swap(m_PendingHierarchyChanges);

  for (Entity entity : processingEntities) {
    if (registry.IsValid(entity)) {
      UpdateSceneNodeParent(registry, entity);
    }
  }
}

}  // namespace mite
