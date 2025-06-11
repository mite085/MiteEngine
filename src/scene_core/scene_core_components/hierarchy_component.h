#ifndef MITE_SCENE_HIERACHY_COMPONENT
#define MITE_SCENE_HIERACHY_COMPONENT

#include "headers/headers.h"
#include <entt/entt.hpp>

namespace mite {
/**
 * @brief 实体层次结构组件
 *
 * 管理实体间的父子关系，构成场景图的基础结构
 * 注意：实际父子关系逻辑由Entity类管理，此类仅存储数据
 */
class HierarchyComponent {
 public:
  /**
   * @brief 默认构造函数（创建无父节点的根实体）
   */
  HierarchyComponent() = default;

  // 禁止拷贝（层次关系应唯一）
  HierarchyComponent(const HierarchyComponent &) = delete;
  HierarchyComponent &operator=(const HierarchyComponent &) = delete;

  // 允许移动
  HierarchyComponent(HierarchyComponent &&) noexcept = default;
  HierarchyComponent &operator=(HierarchyComponent &&) noexcept = default;

  /**
   * @brief 获取父实体句柄
   * @return 父实体EnTT句柄（entt::null表示无父节点）
   */
  entt::entity GetParent() const
  {
    return m_Parent;
  }

  /**
   * @brief 获取所有子实体句柄
   * @return 子实体句柄列表（按添加顺序）
   */
  const std::vector<entt::entity> &GetChildren() const
  {
    return m_Children;
  }

  /**
   * @brief 获取子实体数量
   */
  size_t GetChildCount() const
  {
    return m_Children.size();
  }

  /**
   * @brief 检查是否为叶节点（无子节点）
   */
  bool IsLeaf() const
  {
    return m_Children.empty();
  }

  /**
   * @brief 检查是否为根节点（无父节点）
   */
  bool IsRoot() const
  {
    return m_Parent == entt::null;
  }

  /**
   * @brief 获取深度（距离根节点的层级数）
   * @note 需要在场景中查询父级，可能有一定开销
   */
  size_t GetDepth(const entt::registry &registry) const;

 private:
  friend class Entity;  // 允许Entity类直接修改层次关系

  // 内部方法 ==============================================

  /**
   * @brief 添加子节点（内部使用）
   * @param child 子实体句柄
   */
  void AddChild(entt::entity child);

  /**
   * @brief 移除子节点（内部使用）
   * @param child 子实体句柄
   * @return 是否成功移除
   */
  bool RemoveChild(entt::entity child);

  /**
   * @brief 清空所有子节点（内部使用）
   */
  void ClearChildren();

  /**
   * @brief 设置父节点（内部使用）
   * @param parent 父实体句柄
   */
  void SetParent(entt::entity parent);

 private:
  entt::entity m_Parent{entt::null};     // 父实体句柄
  std::vector<entt::entity> m_Children;  // 子实体列表
  size_t m_DepthCache = 0;               // 深度缓存（非持久化）
};
};

#endif
