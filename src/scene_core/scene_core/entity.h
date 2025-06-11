#ifndef MITE_SCENE_ENTITY
#define MITE_SCENE_ENTITY

#include "headers/headers.h"

namespace mite {

class Scene;      // 前向声明
class Component;  // 前向声明

using EntityID = uint32_t;

/**
 * @class Entity
 * @brief 表示场景中的一个实体，作为组件的容器
 *
 * 实体本身不包含逻辑，仅作为组件的集合标识符。
 * 所有功能通过附加的组件实现。
 */
class Entity {
 public:
  /**
   * @brief 构造函数
   * @param scene 所属场景的指针
   * @param id 实体唯一ID
   */
  Entity(Scene *scene, EntityID id);

  ~Entity();

  // 禁用拷贝构造和赋值
  Entity(const Entity &) = delete;
  Entity &operator=(const Entity &) = delete;

  // 允许移动语义
  Entity(Entity &&) = default;
  Entity &operator=(Entity &&) = default;

  /**
   * @brief 获取实体ID
   * @return 实体唯一ID
   */
  EntityID GetID() const;

  /**
   * @brief 获取所属场景
   * @return 指向所属场景的指针
   */
  Scene *GetScene() const;

  /**
   * @brief 检查实体是否活跃
   * @return true如果实体活跃，false否则
   */
  bool IsActive() const;

  /**
   * @brief 设置实体活跃状态
   * @param active 要设置的状态
   */
  void SetActive(bool active);

  /**
   * @brief 获取实体名称
   * @return 实体名称字符串
   */
  const std::string &GetName() const;

  /**
   * @brief 设置实体名称
   * @param name 新名称
   */
  void SetName(const std::string &name);

  // ==================== 组件管理 ====================

  /**
   * @brief 添加组件到实体
   * @tparam T 组件类型
   * @tparam Args 构造函数参数类型
   * @param args 组件构造函数参数
   * @return 指向新组件的指针
   */
  template<typename T, typename... Args> T *AddComponent(Args &&...args);

  /**
   * @brief 从实体移除组件
   * @tparam T 组件类型
   */
  template<typename T> void RemoveComponent();

  /**
   * @brief 检查实体是否有特定类型的组件
   * @tparam T 组件类型
   * @return true如果实体有该组件，false否则
   */
  template<typename T> bool HasComponent() const;

  /**
   * @brief 获取实体上的组件
   * @tparam T 组件类型
   * @return 指向组件的指针，如果没有则返回nullptr
   */
  template<typename T> T *GetComponent();

  /**
   * @brief 获取实体上的组件(常量版本)
   * @tparam T 组件类型
   * @return 指向组件的常量指针，如果没有则返回nullptr
   */
  template<typename T> const T *GetComponent() const;

  /**
   * @brief 获取实体所有组件
   * @return 组件类型到组件指针的映射
   */
  const std::unordered_map<std::type_index, std::unique_ptr<Component>> &GetAllComponents() const
  {
    return m_Components;
  }

  // ==================== 父子关系管理 ====================

  /**
   * @brief 设置父实体
   * @param parent 要设置为父实体的指针
   */
  void SetParent(Entity *parent);

  /**
   * @brief 获取父实体
   * @return 父实体指针，如果没有则返回nullptr
   */
  Entity *GetParent() const
  {
    return m_Parent;
  }

  /**
   * @brief 获取所有子实体
   * @return 子实体指针的向量
   */
  const std::vector<Entity *> &GetChildren() const
  {
    return m_Children;
  }

  /**
   * @brief 添加子实体
   * @param child 要添加的子实体指针
   */
  void AddChild(Entity *child);

  /**
   * @brief 移除子实体
   * @param child 要移除的子实体指针
   */
  void RemoveChild(Entity *child);

  /**
   * @brief 检查是否为另一个实体的祖先
   * @param entity 要检查的实体
   * @return true如果是祖先，false否则
   */
  bool IsAncestorOf(const Entity *entity) const;

  /**
   * @brief 检查是否为另一个实体的后代
   * @param entity 要检查的实体
   * @return true如果是后代，false否则
   */
  bool IsDescendantOf(const Entity *entity) const;

 private:
  Scene *m_Scene;      // 所属场景
  EntityID m_ID;       // 唯一标识符
  bool m_IsActive;     // 活跃状态
  std::string m_Name;  // 实体名称

  // 组件存储
  std::unordered_map<std::type_index, std::unique_ptr<Component>> m_Components;

  // 层级关系
  Entity *m_Parent = nullptr;        // 父实体
  std::vector<Entity *> m_Children;  // 子实体列表
};

}  // namespace mite

#endif