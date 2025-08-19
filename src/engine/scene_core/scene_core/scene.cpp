#include "scene.h"
#include "scene_core_components/component_headers.h"

namespace mite {
Scene::Scene(const std::string &name) : m_Name(name), m_Registry(), m_SystemManager(m_Registry)
{
  // 初始化核心系统
  InitSystems();

  //// TODO: 创建默认环境实体
  // auto env = CreateEntity("Environment");
  // env.AddComponent<EnvironmentComponent>();
}

Scene::~Scene()
{
  Clear(false);
}

void Scene::InitSystems()
{
  // 创建核心系统
  //m_SceneGraph = std::make_unique<SceneGraph>();
  //m_SceneGraph->Initialize(GetRegistry());
  //m_Serializer = std::make_unique<SceneSerializer>(*this);

  // 初始化组件系统
  InitComponentSystems();
}

void Scene::InitComponentSystems()
{
  // 逐个注册组件系统
  m_SystemManager.RegisterSystem<CameraComponentSystem>();
  m_SystemManager.RegisterSystem<DestroyComponentSystem>();
  m_SystemManager.RegisterSystem<IDComponentSystem>();
  m_SystemManager.RegisterSystem<HierarchyComponentSystem>();
  m_SystemManager.RegisterSystem<MaterialComponentSystem>();
  m_SystemManager.RegisterSystem<MeshComponentSystem>();
  m_SystemManager.RegisterSystem<TagComponentSystem>();
  m_SystemManager.RegisterSystem<TransformComponentSystem>();
  m_SystemManager.RegisterSystem<VisibilityComponentSystem>();

  // 初始化所有组件系统
  m_SystemManager.InitializeAll();
}

void Scene::ShutDownComponentSystems()
{
  // 销毁所有组件系统
  m_SystemManager.ShutdownAll();

  // 逐个注销组件系统
  m_SystemManager.UnregisterSystem<CameraComponentSystem>();
  m_SystemManager.UnregisterSystem<DestroyComponentSystem>();
  m_SystemManager.UnregisterSystem<IDComponentSystem>();
  m_SystemManager.UnregisterSystem<HierarchyComponentSystem>();
  m_SystemManager.UnregisterSystem<MaterialComponentSystem>();
  m_SystemManager.UnregisterSystem<MeshComponentSystem>();
  m_SystemManager.UnregisterSystem<TagComponentSystem>();
  m_SystemManager.UnregisterSystem<TransformComponentSystem>();
  m_SystemManager.UnregisterSystem<VisibilityComponentSystem>();
}

void Scene::OnUpdate(float timestep)
{
  // 更新所有注册的系统
  m_SystemManager.UpdateAll(timestep);

  // 场景图更新
  //m_SceneGraph->OnUpdate(timestep);

  // 处理实体销毁队列
  auto entities = m_Registry.GetEntitiesWith<DestroyComponent>();
  for (auto entity : entities) {
    m_Registry.DestroyEntity(entity);
  }
}

void Scene::OnRenderPrepare()
{
  // 准备场景图渲染状态
  //m_SceneGraph->OnRenderPrepare();
}

void Scene::Clear(bool keepSystems)
{
  // 1. 销毁所有实体（不触发单独销毁事件，直接批量清除）
  m_Registry.Clear();

  // 2. 重置实体ID计数器
  m_EntityCounter = 0;

  // 3. TODO: 重置主相机

  // 4. 重置场景图状态
  //if (m_SceneGraph) {
  //  m_SceneGraph->Clear();
  //}

  // 6. 根据参数决定是否重置系统
  // TODO：重置其他系统
  if (!keepSystems) {
    ShutDownComponentSystems();
  }

  // 7. TODO: 系统保留则重新创建默认环境实体（避免与析构函数的Clear冲突）
  if (keepSystems) {
  }
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

std::shared_ptr<Camera> Scene::GetMainCamera() const
{
  // 通过访问Camera组件系统，获取到其维护的主相机实体
  std::optional<Entity> mainCameraEntity =
      m_SystemManager.GetSystem<CameraComponentSystem>()->GetMainCameraEntity();

  // 无主相机情况报错，并返回nullptr
  if (!mainCameraEntity) {
    LOG_ERROR("Invalid Main Camera in CameraComponentSystem!");
    return nullptr;
  }

  // 查询到组件，获取到相机并返回
  std::shared_ptr<Camera> mainCamera =
      m_Registry.GetComponent<CameraComponent>(*mainCameraEntity).GetCamera();
  return mainCamera;
}

void Scene::SetMainCamera(Entity mainCameraEntity)
{
  // 通过访问Camera组件系统，更换其维护的主相机实体
  m_SystemManager.GetSystem<CameraComponentSystem>()->SetMainCameraEntity(mainCameraEntity);
}

//void Scene::Serialize(const std::filesystem::path &filepath)
//{
//  const std::string ext = filepath.extension().string();
//
//  if (ext == ".json") {
//    if (!m_Serializer->SerializeToJson(filepath.string()))
//      throw std::runtime_error("JSON serialization failed: " + m_Serializer->GetLastError());
//  }
//  else if (ext == ".bin") {
//    if (!m_Serializer->SerializeToBinary(filepath.string()))
//      throw std::runtime_error("Binary serialization failed: " + m_Serializer->GetLastError());
//  }
//  else {
//    throw std::runtime_error("Unsupported file extension: " + ext);
//  }
//}
//void Scene::Deserialize(const std::filesystem::path &filepath)
//{
//  if (!m_Serializer) {
//    throw std::runtime_error("Serializer not initialized");
//  }
//
//  if (!std::filesystem::exists(filepath)) {
//    throw std::runtime_error("File not found: " + filepath.string());
//  }
//
//  const std::string ext = filepath.extension().string();
//
//  if (ext == ".json") {
//    if (!m_Serializer->DeserializeFromJson(filepath.string()))
//      throw std::runtime_error("JSON deserialization failed: " + m_Serializer->GetLastError());
//  }
//  else if (ext == ".bin") {
//    if (!m_Serializer->DeserializeFromBinary(filepath.string()))
//      throw std::runtime_error("Binary deserialization failed: " + m_Serializer->GetLastError());
//  }
//  else {
//    throw std::runtime_error("Unsupported file extension: " + ext);
//  }
//}
}  // namespace mite