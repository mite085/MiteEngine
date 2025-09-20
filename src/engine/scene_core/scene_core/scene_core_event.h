#ifndef MITE_SCENE_CORE_EVENT
#define MITE_SCENE_CORE_EVENT

#include "component.h"
#include "component_id.h"
#include "entity.h"
#include "subscription_group.h"

namespace mite {
// 1. 场景事件	=====================================================

/**
 * @class SceneLoadedEvent
 * @brief 场景加载事件
 */
class SceneLoadedEvent : public Event {
 public:
  SceneLoadedEvent() {}

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new SceneLoadedEvent();
  }
};
/**
 * @class SceneLoadedEvent
 * @brief 场景清空事件
 */
class SceneClearedEvent : public Event {
 public:
  SceneClearedEvent() {}

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new SceneClearedEvent();
  }
};

// 2. 实体事件	=====================================================

/**
 * @class EntityEvent
 * @brief 实体事件基类(抽象类)
 */
class EntityEvent : public Event {
 public:
  EntityEvent(Entity entity) : entity(entity) {}
  Entity GetEntity()
  {
    return entity;
  }

 protected:
  Entity entity;  // 关联的实体
};

/**
 * @class EntityCreatedEvent
 * @brief 创建实体事件
 */
class EntityCreatedEvent : public EntityEvent {
 public:
  EntityCreatedEvent(Entity entity) : EntityEvent(entity) {}

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new EntityCreatedEvent(entity);
  }
};
/**
 * @class EntityPreDestroyedEvent
 * @brief 销毁实体事件
 */
class EntityDestroyedEvent : public EntityEvent {
 public:
  EntityDestroyedEvent(Entity entity) : EntityEvent(entity) {}

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new EntityDestroyedEvent(entity);
  }
};

/**
 * @class EntityParentChangedEvent
 * @brief 实体Parent修改事件
 */
class EntityParentChangedEvent : public EntityEvent {
 public:
  EntityParentChangedEvent(Entity entity) : EntityEvent(entity) {}

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new EntityParentChangedEvent(entity);
  }
};
/**
 * @class EntityTagChangedEvent
 * @brief 实体Tag修改事件
 */
class EntityTagChangedEvent : public EntityEvent {
 public:
  EntityTagChangedEvent(Entity entity) : EntityEvent(entity) {}

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new EntityTagChangedEvent(entity);
  }
};

// 3. 组件生命周期事件	=====================================================
/**
 * @class ComponentEvent
 * @brief 组件事件基类(抽象类)
 */
template<typename T> class ComponentEvent : public Event {
 public:
  ComponentEvent(Entity entity, T &component)
      : entity(entity), component(component), id(ComponentID::Get<T>())
  {
  }
  Entity GetEntity()
  {
    return entity;
  }
  T &GetComponent()
  {
    return component;
  }
 protected:
  Entity entity;   // 关联的实体
  T &component;    // 组件
  ComponentID id;  // 组件类型标识符
};

/**
 * @class ComponentAddedEvent
 * @brief 组件添加事件
 */
template<typename T> class ComponentAddedEvent : public ComponentEvent<T> {
 public:
  ComponentAddedEvent(Entity entity, T &component) : ComponentEvent<T>(entity, component) {}

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)

  Event *Clone() const override
  {
    return new ComponentAddedEvent<T>(entity, component);
  }
};

/**
 * @class ComponentRemovedEvent
 * @brief 组件删除事件
 */
template<typename T> class ComponentRemovedEvent : public ComponentEvent<T> {
 public:
  ComponentRemovedEvent(Entity entity, T &component) : ComponentEvent<T>(entity, component) {}

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new ComponentRemovedEvent<T>(entity, component);
  }
};

// 4. 组件快照事件	=====================================================
/**
 * @brief 快照应用事件模板类
 * @tparam DataT 组件数据类型
 * 
 * 用于在事件总线中传递快照应用请求，实现解耦的快照应用机制
 */
template<typename DataT> class ApplySnapshotEvent : public Event {
 public:
  /**
   * @brief 构造函数
   * @param entityId 目标实体ID
   * @param data 要应用的快照数据
   */
  ApplySnapshotEvent(Entity entityId, const DataT &data) : entityId(entityId), snapshotData(data)
  {
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)

  /**
   * @brief 事件克隆方法
   * @return Event* 克隆的事件对象
   */
  Event *Clone() const override
  {
    return new ApplySnapshotEvent<DataT>(*this);
  }
  Entity GetEntity() {
    return entityId;
  }
  DataT& GetData() {
    return snapshotData;
  }

 private:
  Entity entityId;   // 目标实体标识符
  DataT snapshotData;  // 要应用的快照数据
};

};  // namespace mite

#endif
