#ifndef MITE_SCENE_EVENT
#define MITE_SCENE_EVENT

#include "component_id.h"
#include "entity.h"
#include "headers/headers.h"

namespace mite {
// 1. 场景事件	=====================================================

/**
 * @class SceneLoadedEvent
 * @brief 场景加载事件
 */
class SceneLoadedEvent : public Event {
 public:
  SceneLoadedEvent() {}

  EVENT_CLASS_TYPE(SCENE_LOADED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
};
/**
 * @class SceneLoadedEvent
 * @brief 场景清空事件
 */
class SceneClearedEvent : public Event {
 public:
  SceneClearedEvent() {}

  EVENT_CLASS_TYPE(SCENE_CLEARED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
};

// 2. 实体事件	=====================================================

/**
 * @class EntityEvent
 * @brief 实体事件基类(抽象类)
 */
class EntityEvent : public Event {
 public:
  EntityEvent(Entity entity) : entity(entity) {}

  virtual EventType GetEventType() const = 0;
  virtual const char *GetName() const = 0;
  virtual int GetCategoryFlags() const = 0;

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

  EVENT_CLASS_TYPE(ENTITY_CREATED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
};
/**
 * @class EntityDestroyedEvent
 * @brief 销毁实体事件
 */
class EntityDestroyedEvent : public EntityEvent {
 public:
  EntityDestroyedEvent(Entity entity) : EntityEvent(entity) {}

  EVENT_CLASS_TYPE(ENTITY_DESTROYED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
};
/**
 * @class EntityParentChangedEvent
 * @brief 实体Parent修改事件
 */
class EntityParentChangedEvent : public EntityEvent {
 public:
  EntityParentChangedEvent(Entity entity) : EntityEvent(entity) {}

  EVENT_CLASS_TYPE(PARENT_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
};
/**
 * @class EntityTagChangedEvent
 * @brief 实体Tag修改事件
 */
class EntityTagChangedEvent : public EntityEvent {
 public:
  EntityTagChangedEvent(Entity entity) : EntityEvent(entity) {}

  EVENT_CLASS_TYPE(PARENT_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
};

// 3. 组件事件	=====================================================
/**
 * @class ComponentEvent
 * @brief 组件事件基类(抽象类)
 */
template<typename T> class ComponentEvent : public Event {
 public:
  ComponentEvent(Entity entity) : entity(entity), id(ComponentID::Get<T>()) {}
  Entity GetEntity()
  {
    return entity;
  }

  virtual EventType GetEventType() const = 0;
  virtual const char *GetName() const = 0;
  virtual int GetCategoryFlags() const = 0;

 protected:
  Entity entity;   // 关联的实体
  ComponentID id;  // 组件类型标识符
};

/**
 * @class ComponentAddedEvent
 * @brief 组件添加事件
 */
template<typename T> class ComponentAddedEvent : public ComponentEvent<T> {
 public:
  ComponentAddedEvent(Entity entity) : ComponentEvent<T>(entity) {}

  EVENT_CLASS_TYPE(COMPONENT_ADDED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
};

/**
 * @class ComponentRemovedEvent
 * @brief 组件删除事件
 */
template<typename T> class ComponentRemovedEvent : public ComponentEvent<T> {
 public:
  ComponentRemovedEvent(Entity entity) : ComponentEvent<T>(entity) {}

  EVENT_CLASS_TYPE(COMPONENT_REMOVED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)

};
/**
 * @class ComponentChangedEvent
 * @brief 组件替换事件
 */
template<typename T> class ComponentChangedEvent : public ComponentEvent<T> {
 public:
  ComponentChangedEvent(Entity entity) : ComponentEvent<T>(entity) {}

  EVENT_CLASS_TYPE(COMPONENT_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
};

};  // namespace mite

#endif
