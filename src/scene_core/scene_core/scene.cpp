#include "scene.h"
#include "scene_graph.h"
#include "scene_observer.h"
#include "scene_serializer.h"
#include "scene_core_components/component_headers.h"

namespace mite {
Scene::Scene(const std::string &name)
    : m_Name(name), m_Registry(weak_from_this()), m_SystemManager(m_Registry)
{
  // 初始化核心系统
  InitSystems();

  //// TODO: 创建默认环境实体
  // auto env = CreateEntity("Environment");
  // env.AddComponent<EnvironmentComponent>();

  m_MainCamera = Entity(weak_from_this(), entt::null);
}

Scene::~Scene()
{
  // 确保所有系统按正确顺序销毁
  m_SystemManager.ShutdownAll();
}

void Scene::InitSystems()
{
  // 注册核心系统
  m_SceneGraph = std::make_unique<SceneGraph>(GetRegistry());
  m_SceneObserver = std::make_unique<SceneObserver>(GetRegistry());
  m_Serializer = std::make_unique<SceneSerializer>(*this);

  // 注册组件系统
  RegisterComponentSystems();
}

void Scene::RegisterComponentSystems()
{  
  // TODO: 注册核心系统（按执行顺序）
  m_SystemManager.RegisterSystem<TransformSystem>();
  //m_SystemManager.RegisterSystem<id>();
  //m_SystemManager.RegisterSystem<RenderSystem>();

}

void Scene::OnUpdate(float timestep)
{
  // 开始记录本帧的变更
  m_SceneObserver->BeginObservation();

  // 更新所有注册的系统
  m_SystemManager.UpdateAll(timestep);

  // 场景图更新
  m_SceneGraph->OnUpdate(timestep);

  // 处理实体销毁队列
  auto entities = m_Registry.GetEntitiesWith<DestroyComponent>();
  for (auto entity : entities) {
    m_Registry.DestroyEntity(entity);
  }

  // 结束观察并分发事件
  EventDispatcher dispatcher;
  m_SceneObserver->EndObservationAndEmitEvents(dispatcher);
}

void Scene::OnRenderPrepare()
{
  // 准备场景图渲染状态
  m_SceneGraph->OnRenderPrepare();
}

void Scene::Clear(bool keepSystems)
{
  // 1. 销毁所有实体（不触发单独销毁事件，直接批量清除）
  m_Registry.Clear();

  // 2. 重置实体ID计数器
  m_EntityCounter = 0;

  // 3. 重置主相机引用
  m_MainCamera = Entity(weak_from_this(), entt::null);

  // 4. 重置场景图状态
  if (m_SceneGraph) {
    m_SceneGraph->Clear();
  }

  // 5. 重置场景观察者状态
  if (m_SceneObserver) {
    m_SceneObserver->Clear();
  }

  // 6. 根据参数决定是否重置系统
  if (!keepSystems) {
    // 销毁所有系统
    m_SystemManager.ShutdownAll();
  }

  // 7. TODO: 重新创建默认环境实体(是否存在重新创建的必要？)
}

Entity Scene::CreateEntity(const std::string &name)
{
  auto entity = m_Registry.CreateEntity(name);
  ++m_EntityCounter;

  return entity;
}

void Scene::DestroyEntity(Entity entity)
{
  if (!IsValid(entity)) {
    return;
  }

  // 标记销毁而不是立即销毁，避免迭代器失效
  m_Registry.AddComponent<DestroyComponent>(entity);
}

bool Scene::IsValid(Entity entity) const
{
  return entity && m_Registry.IsValid(entity);
}

void Scene::Serialize(const std::filesystem::path &filepath)
{
  const std::string ext = filepath.extension().string();

  if (ext == ".json") {
    if (!m_Serializer->SerializeToJson(filepath.string()))
      throw std::runtime_error("JSON serialization failed: " + m_Serializer->GetLastError());
  }
  else if (ext == ".bin") {
    if (!m_Serializer->SerializeToBinary(filepath.string()))
      throw std::runtime_error("Binary serialization failed: " + m_Serializer->GetLastError());
  }
  else {
    throw std::runtime_error("Unsupported file extension: " + ext);
  }
}
  void Scene::Deserialize(const std::filesystem::path &filepath)
  {
    if (!m_Serializer) {
      throw std::runtime_error("Serializer not initialized");
    }

    if (!std::filesystem::exists(filepath)) {
      throw std::runtime_error("File not found: " + filepath.string());
    }

    const std::string ext = filepath.extension().string();

    if (ext == ".json") {
      if (!m_Serializer->DeserializeFromJson(filepath.string()))
        throw std::runtime_error("JSON deserialization failed: " + m_Serializer->GetLastError());
    }
    else if (ext == ".bin") {
      if (!m_Serializer->DeserializeFromBinary(filepath.string()))
        throw std::runtime_error("Binary deserialization failed: " + m_Serializer->GetLastError());
    }
    else {
      throw std::runtime_error("Unsupported file extension: " + ext);
    }
  }
}  // namespace mite