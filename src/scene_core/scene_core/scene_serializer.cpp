#include "scene_serializer.h"
#include "scene.h"

namespace mite {
SceneSerializer::SceneSerializer(Scene &scene) : m_scene(scene) {}

bool SceneSerializer::SerializeToJson(const std::string &filepath)
{
  try {
    std::ofstream os(filepath);
    if (!os.is_open()) {
      m_lastError = "Failed to open file for writing: " + filepath;
      return false;
    }

    cereal::JSONOutputArchive archive(os);

    // ���л�����Ԫ����
    archive(cereal::make_nvp("Scene", m_scene.GetName()));

    // ע�Ტ���л������������
    RegisterComponentTypes(archive);

    // ���л�ʵ��
    SerializeEntities(archive, m_scene.GetRegistry());

    return true;
  }
  catch (const std::exception &e) {
    m_lastError = "JSON Serialization Error: " + std::string(e.what());
    return false;
  }
}

bool SceneSerializer::DeserializeFromJson(const std::string &filepath)
{
  try {
    std::ifstream is(filepath);
    if (!is.is_open()) {
      m_lastError = "Failed to open file for reading: " + filepath;
      return false;
    }

    cereal::JSONInputArchive archive(is);

    // �����л�����Ԫ����
    std::string sceneName;
    archive(cereal::make_nvp("Scene", sceneName));
    m_scene.SetName(sceneName);

    // ע���������
    RegisterComponentTypes(archive);

    // ��յ�ǰ����
    m_scene.Clear();

    // �����л�ʵ��
    DeserializeEntities(archive, m_scene.GetRegistry());

    return true;
  }
  catch (const std::exception &e) {
    m_lastError = "JSON Deserialization Error: " + std::string(e.what());
    return false;
  }
}

bool SceneSerializer::SerializeToBinary(const std::string &filepath)
{
  try {
    std::ofstream os(filepath, std::ios::binary);
    if (!os.is_open()) {
      m_lastError = "Failed to open file for writing: " + filepath;
      return false;
    }

    cereal::BinaryOutputArchive archive(os);

    // ���л�����Ԫ����
    archive(m_scene.GetName());

    // ע�Ტ���л������������
    RegisterComponentTypes(archive);

    // ���л�ʵ��
    SerializeEntities(archive, m_scene.GetRegistry());

    return true;
  }
  catch (const std::exception &e) {
    m_lastError = "Binary Serialization Error: " + std::string(e.what());
    return false;
  }
}

bool SceneSerializer::DeserializeFromBinary(const std::string &filepath)
{
  try {
    std::ifstream is(filepath, std::ios::binary);
    if (!is.is_open()) {
      m_lastError = "Failed to open file for reading: " + filepath;
      return false;
    }

    cereal::BinaryInputArchive archive(is);

    // �����л�����Ԫ����
    std::string sceneName;
    archive(sceneName);
    m_scene.SetName(sceneName);

    // ע���������
    RegisterComponentTypes(archive);

    // ��յ�ǰ����
    m_scene.Clear();

    // �����л�ʵ��
    DeserializeEntities(archive, m_scene.GetRegistry());

    return true;
  }
  catch (const std::exception &e) {
    m_lastError = "Binary Deserialization Error: " + std::string(e.what());
    return false;
  }
}

template<typename Archive> void SceneSerializer::RegisterComponentTypes(Archive &archive)
{
  // Ϊÿ���������ע�����л�����
  // ʹ��CEREAL_REGISTER_TYPE��ȷ��������Ϣ����ȷ��¼

  // ʾ����Tag���
  archive.template register_type<TagComponent>();

  // TODO: �������...
  // archive.template register_type<TransformComponent>();
  // archive.template register_type<MeshRendererComponent>();
  // archive.template register_type<LightComponent>();
  // archive.template register_type<CameraComponent>();
  // archive.template register_type<ScriptComponent>();
}

template<typename Archive>
void SceneSerializer::SerializeEntities(Archive &archive, entt::registry &registry)
{
  // ��ȡ����ʵ��
  auto view = registry.view<entt::entity>();

  // �������л�ʵ������
  const auto count = view.size();
  archive(cereal::make_nvp("EntityCount", count));

  // ���л�ÿ��ʵ�弰����� - ʹ����ͼ����
  for (auto entity : view) {
    // ���л�ʵ��ID
    archive(cereal::make_nvp("Entity", entity));

    // ���л���ʵ���ϵ��������
    registry.visit(entity, [&archive, &registry, entity](const auto &component) {
      using ComponentType = std::decay_t<decltype(registry.get<ComponentType>(entity))>;
      try {
        archive(cereal::make_nvp(component_type_name<ComponentType>(),  // ��Ҫʵ�ֻ�ȡ�������ĺ���
                                 registry.template get<ComponentType>(entity)));
      }
      catch (const std::exception &e) {
        // �������л�����
        m_lastError = "Failed to serialize component: " + std::string(e.what());
      }
    });
  }
}

template<typename Archive>
void SceneSerializer::DeserializeEntities(Archive &archive, entt::registry &registry)
{
  // �����л�ʵ������
  size_t entityCount = 0;
  archive(cereal::make_nvp("EntityCount", entityCount));

  // �����л�ÿ��ʵ�弰�����
  for (size_t i = 0; i < entityCount; ++i) {
    entt::entity entity = entt::null;

    // �����л�ʵ��ID
    archive(cereal::make_nvp("Entity", entity));

    // ȷ��ʵ����ע����д���
    if (!registry.valid(entity)) {
      entity = registry.create(entity);
    }

    // �����л����
    // Cereal��������л�ʱ��¼��������Ϣ�Զ�����
    std::string componentName;
    while (true) {
      try {
        // ���Զ�ȡ��һ�������
        archive.setNextName(nullptr);
        if (!archive.tryGetName(componentName)) {
          break;  // û�и��������
        }

        // ��������������л����
        // ������ҪΪÿ���������ʵ���ض��ķ����л��߼�
        // ����ʹ�ù���ģʽ������ע�������̬�������

        if (componentName == TransformComponent::GetSerializationName()) {
          TransformComponent transform;
          archive(transform);
          registry.emplace_or_replace<TransformComponent>(entity, transform);
        }
        else if (componentName == TagComponent::GetSerializationName()) {
          TagComponent tag;
          archive(tag);
          registry.emplace_or_replace<TagComponent>(entity, tag);
        }
        // ����������͵Ĵ���...
      }
      catch (const cereal::Exception &e) {
        // ������������л�����
        m_lastError = "Component deserialization error: " + std::string(e.what());
        break;
      }
    }
  }

  // �����л��������Ҫ�ؽ�ʵ���Ĺ�ϵ���縸�ӹ�ϵ��
  // ����������ʵ�������ɺ������һ��
}

template<typename T> const char *SceneSerializer::component_type_name()
{
  if constexpr (std::is_same_v<T, TransformComponent>) {
    return "Transform";
  }
  else if constexpr (std::is_same_v<T, TagComponent>) {
    return "Tag";
  }
  // TODO: �����������...
  else {
    static_assert(false, "Unregistered component type");
  }
}

};
