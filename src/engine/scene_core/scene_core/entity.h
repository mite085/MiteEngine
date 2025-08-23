#ifndef MITE_SCENE_ENTITY
#define MITE_SCENE_ENTITY

#include "headers/headers.h"

namespace mite {

/**
 * @brief 实体类，表示场景中的一个独立对象
 *
 * 使用UUID作为唯一标识，提供类型安全的组件操作和层次结构管理
 */
class Entity {
 public:
  /**
   * @brief 默认构造一个空实体（无效实体）
   */
  Entity();  // 生成空ID

  /**
   * @brief 构造一个有效实体（原则上仅SceneRegistry::CreateEntity有权限调用）
   */
  static Entity CreateEntity();

  /**
   * @brief 从UUID构造实体
   * @param uuid 实体UUID
   */
  explicit Entity(const uuids::uuid &uuid);

  /**
   * @brief 拷贝构造函数
   */
  Entity(const Entity &other);

  ~Entity() = default;



  // 实体状态操作 ============================================

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
  uuids::uuid GetUUID() const
  {
    return m_UUID;
  }

  /**
   * @brief 获取实体UUID字符串
   */
  std::string GetUUIDString() const
  {
    return uuids::to_string(m_UUID);
  }

  // 操作符重载 =============================================

  bool operator==(const Entity &other) const
  {
    return m_UUID == other.m_UUID;
  }

  bool operator!=(const Entity &other) const
  {
    return !(*this == other);
  }

  explicit operator bool() const
  {
    return IsValid();
  }

 private:
  uuids::uuid m_UUID;  // 实体唯一标识
};

}  // namespace mite

// 哈希支持，用于将Entity用作unordered_map的key
namespace std {
template<> struct hash<mite::Entity> {
  size_t operator()(const mite::Entity &entity) const
  {
    return hash<uuids::uuid>()(entity.GetUUID());
  }
};
}  // namespace std

#endif
