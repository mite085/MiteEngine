#include "component_system_manager.h"

namespace mite {
ComponentSystemManager::ComponentSystemManager(SceneRegistry &registry)
    : m_Registry(registry)
{
}
ComponentSystemManager::~ComponentSystemManager()
{
  // 确保所有系统都已销毁
  for (auto &entry : m_SystemMap) {
    entry.second->Shutdown();
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
    system->Initialize();
  }
}

void ComponentSystemManager::UpdateDirtyComponentSystems(float deltaTime)
{
  // 按顺序更新脏标记组件系统
  for (auto &system : m_Systems) {
    system->Update(deltaTime, m_Registry);
  }
}

void ComponentSystemManager::ClearAll() 
{
  // 逆序清理
  for (auto it = m_Systems.rbegin(); it != m_Systems.rend(); ++it) {
    (*it)->Clear();
  }
}

void ComponentSystemManager::ShutdownAll()
{
  // 逆序销毁
  for (auto it = m_Systems.rbegin(); it != m_Systems.rend(); ++it) {
    (*it)->Shutdown();
  }
}

void ComponentSystemManager::SortSystems()
{
  // 1. 先按执行顺序排序（初步排序）
  std::sort(m_Systems.begin(), m_Systems.end(), [](const auto &a, const auto &b) {
    return a->GetExecutionOrder() < b->GetExecutionOrder();
  });

  // 2. 拓扑排序调整依赖关系
  bool changed;
  size_t iterations = 0;
  const size_t maxIterations = m_Systems.size();  // 防止无限循环

  do {
    changed = false;
    for (size_t i = 0; i < m_Systems.size(); ++i) {
      auto &system = m_Systems[i];
      for (const auto &depType : system->GetSystemDependencies()) {
        // 查找依赖的系统
        auto depIt = std::find_if(m_Systems.begin(), m_Systems.end(), [&depType](const auto &s) {
          return std::type_index(typeid(*s)) == depType;
        });

        // 如果依赖的系统在当前系统之后，调整顺序
        if (depIt != m_Systems.end() && depIt > m_Systems.begin() + i) {
          // 把当前系统移到依赖系统之后
          std::rotate(m_Systems.begin() + i, m_Systems.begin() + i + 1, depIt + 1);
          changed = true;
          --i;  // 重新检查当前位置
          break;
        }
      }
    }
  } while (changed && ++iterations < maxIterations);

  // 3. 重建类型映射
  m_SystemMap.clear();
  for (auto &system : m_Systems) {
    m_SystemMap[typeid(*system)] = system.get();
  }

  m_SystemsSorted = true;
}
};
