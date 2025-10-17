#include "scene_core.h"
#include "scene_core_components/component_headers.h"

namespace mite {
SceneCore::SceneCore(const std::string &name)
    : m_Name(name), m_Registry(), m_SystemManager(m_Registry)
{
  // 初始化核心系统
  RegisterComponentSystems();

  //// TODO: 创建默认环境实体
  // auto env = CreateEntity("Environment");
  // env.AddComponent<EnvironmentComponent>();
}

SceneCore::~SceneCore()
{
  Clear(false);
}

void SceneCore::InitializeComponentSystems()
{
  // 初始化所有组件系统
  m_SystemManager.InitializeAll();
}

void SceneCore::ShutdownComponentSystems()
{
  // 关闭所有组件系统
  m_SystemManager.ShutdownAll();
}

void SceneCore::RegisterComponentSystems()
{
  // 逐个注册组件系统
  m_SystemManager.RegisterSystem<CameraComponentSystem>();
  m_SystemManager.RegisterSystem<DestroyComponentSystem>();
  m_SystemManager.RegisterSystem<IDComponentSystem>();
  m_SystemManager.RegisterSystem<LightComponentSystem>();
  m_SystemManager.RegisterSystem<BoundingVolumeComponentSystem>();
  m_SystemManager.RegisterSystem<MaterialComponentSystem>();
  m_SystemManager.RegisterSystem<MeshComponentSystem>();
  m_SystemManager.RegisterSystem<TagComponentSystem>();
  m_SystemManager.RegisterSystem<TransformComponentSystem>();
}

void SceneCore::UnregisterComponentSystems()
{
  // 逐个注销组件系统
  m_SystemManager.UnregisterSystem<CameraComponentSystem>();
  m_SystemManager.UnregisterSystem<DestroyComponentSystem>();
  m_SystemManager.UnregisterSystem<IDComponentSystem>();
  m_SystemManager.UnregisterSystem<BoundingVolumeComponentSystem>();
  m_SystemManager.UnregisterSystem<MaterialComponentSystem>();
  m_SystemManager.UnregisterSystem<MeshComponentSystem>();
  m_SystemManager.UnregisterSystem<TagComponentSystem>();
  m_SystemManager.UnregisterSystem<TransformComponentSystem>();
}

void SceneCore::OnUpdate(float timestep)
{
  // 更新所有注册的带脏标记的组件系统
  m_SystemManager.UpdateDirtyComponentSystems(timestep);

  // 处理实体销毁队列
  auto entities = m_Registry.GetEntitiesWith<DestroyComponent>();
  for (auto entity : entities) {
    m_Registry.DestroyEntity(entity);
  }
}

void SceneCore::OnRenderPrepare()
{
  // 准备场景图渲染状态
  // m_SceneGraph->OnRenderPrepare();
}

void SceneCore::Clear(bool keepSystems)
{
  // 1. 销毁所有实体（不触发单独销毁事件，直接批量清除）
  m_Registry.Clear();

  // 2. 重置实体ID计数器
  m_EntityCounter = 0;

  // 3. TODO: 重置主相机

  // 6. 根据参数决定是否重置系统
  if (!keepSystems) {
    UnregisterComponentSystems();
  }

  // 7. TODO: 系统保留则重新创建默认环境实体（避免与析构函数的Clear冲突）
  if (keepSystems) {
  }
}

Entity SceneCore::CreateEntity(const std::string &name)
{
  auto entity = m_Registry.CreateEntity(name);
  ++m_EntityCounter;

  return entity;
}

void SceneCore::DestroyEntity(Entity entity)
{
  if (!IsValid(entity)) {
    return;
  }

  // 标记销毁而不是立即销毁，避免迭代器失效
  m_Registry.AddComponent<DestroyComponent>(entity);
}

bool SceneCore::IsValid(Entity entity) const
{
  return entity && m_Registry.IsValid(entity);
}
//
//Entity SceneCore::GetMainCamera() const
//{
//  // 通过访问Camera组件系统，获取到其维护的主相机实体
//  Entity mainCameraEntity =
//      m_SystemManager.GetSystem<CameraComponentSystem>()->GetMainCameraEntity();
//
//  // 无主相机情况报错，并返回nullptr
//  if (!mainCameraEntity.IsValid()) {
//    LOG_ERROR("Invalid Main Camera in CameraComponentSystem!");
//    return Entity{};
//  }
//
//  return mainCameraEntity;
//}
//
//void SceneCore::SetMainCamera(Entity mainCameraEntity)
//{
//  // 通过访问Camera组件系统，更换其维护的主相机实体
//  m_SystemManager.GetSystem<CameraComponentSystem>()->SetMainCameraEntity(mainCameraEntity);
//}
}  // namespace mite