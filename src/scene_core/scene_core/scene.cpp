#include "scene.h"
#include "scene_graph.h"
#include "scene_observer.h"
#include "scene_serializer.h"
namespace mite {
Scene::Scene(const std::string &name) : m_Name(name)
{
  // ��ʼ������ϵͳ
  InitSystems();

  //// TODO: ����Ĭ�ϻ���ʵ��
  // auto env = CreateEntity("Environment");
  // env.AddComponent<EnvironmentComponent>();

  m_MainCamera = std::make_shared<Entity>(weak_from_this(), entt::null);
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
  // RegisterSystem<TransformSystem>();
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

void Scene::Clear(bool keepSystems)
{
  // 1. ��������ʵ�壨���������������¼���ֱ�����������
  m_Registry.clear();

  // 2. ����ʵ��ID������
  m_EntityCounter = 0;

  // 3. �������������
  m_MainCamera = std::make_unique<Entity>(weak_from_this(), entt::null);

  // 4. ���ó���ͼ״̬
  if (m_SceneGraph) {
    m_SceneGraph->Clear();
  }

  // 5. ���ó����۲���״̬
  if (m_SceneObserver) {
    m_SceneObserver->Clear();
  }

  // 6. ���ݲ��������Ƿ�����ϵͳ
  if (!keepSystems) {
    // ��������ϵͳ�����˺���ϵͳ��
    auto it = m_Systems.begin();
    while (it != m_Systems.end()) {
      // ��������ϵͳ
      if (it->second == m_SceneGraph || it->second == m_SceneObserver ||
          it->second == m_Serializer)
      {
        ++it;
      }
      else {
        it = m_Systems.erase(it);
      }
    }

    // ���³�ʼ������ϵͳ
    if (m_SceneGraph)
      m_SceneGraph->Init();
    if (m_SceneObserver)
      m_SceneObserver->Init();
  }

  // 7. ���´���Ĭ�ϻ���ʵ�壨���ƹ��캯���еĳ�ʼ����
  auto env = CreateEntity("Environment");
  env.AddComponent<EnvironmentComponent>();
}

Entity Scene::CreateEntity(const std::string &name)
{
  Entity entity = {std::make_shared<Scene>(this), m_Registry.create()};

  // ��ӻ���������Զ�����ΨһID
  auto &id = entity.AddComponent<IDComponent>();
  ++m_EntityCounter;

  // ���Tagϵͳ������ʵ��������ɸѡ
  auto &tag = entity.AddComponent<TagComponent>();
  tag.Tag = name.empty() ? "Entity_" + id.String() : name;

  //// TODO: ��ӱ任ϵͳ
  // entity.AddComponent<TransformComponent>();

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