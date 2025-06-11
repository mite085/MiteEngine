#include "scene.h"
#include "scene_graph.h"
#include "scene_observer.h"
#include "scene_serializer.h"
namespace mite {
Scene::Scene(const std::string &name) : m_Name(name), m_MainCamera(entt::null, this)
{

  // ��ʼ������ϵͳ
  InitSystems();

  //// TODO: ����Ĭ�ϻ���ʵ��
  //auto env = CreateEntity("Environment");
  //env.AddComponent<EnvironmentComponent>();
}

Scene::~Scene()
{
  // ȷ������ϵͳ����ȷ˳������
  m_Systems.clear();
}

void Scene::InitSystems()
{
  // ע�����ϵͳ
  m_SceneGraph = std::make_unique<SceneGraph>(this);
  m_SceneObserver = std::make_unique<SceneObserver>(this);
  m_Serializer = std::make_unique<SceneSerializer>(this);

  //// TODO: ע��任ϵͳ
  //RegisterSystem<TransformSystem>();
}

void Scene::OnUpdate(float timestep)
{
  // ��������ע���ϵͳ
  for (auto &[_, system] : m_Systems) {
    system->OnUpdate(timestep);
  }

  // ����ͼ����
  m_SceneGraph->OnUpdate(timestep);

  // ����ʵ�����ٶ���
  m_Registry.view<DestroyComponent>().each(
      [this](auto entity, auto &) { m_Registry.destroy(entity); });
}

void Scene::OnRenderPrepare()
{
  // ���³����۲���
  m_SceneObserver->OnUpdate();

  // ׼������ͼ��Ⱦ״̬
  m_SceneGraph->OnRenderPrepare();
}

Entity Scene::CreateEntity(const std::string &name)
{
  Entity entity = {std::make_shared<Scene>(this), m_Registry.create()};

  // ��ӻ���������Զ�����ΨһID
  auto &id = entity.AddComponent<IDComponent>();
  ++m_EntityCounter;

  // TODO: ���Tagϵͳ������ʵ��������ɸѡ
  auto &tag = entity.AddComponent<TagComponent>();
  tag.Tag = name.empty() ? "Entity_" + id.String() : name;

  //// TODO: ��ӱ任ϵͳ
  //entity.AddComponent<TransformComponent>();

  return entity;
}

void Scene::DestroyEntity(Entity entity)
{
  if (!IsValid(entity)) {
    return;
  }

  // ������ٶ������������٣����������ʧЧ
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