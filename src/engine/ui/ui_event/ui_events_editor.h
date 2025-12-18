#ifndef MITE_EDITOR_UI_EVENTS_H
#define MITE_EDITOR_UI_EVENTS_H

#include "ui_event.h"
#include "scene_core/entity.h"

namespace mite {

/**
 * @brief 场景保存事件
 */
class SceneSaveEvent : public Event {
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
class SceneLoadEvent : public Event {
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
 * @brief 实体选择事件
 */
class EntitySelectedEvent : public Event {
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
class EntityDeselectedEvent : public Event {
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
class GizmoOperationEvent : public Event {
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

/**
 * @brief 模态文件对话框选择文件事件
 */
class FileSelectedEvent : public Event {
 public:
  explicit FileSelectedEvent(const std::string &filePathName,
                             const std::string &fileName,
                             const std::string &filePath)
      : m_FilePathName(filePathName), m_FileName(fileName), m_FilePath(filePath)
  {
  }

  // 支持完整文件路径+文件名、文件名、文件路径三种读取模式
  const std::string &GetFilePathName() const { return m_FilePathName; }
  const std::string &GetFileName() const { return m_FileName; }
  const std::string &GetFilePath() const { return m_FilePath; }

  std::string ToString() const override
  {
    return "FileSelectedEvent: " + m_FilePathName;
  }

  Event *Clone() const override
  {
    return new FileSelectedEvent(m_FilePathName, m_FileName, m_FilePath);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  Entity m_Entity;
  std::string m_FilePathName, m_FileName, m_FilePath;
};

}  // namespace mite

#endif  // MITE_EDITOR_UI_EVENTS_H
