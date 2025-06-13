#include "scene.h"
#include "scene_graph.h"
#include "scene_observer.h"
#include "scene_serializer.h"
namespace mite {
Scene::Scene(const std::string &name) : m_Name(name)
{
  // 初始化核心系统
  InitSystems();

  //// TODO: 创建默认环境实体
  // auto env = CreateEntity("Environment");
  // env.AddComponent<EnvironmentComponent>();

  m_MainCamera = std::make_shared<Entity>(weak_from_this(), entt::null);
}

Scene::~Scene()
{
  // 确保所有系统按正确顺序销毁
  m_Systems.clear();
}

void Scene::InitSystems()
{
  // 注册核心系统
  m_SceneGraph = std::make_unique<SceneGraph>(this);
  m_SceneObserver = std::make_unique<SceneObserver>(this);
  m_Serializer = std::make_unique<SceneSerializer>(this);

  //// TODO: 注册变换系统
  // RegisterSystem<TransformSystem>();
}

void Scene::OnUpdate(float timestep)
{
  // 更新所有注册的系统
  for (auto &[_, system] : m_Systems) {
    system->OnUpdate(timestep);
  }

  // 场景图更新
  m_SceneGraph->OnUpdate(timestep);

  // 处理实体销毁队列
  m_Registry.view<DestroyComponent>().each(
      [this](auto entity, auto &) { m_Registry.destroy(entity); });
}

void Scene::OnRenderPrepare()
{
  // 更新场景观察者
  m_SceneObserver->OnUpdate();

  // 准备场景图渲染状态
  m_SceneGraph->OnRenderPrepare();
}

void Scene::Clear(bool keepSystems)
{
  // 1. 销毁所有实体（不触发单独销毁事件，直接批量清除）
  m_Registry.clear();

  // 2. 重置实体ID计数器
  m_EntityCounter = 0;

  // 3. 重置主相机引用
  m_MainCamera = std::make_unique<Entity>(weak_from_this(), entt::null);

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
    // 销毁所有系统（除了核心系统）
    auto it = m_Systems.begin();
    while (it != m_Systems.end()) {
      // 保留核心系统
      if (it->second == m_SceneGraph || it->second == m_SceneObserver ||
          it->second == m_Serializer)
      {
        ++it;
      }
      else {
        it = m_Systems.erase(it);
      }
    }

    // 重新初始化核心系统
    if (m_SceneGraph)
      m_SceneGraph->Init();
    if (m_SceneObserver)
      m_SceneObserver->Init();
  }

  // 7. 重新创建默认环境实体（类似构造函数中的初始化）
  auto env = CreateEntity("Environment");
  env.AddComponent<EnvironmentComponent>();
}

Entity Scene::CreateEntity(const std::string &name)
{
  Entity entity = {std::make_shared<Scene>(this), m_Registry.create()};

  // 添加基本组件，自动生成唯一ID
  auto &id = entity.AddComponent<IDComponent>();
  ++m_EntityCounter;

  // 添加Tag系统，用于实体搜索和筛选
  auto &tag = entity.AddComponent<TagComponent>();
  tag.Tag = name.empty() ? "Entity_" + id.String() : name;

  //// TODO: 添加变换系统
  // entity.AddComponent<TransformComponent>();

  return entity;
}

void Scene::DestroyEntity(Entity entity)
{
  if (!IsValid(entity)) {
    return;
  }

  // 标记销毁而不是立即销毁，避免迭代器失效
  entity.AddComponent<DestroyComponent>();
}

bool Scene::IsValid(Entity entity) const
{
  return entity && m_Registry.valid(entity.GetHandle());
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