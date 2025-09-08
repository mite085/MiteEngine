#ifndef MITE_EDITOR_UI_EVENTS_H
#define MITE_EDITOR_UI_EVENTS_H

#include "ui_event.h"
#include "scene_core/entity.h"

namespace mite {

/**
 * @brief 场景保存事件
 */
class SceneSaveEvent : public UIEvent {
 public:
  explicit SceneSaveEvent(const std::string &filePath) : m_FilePath(filePath) {}

  const std::string &GetFilePath() const
  {
    return m_FilePath;
  }

  std::string ToString() const override
  {
    return "SceneSaveEvent: " + m_FilePath;
  }

  Event *Clone() const override
  {
    return new SceneSaveEvent(m_FilePath);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  std::string m_FilePath;
};

/**
 * @brief 场景加载事件
 */
class SceneLoadEvent : public UIEvent {
 public:
  explicit SceneLoadEvent(const std::string &filePath) : m_FilePath(filePath) {}

  const std::string &GetFilePath() const
  {
    return m_FilePath;
  }

  std::string ToString() const override
  {
    return "SceneLoadEvent: " + m_FilePath;
  }

  Event *Clone() const override
  {
    return new SceneLoadEvent(m_FilePath);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  std::string m_FilePath;
};

/**
 * @brief 视口渲染事件
 */
class ViewportRenderEvent : public UIEvent {
 public:
  explicit ViewportRenderEvent(UUID viewportId, const glm::vec2 &size)
      : m_ViewportId(viewportId), m_Size(size)
  {
  }

  UUID GetSourceWidgetID() const override
  {
    return m_ViewportId;
  }
  glm::vec2 GetSize() const
  {
    return m_Size;
  }

  std::string ToString() const override
  {
    return "ViewportRenderEvent: ID " + UUIDGenerator::UUIDToString(m_ViewportId) +
           " Size: " + std::to_string(m_Size.x) + "x" + std::to_string(m_Size.y);
  }

  Event *Clone() const override
  {
    return new ViewportRenderEvent(m_ViewportId, m_Size);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  UUID m_ViewportId;
  glm::vec2 m_Size;
};

/**
 * @brief 实体选择事件
 */
class EntitySelectedEvent : public UIEvent {
 public:
  explicit EntitySelectedEvent(Entity entity, const std::string &entityName = "")
      : m_Entity(entity), m_EntityName(entityName)
  {
  }

  Entity GetEntity() const
  {
    return m_Entity;
  }
  const std::string &GetEntityName() const
  {
    return m_EntityName;
  }

  std::string ToString() const override
  {
    return "EntitySelectedEvent: " + m_EntityName + " (ID: " + m_Entity.GetUUIDString() + ")";
  }

  Event *Clone() const override
  {
    return new EntitySelectedEvent(m_Entity, m_EntityName);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  Entity m_Entity;
  std::string m_EntityName;
};

/**
 * @brief 实体取消选择事件
 */
class EntityDeselectedEvent : public UIEvent {
 public:
  explicit EntityDeselectedEvent(Entity entity) : m_Entity(entity) {}

  Entity GetEntity() const
  {
    return m_Entity;
  }

  std::string ToString() const override
  {
    return "EntityDeselectedEvent: ID " + m_Entity.GetUUIDString();
  }

  Event *Clone() const override
  {
    return new EntityDeselectedEvent(m_Entity);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  Entity m_Entity;
};

/**
 * @brief Gizmo操作事件
 */
class GizmoOperationEvent : public UIEvent {
 public:
  enum class OperationType { TRANSLATE, ROTATE, SCALE, NONE };

  explicit GizmoOperationEvent(OperationType operation, const glm::vec3 &delta)
      : m_Operation(operation), m_Delta(delta)
  {
  }

  OperationType GetOperation() const
  {
    return m_Operation;
  }
  glm::vec3 GetDelta() const
  {
    return m_Delta;
  }

  std::string ToString() const override
  {
    std::string opStr;
    switch (m_Operation) {
      case OperationType::TRANSLATE:
        opStr = "TRANSLATE";
        break;
      case OperationType::ROTATE:
        opStr = "ROTATE";
        break;
      case OperationType::SCALE:
        opStr = "SCALE";
        break;
      default:
        opStr = "NONE";
        break;
    }
    return "GizmoOperationEvent: " + opStr + " Delta: (" + std::to_string(m_Delta.x) + ", " +
           std::to_string(m_Delta.y) + ", " + std::to_string(m_Delta.z) + ")";
  }

  Event *Clone() const override
  {
    return new GizmoOperationEvent(m_Operation, m_Delta);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  OperationType m_Operation;
  glm::vec3 m_Delta;
};



}  // namespace mite

#endif  // MITE_EDITOR_UI_EVENTS_H
