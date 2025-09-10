#ifndef MITE_SCENE_CORE_EVENT
#define MITE_SCENE_CORE_EVENT

#include "component.h"
#include "component_id.h"
#include "entity.h"

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

// 3. 组件事件	=====================================================
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
///**
// * @class ComponentChangedEvent
// * @brief 组件替换事件
// */
// template<typename T> class ComponentChangedEvent : public ComponentEvent<T> {
// public:
//  ComponentChangedEvent(Entity entity, T &newComponent, T &oldComponent)
//      : ComponentEvent<T>(entity, newComponent), oldComponent(oldComponent)
//  {
//  }
//  T &GetOldComponent()
//  {
//    return oldComponent;
//  }
//  EVENT_CLASS_TYPE(COMPONENT_CHANGED)
//  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
//  Event *Clone() const override
//  {
//    return new ComponentChangedEvent<T>(entity, component, oldComponent);
//  }
//
// private:
//  T &oldComponent;  // 组件
//};
};  // namespace mite

#endif
