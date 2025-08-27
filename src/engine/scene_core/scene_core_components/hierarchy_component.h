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
class HierarchyComponent
    : public ComponentTraits<HierarchyComponent, Component::Family::Hierarchy> {
 public:
  /**
   * @brief 构造函数（创建无父节点的根实体）
   */
  HierarchyComponent();
  ~HierarchyComponent() override = default;

  // 禁止拷贝与移动
  HierarchyComponent(const HierarchyComponent &) = delete;
  HierarchyComponent &operator=(const HierarchyComponent &) = delete;
  HierarchyComponent(HierarchyComponent &&) = delete;
  HierarchyComponent &operator=(HierarchyComponent &&) = delete;

  /**
   * @brief 处理脏标记，主要处理层级关系变化
   */
  void ProcessDirty(float deltaTime, SceneRegistry &reg) override;

  // ==================== 查询接口 ====================

  Entity GetParent() const;                        // 获取父实体句柄
  const std::vector<Entity> &GetChildren() const;  // 获取所有子实体句柄
  size_t GetChildCount() const;                    // 获取子实体数量
  bool IsLeaf() const;                             // 检查是否为叶节点（无子节点）
  bool IsRoot() const;                             // 检查是否为根节点（无父节点）
  size_t GetDepth(SceneRegistry &registry);        // 获取深度（距离根节点的层级数）

  // ==================== 操作接口 ====================

  bool SetParent(SceneRegistry &registry, Entity newParent);  // 设置父节点
  bool AddChild(SceneRegistry &registry, Entity child);       // 添加子节点
  bool RemoveChild(SceneRegistry &registry, Entity child);    // 移除子节点
  void ClearChildren(SceneRegistry &registry);                // 清空所有子节点

 private:
  // ==================== 内部方法 ====================
  bool ValidateHierarchy(SceneRegistry &registry, Entity newParent) const;
  void UpdateTransformDirtyState(SceneRegistry &registry);

 private:
  Entity m_Parent;                 // 父实体句柄
  std::vector<Entity> m_Children;  // 子实体列表
};

// ==================== 组件系统 ====================

class HierarchyComponentSystem : public DirtyComponentSystem<HierarchyComponent> {
  DECLARE_COMPONENT_SYSTEM(HierarchyComponentSystem)
 public:
  void Initialize() override;
  void Shutdown() override;

  std::vector<std::type_index> GetSystemDependencies() const override;

 private:
  // 组件添加与移除事件响应函数重写：
  // 后续SceneGraph模块的TransformSceneNodeSystem负责处理Entity和SceneNode的Transform同步，不应当阻断事件传播
  bool OnComponentAdded(ComponentAddedEvent<HierarchyComponent> &e) override;
  bool OnComponentRemoved(ComponentRemovedEvent<HierarchyComponent> &e) override;
  void ProcessDirtyComponents(float deltaTime, SceneRegistry &registry) override;

  /**
   * @brief 验证并修复层级关系完整性
   */
  void ValidateAndRepairHierarchy(SceneRegistry &registry);
  /**
   * @brief 处理延迟的组件移除操作
   */
  void ProcessPendingRemovals(SceneRegistry &registry);

  private:
  // 待处理的组件移除操作队列
   struct PendingRemoval {
     Entity entity;
     Entity parent;
     std::vector<Entity> children;

     PendingRemoval(Entity entity, Entity parent, const std::vector<Entity> &children)
         : entity(entity), parent(parent), children(children)
     {
     }
   };

   std::vector<PendingRemoval> m_pendingRemovals;
   std::mutex m_removalMutex;
};
// ==================== 事件定义 ====================
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
};  // namespace mite

#endif
