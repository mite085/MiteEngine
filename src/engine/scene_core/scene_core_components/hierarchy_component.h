#ifndef MITE_SCENE_HIERACHY_COMPONENT
#define MITE_SCENE_HIERACHY_COMPONENT

#include "scene_core/component_system.h"

namespace mite {
/**
 * @brief 实体层次结构组件，
 * 管理实体间的父子关系，构成场景树的基础结构
 *
 * 注意：
 * 实际父子关系逻辑由Entity类管理，此类仅存储数据
 */
class HierarchyComponent : public ComponentTraits<HierarchyComponent, Component::Family::Core> {
 public:

  /**
   * @brief 构造函数（创建无父节点的根实体）
   */
  HierarchyComponent();

  // 显示拷贝
  HierarchyComponent(const HierarchyComponent &) noexcept;
  HierarchyComponent &operator=(const HierarchyComponent &) noexcept;

  // 允许移动
  HierarchyComponent(HierarchyComponent &&) noexcept = default;
  HierarchyComponent &operator=(HierarchyComponent &&) noexcept = default;

  /**
   * @brief 针对dirty对象进行处理
   */
  void ProcessDirty(float deltaTime, SceneRegistry &reg) override {}

  /**
   * @brief 获取父实体句柄
   * @return 父实体EnTT句柄（Entity()表示无父节点）
   */
  Entity GetParent() const
  {
    return m_Parent;
  }

  /**
   * @brief 获取所有子实体句柄
   * @return 子实体句柄列表（按添加顺序）
   */
  const std::vector<Entity> &GetChildren() const
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
    return m_Parent == Entity();
  }

  /**
   * @brief 获取深度（距离根节点的层级数）
   * @note 需要在场景中查询父级，可能有一定开销
   */
  size_t GetDepth(SceneRegistry &registry);

 private:
  

  friend class Entity;  // 允许Entity类直接修改层次关系
  friend class SceneGraph;

  // 内部方法 ==============================================

  /**
   * @brief 添加子节点（内部使用）
   * @param child 子实体句柄
   */
  void AddChild(Entity child);

  /**
   * @brief 移除子节点（内部使用）
   * @param child 子实体句柄
   * @return 是否成功移除
   */
  bool RemoveChild(Entity child);

  /**
   * @brief 清空所有子节点（内部使用）
   */
  void ClearChildren();

  /**
   * @brief 设置父节点（内部使用）
   * @param parent 父实体句柄
   */
  void SetParent(Entity parent);

 public:
  size_t m_DepthCache = 0;  // 深度缓存（非持久化）

 private:
  Entity m_Parent;                 // 父实体句柄
  std::vector<Entity> m_Children;  // 子实体列表

  friend class HierarchyComponentSystem;
};

// Hierarchy组件系统--用于批量处理脏数据 =====================================================

class HierarchyComponentSystem : public DirtyComponentSystem<HierarchyComponent> {
  DECLARE_COMPONENT_SYSTEM(HierarchyComponentSystem)
 public:
  void Initialize(SceneRegistry &registry) override;
  void Shutdown(SceneRegistry &registry) override;

  /**
   * @brief 获取系统执行优先级
   * @note 需要在TransformSystem之前执行
   */
  Component::Family GetExecutionOrder() const override
  {
    return Component::Family::Core;
  }

 private:
  void ProcessDirtyComponents(float deltaTime, SceneRegistry &registry) override;
  //void OnComponentUpdated(ComponentChangedEvent<HierarchyComponent> &e) override;
  bool OnComponentRemoved(ComponentRemovedEvent<HierarchyComponent> &e) override;

  /**
   * @brief 验证层次结构，防止循环依赖
   * @param entity 要检查的实体
   * @param newParent 新的父实体
   * @return 是否允许建立此父子关系
   */
  bool ValidateHierarchy(Entity entity, Entity newParent, SceneRegistry &registry);

  /**
   * @brief 递归更新子实体的深度缓存
   * @param entity 起始实体
   * @param registry 场景注册表
   */
  void UpdateChildrenDepthCache(Entity entity, SceneRegistry &registry);
};
// Hierarchy组件事件 =====================================================
/**
 * @class ParentChangedEvent
 * @brief 父节点改变事件
 */
class ParentChangedEvent : public ComponentEvent<HierarchyComponent> {
 public:
  ParentChangedEvent(Entity entity,
                     HierarchyComponent &component,
                     Entity oldParent,
                     Entity newParent)
      : ComponentEvent<HierarchyComponent>(entity, component),
        m_OldParent(oldParent),
        m_NewParent(newParent)
  {
  }

  EVENT_CLASS_TYPE(HIERACHY_COMPONENT_PARENT_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new ParentChangedEvent(entity, component, m_OldParent, m_NewParent);
  }

  Entity GetOldParent() const
  {
    return m_OldParent;
  }
  Entity GetNewParent() const
  {
    return m_NewParent;
  }

 private:
  Entity m_OldParent;
  Entity m_NewParent;
};

/**
 * @class ChildAddedEvent
 * @brief 子节点添加事件
 */
class ChildAddedEvent : public ComponentEvent<HierarchyComponent> {
 public:
  ChildAddedEvent(Entity entity, HierarchyComponent &component, Entity child)
      : ComponentEvent<HierarchyComponent>(entity, component), m_Child(child)
  {
  }

  EVENT_CLASS_TYPE(HIERACHY_COMPONENT_CHILD_ADDED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new ChildAddedEvent(entity, component, m_Child);
  }

  Entity GetChild() const
  {
    return m_Child;
  }

 private:
  Entity m_Child;
};

/**
 * @class ChildRemovedEvent
 * @brief 子节点移除事件
 */
class ChildRemovedEvent : public ComponentEvent<HierarchyComponent> {
 public:
  ChildRemovedEvent(Entity entity, HierarchyComponent &component, Entity child)
      : ComponentEvent<HierarchyComponent>(entity, component), m_Child(child)
  {
  }

  EVENT_CLASS_TYPE(HIERACHY_COMPONENT_CHILD_REMOVE)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new ChildRemovedEvent(entity, component, m_Child);
  }

  Entity GetChild() const
  {
    return m_Child;
  }

 private:
  Entity m_Child;
};

};  // namespace mite

#endif
