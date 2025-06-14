#ifndef MITE_SCENE_ENTITY
#define MITE_SCENE_ENTITY

#include "component.h"

namespace mite {

// 前置声明
class Scene;

/**
 * @brief 实体类，表示场景中的一个独立对象
 *
 * 封装EnTT实体，提供类型安全的组件操作和层次结构管理
 * 注意：Entity对象是轻量级的，可以自由复制和传递
 */
class Entity {
 public:
  // 构造函数 ===================================================

  /**
   * @brief 默认构造一个空实体（无效实体）
   */
  Entity() = default;

  /**
   * @brief 从场景和EnTT实体构造
   * @param scene 所属场景的弱引用
   * @param handle 底层EnTT实体句柄
   */
  Entity(std::weak_ptr<Scene> scene, entt::entity handle);

  /**
   * @brief 默认析构函数
   */
  ~Entity() = default;

  // 组件操作 ===================================================

  /**
   * @brief 添加组件（构造新组件）
   * @tparam T 组件类型
   * @tparam Args 构造参数类型
   * @param args 组件构造参数
   * @return 新添加的组件引用
   */
  template<typename T, typename... Args> T &AddComponent(Args &&...args);

  /**
   * @brief 移除组件
   * @tparam T 组件类型
   */
  template<typename T> void RemoveComponent();

  /**
   * @brief 检查是否拥有某个组件
   * @tparam T 组件类型
   * @return 是否拥有该组件
   */
  template<typename T> bool HasComponent() const;

  /**
   * @brief 获取组件（非const版本）
   * @tparam T 组件类型
   * @return 组件引用
   * @throws std::runtime_error 当组件不存在时抛出异常
   */
  template<typename T> T &GetComponent();

  /**
   * @brief 获取组件（const版本）
   * @tparam T 组件类型
   * @return 组件const引用
   * @throws std::runtime_error 当组件不存在时抛出异常
   */
  template<typename T> const T &GetComponent() const;

  /**
   * @brief 尝试获取组件（非const版本）
   * @tparam T 组件类型
   * @return 组件指针（不存在时返回nullptr）
   */
  template<typename T> T *TryGetComponent();

  /**
   * @brief 尝试获取组件（const版本）
   * @tparam T 组件类型
   * @return 组件const指针（不存在时返回nullptr）
   */
  template<typename T> const T *TryGetComponent() const;

  // 层次结构操作 ================================================

  /**
   * @brief 设置父实体
   * @param parent 父实体
   * @param keepWorldTransform 是否保持世界空间变换
   */
  void SetParent(Entity parent, bool keepWorldTransform = true);

  /**
   * @brief 获取父实体
   * @return 父实体（无效实体表示没有父级）
   */
  Entity GetParent() const;

  /**
   * @brief 获取所有子实体
   * @return 子实体列表（按创建顺序）
   */
  const std::vector<Entity> &GetChildren() const;

  /**
   * @brief 添加子实体
   * @param child 子实体
   * @param keepWorldTransform 是否保持世界空间变换
   */
  void AddChild(Entity child, bool keepWorldTransform = true);

  /**
   * @brief 移除子实体
   * @param child 子实体
   * @param keepWorldTransform 是否保持世界空间变换
   */
  void RemoveChild(Entity child, bool keepWorldTransform = true);

  /**
   * @brief 检查是否为某个实体的祖先
   * @param potentialAncestor 可能的祖先实体
   * @return 是否是祖先
   */
  bool IsDescendantOf(Entity potentialAncestor) const;

  /**
   * @brief 获取实体层级
   * @return 当前实体的层数
   */
  size_t GetDepth() const;

  // 实体状态 ===================================================

  /**
   * @brief 获取实体ID（持久化标识）
   * @return UUID字符串
   */
  const std::string &GetID() const;

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
  // 内部方法 ===================================================
  void UpdateChildParentRelationship(Entity child, bool keepWorldTransform);
  void RemoveFromParent();

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

#endif
