#ifndef MITE_SCENE_ENTITY
#define MITE_SCENE_ENTITY

#include "headers/headers.h"
#include <entt/entt.hpp>

namespace mite {
// 前向声明
class Scene;
class SceneRegistry;

/**
 * @brief 实体类，表示场景中的一个独立对象
 *
 * 封装EnTT实体，提供类型安全的组件操作和层次结构管理
 * 注意：Entity对象是轻量级的，可以自由复制和传递
 */
class Entity {
 public:
  // 基于EnTT的Custom class Entity所必须的设定 ==================

  using entity_type = uint32_t;
  operator entt::entity() const noexcept
  {
    return m_Handle;
  }
  operator entt::entity &() noexcept
  {
    return m_Handle;
  }

  // 构造函数 ===================================================

  /**
   * @brief 默认构造一个空实体（无效实体）
   *
   * 注意，此时m_Handle = entt::null，执行IsValid()返回值为false
   * 
   * TODO：潜在风险：智能指针weak_ptr未正常初始化，可能导致逻辑错误
   */
  Entity() = default;

  /**
   * @brief 从场景和EnTT实体构造
   * @param scene 所属场景的弱引用
   * @param handle 底层EnTT实体句柄
   */
  explicit Entity(std::weak_ptr<Scene> scene, entt::entity handle);

  /**
   * @brief 拷贝构造函数
   * @param other
   */
  Entity(const Entity &other);

  /**
   * @brief 默认析构函数
   */
  ~Entity() = default;

  // 实体状态 ===================================================

  /**
   * @brief 检查实体是否有效
   * @return 是否有效（未被销毁）
   */
  bool IsValid() const;

  /**
   * @brief 销毁此实体（包括所有子实体）
   */
  void Destroy();

  /**
   * @brief 获取底层EnTT实体句柄
   */
  entt::entity GetHandle() const;

  /**
   * @brief 获取所属场景
   * @return 场景共享指针（可能为空）
   */
  std::shared_ptr<Scene> GetScene() const;

  // 操作符重载 =================================================

  /**
   * @brief 比较两个实体是否相同
   */
  bool operator==(const Entity &other) const;
  bool operator!=(const Entity &other) const;

  /**
   * @brief 转换为bool表示实体是否有效
   */
  explicit operator bool() const;

 private:
  std::weak_ptr<Scene> m_Scene;       // 所属场景的弱引用（避免循环引用）
  entt::entity m_Handle{entt::null};  // 底层EnTT实体句柄
};
};  // namespace mite

// 哈希支持，用于将Entity用作unordered_map的key
namespace std {
template<> struct hash<mite::Entity> {
  size_t operator()(const mite::Entity &entity) const
  {
    return hash<entt::entity>()(entity.GetHandle());
  }
};
}  // namespace std

// 将 Entity 用作 EnTT 的标识类型,特化entt_traits
// （暂时不需要，自定义的注册组件已完成对其功能的替代）
//namespace entt {
//template<> struct entt_traits<mite::Entity> : entt_traits<entt::entity> {};

//}  // namespace entt

#endif
