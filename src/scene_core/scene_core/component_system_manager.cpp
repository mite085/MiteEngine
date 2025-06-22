#include "component_system_manager.h"

namespace mite {
ComponentSystemManager::ComponentSystemManager(SceneRegistry &registry) : m_Registry(registry)
{
}
ComponentSystemManager::~ComponentSystemManager()
{
  // 确保所有系统都已销毁
  for (auto &entry : m_SystemMap) {
    entry.second->Shutdown(m_Registry);
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
    if (entry != m_SystemMap.end()) {
      system->Update(deltaTime, m_Registry);
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
  // 1. 清空并保留容量（如果m_Systems已经包含所有系统）
  m_Systems.clear();
  m_Systems.reserve(m_SystemMap.size());

  // 2. 转移所有权（假设m_SystemMap中的指针是m_Systems中对象的非拥有指针）
  // 注意：这里需要确保m_SystemMap中的指针确实来自m_Systems
  for (auto &entry : m_SystemMap) {
    // 查找对应的unique_ptr（需要额外维护反向映射）
    auto it = std::find_if(m_Systems.begin(), m_Systems.end(), [&](const auto &ptr) {
      return ptr.get() == entry.second;
    });

    if (it != m_Systems.end()) {
      m_Systems.push_back(std::move(*it));
    }
  }

  // 3. 按执行顺序排序
  std::sort(
      m_Systems.begin(),
      m_Systems.end(),
      [](const std::unique_ptr<ComponentSystem> &a, const std::unique_ptr<ComponentSystem> &b) {
        return a->GetExecutionOrder() < b->GetExecutionOrder();
      });

  // 4. 重建类型映射（因为指针可能因排序而改变）
  m_SystemMap.clear();
  for (auto &system : m_Systems) {
    m_SystemMap[typeid(*system)] = system.get();
  }

  // 5. 处理系统依赖
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
