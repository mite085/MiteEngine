#ifndef MITE_SCENE_SERIALIZER
#define MITE_SCENE_SERIALIZER

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <entt/entt.hpp>
#include "headers/headers.h"

namespace mite {
// 前向声明
class Scene;

/**
 * @brief 场景序列化器，负责将场景数据序列化为各种格式以及反序列化
 *
 * 支持 JSON 和二进制格式，基于 Cereal 库实现。处理核心场景数据、
 * 实体、组件以及资产引用等的序列化。
 */
class SceneSerializer {
 public:
  /**
   * @brief 构造函数
   * @param scene 要序列化的场景引用
   */
  explicit SceneSerializer(Scene &scene);

  /**
   * @brief 序列化场景到文件（JSON格式）
   * @param filepath 文件路径
   * @return 是否成功
   */
  bool SerializeToJson(const std::string &filepath);

  /**
   * @brief 从JSON文件反序列化场景
   * @param filepath 文件路径
   * @return 是否成功
   */
  bool DeserializeFromJson(const std::string &filepath);

  /**
   * @brief 序列化场景到文件（二进制格式）
   * @param filepath 文件路径
   * @return 是否成功
   */
  bool SerializeToBinary(const std::string &filepath);

  /**
   * @brief 从二进制文件反序列化场景
   * @param filepath 文件路径
   * @return 是否成功
   */
  bool DeserializeFromBinary(const std::string &filepath);

  /**
   * @brief 获取最后一次序列化/反序列化的错误信息
   * @return 错误信息字符串
   */
  const std::string &GetLastError() const
  {
    return m_lastError;
  }

 private:
  // 场景引用
  Scene &m_scene;

  // 存储最后一次错误信息
  std::string m_lastError;

  /**
   * @brief 注册所有可序列化的组件类型
   * @tparam Archive Cereal存档类型
   * @param archive Cereal存档对象
   */
  template<typename Archive> void RegisterComponentTypes(Archive &archive);

  /**
   * @brief 序列化实体及其组件
   * @tparam Archive Cereal存档类型
   * @param archive Cereal存档对象
   * @param registry EnTT注册表
   */
  template<typename Archive> void SerializeEntities(Archive &archive, entt::registry &registry);

  /**
   * @brief 反序列化实体及其组件
   * @tparam Archive Cereal存档类型
   * @param archive Cereal存档对象
   * @param registry EnTT注册表
   */
  template<typename Archive> void DeserializeEntities(Archive &archive, entt::registry &registry);

  /**
   * @brief 类型名称辅助函数
   * @return Component的类型名称，如"Transform"、"Tag"等
   */
  template<typename T> const char *component_type_name();
};
};

#endif
