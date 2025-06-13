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

    // 序列化场景元数据
    archive(cereal::make_nvp("Scene", m_scene.GetName()));

    // 注册并序列化所有组件类型
    RegisterComponentTypes(archive);

    // 序列化实体
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

    // 反序列化场景元数据
    std::string sceneName;
    archive(cereal::make_nvp("Scene", sceneName));
    m_scene.SetName(sceneName);

    // 注册组件类型
    RegisterComponentTypes(archive);

    // 清空当前场景
    m_scene.Clear();

    // 反序列化实体
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

    // 序列化场景元数据
    archive(m_scene.GetName());

    // 注册并序列化所有组件类型
    RegisterComponentTypes(archive);

    // 序列化实体
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

    // 反序列化场景元数据
    std::string sceneName;
    archive(sceneName);
    m_scene.SetName(sceneName);

    // 注册组件类型
    RegisterComponentTypes(archive);

    // 清空当前场景
    m_scene.Clear();

    // 反序列化实体
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
  // 为每个组件类型注册序列化函数
  // 使用CEREAL_REGISTER_TYPE宏确保类型信息被正确记录

  // 示例：Tag组件
  archive.template register_type<TagComponent>();

  // TODO: 其他组件...
  // archive.template register_type<TransformComponent>();
  // archive.template register_type<MeshRendererComponent>();
  // archive.template register_type<LightComponent>();
  // archive.template register_type<CameraComponent>();
  // archive.template register_type<ScriptComponent>();
}

template<typename Archive>
void SceneSerializer::SerializeEntities(Archive &archive, entt::registry &registry)
{
  // 获取所有实体
  auto view = registry.view<entt::entity>();

  // 首先序列化实体数量
  const auto count = view.size();
  archive(cereal::make_nvp("EntityCount", count));

  // 序列化每个实体及其组件 - 使用视图迭代
  for (auto entity : view) {
    // 序列化实体ID
    archive(cereal::make_nvp("Entity", entity));

    // 序列化该实体上的所有组件
    registry.visit(entity, [&archive, &registry, entity](const auto &component) {
      using ComponentType = std::decay_t<decltype(registry.get<ComponentType>(entity))>;
      try {
        archive(cereal::make_nvp(component_type_name<ComponentType>(),  // 需要实现获取类型名的函数
                                 registry.template get<ComponentType>(entity)));
      }
      catch (const std::exception &e) {
        // 处理序列化错误
        m_lastError = "Failed to serialize component: " + std::string(e.what());
      }
    });
  }
}

template<typename Archive>
void SceneSerializer::DeserializeEntities(Archive &archive, entt::registry &registry)
{
  // 反序列化实体数量
  size_t entityCount = 0;
  archive(cereal::make_nvp("EntityCount", entityCount));

  // 反序列化每个实体及其组件
  for (size_t i = 0; i < entityCount; ++i) {
    entt::entity entity = entt::null;

    // 反序列化实体ID
    archive(cereal::make_nvp("Entity", entity));

    // 确保实体在注册表中存在
    if (!registry.valid(entity)) {
      entity = registry.create(entity);
    }

    // 反序列化组件
    // Cereal会根据序列化时记录的类型信息自动处理
    std::string componentName;
    while (true) {
      try {
        // 尝试读取下一个组件名
        archive.setNextName(nullptr);
        if (!archive.tryGetName(componentName)) {
          break;  // 没有更多组件了
        }

        // 根据组件名反序列化组件
        // 这里需要为每个组件类型实现特定的反序列化逻辑
        // 可以使用工厂模式或类型注册表来动态创建组件

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
        // 其他组件类型的处理...
      }
      catch (const cereal::Exception &e) {
        // 处理组件反序列化错误
        m_lastError = "Component deserialization error: " + std::string(e.what());
        break;
      }
    }
  }

  // 反序列化后可能需要重建实体间的关系（如父子关系）
  // 可以在所有实体加载完成后进行这一步
}

template<typename T> const char *SceneSerializer::component_type_name()
{
  if constexpr (std::is_same_v<T, TransformComponent>) {
    return "Transform";
  }
  else if constexpr (std::is_same_v<T, TagComponent>) {
    return "Tag";
  }
  // TODO: 其他组件类型...
  else {
    static_assert(false, "Unregistered component type");
  }
}

};
