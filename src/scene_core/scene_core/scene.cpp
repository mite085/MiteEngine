#include "scene.h"
#include "scene_graph.h"
#include "scene_observer.h"
#include "scene_serializer.h"
namespace mite {
Scene::Scene(const std::string &name) : m_Name(name), m_MainCamera(entt::null, this)
{

  // 初始化核心系统
  InitSystems();

  //// TODO: 创建默认环境实体
  //auto env = CreateEntity("Environment");
  //env.AddComponent<EnvironmentComponent>();
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
  //RegisterSystem<TransformSystem>();
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

Entity Scene::CreateEntity(const std::string &name)
{
  Entity entity = {std::make_shared<Scene>(this), m_Registry.create()};

  // 添加基本组件，自动生成唯一ID
  auto &id = entity.AddComponent<IDComponent>();
  ++m_EntityCounter;

  // TODO: 添加Tag系统，用于实体搜索和筛选
  auto &tag = entity.AddComponent<TagComponent>();
  tag.Tag = name.empty() ? "Entity_" + id.String() : name;

  //// TODO: 添加变换系统
  //entity.AddComponent<TransformComponent>();

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
  m_Serializer->Serialize(filepath);
}

void Scene::Deserialize(const std::filesystem::path &filepath)
{
  m_Serializer->Deserialize(filepath);
}

}  // namespace mite