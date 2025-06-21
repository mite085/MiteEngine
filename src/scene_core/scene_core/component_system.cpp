#include "component_system.h"
#include "scene_core_components/component_headers.h"

namespace mite {
ComponentSystemManager::ComponentSystemManager(SceneRegistry &registry) : m_Registry(registry)
{  
  // 注册组件事件回调
  //m_Registry.RegisterCallbackComponentConstruct<Component>(
  //    [this](Entity entity, Component &component) { OnComponentAdded(entity, component); });

  //m_Registry.RegisterCallbackComponentUpdate<Component>(
  //    [this](Entity entity, Component &component) { OnComponentUpdated(entity, component); });

  //m_Registry.RegisterCallbackComponentDestroy<Component>(
  //    [this](Entity entity, Component &component) { OnComponentRemoved(entity, component); });
}
ComponentSystemManager::~ComponentSystemManager()
{
  // 确保所有系统都已销毁
  for (auto &entry : m_SystemMap) {
    entry.second.system.reset();
  }
}

void ComponentSystemManager::InitializeAll()
{
  // 确保系统已排序
  if (!m_SystemsSorted) {
    SortSystems();
  }

  // 按顺序初始化
  for (auto &system : m_Systems) {
    system->Initialize(m_Registry);
  }
}

void ComponentSystemManager::UpdateAll(float deltaTime)
{
  // 按顺序更新
  for (auto &system : m_Systems) {
    auto entry = m_SystemMap.find(system->GetSystemType());
    if (entry != m_SystemMap.end() && entry->second.enabled) {
      system->Update(m_Registry, deltaTime);
    }
  }
}

void ComponentSystemManager::ShutdownAll()
{
  // 逆序销毁
  for (auto it = m_Systems.rbegin(); it != m_Systems.rend(); ++it) {
    (*it)->Shutdown(m_Registry);
  }
}

void ComponentSystemManager::SortSystems()
{
  // 准备排序
  m_Systems.clear();
  m_Systems.reserve(m_SystemMap.size());

  // 收集所有系统
  for (auto &entry : m_SystemMap) {
    if (entry.second.enabled) {
      m_Systems.push_back(std::move(entry.second.system));
    }
  }

  // 按执行顺序排序
  std::sort(
      m_Systems.begin(),
      m_Systems.end(),
      [](const std::unique_ptr<ComponentSystem> &a, const std::unique_ptr<ComponentSystem> &b) {
        return a->GetExecutionOrder() < b->GetExecutionOrder();
      });

  // 处理系统依赖
  bool dependenciesResolved = false;
  size_t maxIterations = m_Systems.size() * 2;  // 防止无限循环
  size_t iterations = 0;

  while (!dependenciesResolved && iterations < maxIterations) {
    dependenciesResolved = true;
    iterations++;

    for (size_t i = 0; i < m_Systems.size(); ++i) {
      auto &system = m_Systems[i];
      auto dependencies = system->GetSystemDependencies();

      for (const auto &depType : dependencies) {
        // 查找依赖系统在当前列表中的位置
        auto depIt = std::find_if(m_Systems.begin(),
                                  m_Systems.end(),
                                  [&depType](const std::unique_ptr<ComponentSystem> &s) {
                                    return s->GetSystemType() == depType;
                                  });

        if (depIt != m_Systems.end()) {
          size_t depIndex = std::distance(m_Systems.begin(), depIt);
          if (depIndex > i) {
            // 依赖系统在当前系统之后，需要交换
            std::iter_swap(m_Systems.begin() + i, depIt);
            dependenciesResolved = false;
            break;
          }
        }
      }

      if (!dependenciesResolved)
        break;
    }
  }

  m_SystemsSorted = true;
}

};
