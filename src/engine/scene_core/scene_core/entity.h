#ifndef MITE_SCENE_ENTITY
#define MITE_SCENE_ENTITY

#include "headers/headers.h"

namespace mite {
// 前向声明
class SceneRegistry;

/**
 * @brief 实体类，表示场景中的一个独立对象
 * 使用UUID作为唯一标识，提供类型安全的组件操作和层次结构管理
 *
 * 注意：
 * 仅SceneRegistry有权限构造有效实体，其他所有非拷贝构造的实体均为空实体
 */
class Entity {
  //=================== 构造函数 =========================
 public:
  /**
   * @brief 默认构造一个空实体（无效实体）
   */
  Entity();  // 生成空ID

  /**
   * @brief 拷贝构造函数
   */
  Entity(const Entity &other);

  ~Entity() = default;

 private:
  /**
   * @brief 从UUID构造有效实体（内部使用）
   * @param uuid 实体UUID
   */
  explicit Entity(const std::string &name, const UUID &uuid);

  /**
   * @brief 构造一个有效实体（内部和友元函数使用）
   *
   * 原则上仅SceneRegistry有权限调用，
   * 其他所有非拷贝构造的实体均为空实体
   *
   * 若有其他模块需要构建有效实体，需要添加为friend
   */
  static Entity CreateEntity(const std::string &name = "");
  friend SceneRegistry;

  //=================== 实体状态操作 =========================
 public:
  /**
   * @brief 检查实体是否有效
   * @return 是否有效（未被销毁）
   */
  bool IsValid() const;

  /**
   * @brief 销毁此实体（标记为无效）
   */
  void Destroy();

  /**
   * @brief 获取实体UUID
   */
  UUID GetUUID() const { return m_UUID; }
  std::string GetName() const { return m_Name; }

  /**
   * @brief 获取实体UUID字符串
   */
  std::string GetUUIDString() const { return UUIDGenerator::UUIDToString(m_UUID); }

  //===================== 操作符重载 ========================

  bool operator==(const Entity &other) const { return m_UUID == other.m_UUID; }

  bool operator!=(const Entity &other) const { return !(*this == other); }

  explicit operator bool() const { return IsValid(); }

 private:
  std::string m_Name;  // 实体名称
  UUID m_UUID;         // 实体唯一标识
};
}  // namespace mite

// 哈希支持，用于将Entity用作unordered_map的key
namespace std {
template<> struct hash<mite::Entity> {
  size_t operator()(const mite::Entity &entity) const
  {
    return hash<mite::UUID>()(entity.GetUUID());
  }
};
}  // namespace std

#endif
