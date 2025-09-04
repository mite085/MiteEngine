#ifndef MITE_EDITOR_UI_EVENTS_H
#define MITE_EDITOR_UI_EVENTS_H

#include "ui_event.h"

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
  explicit ViewportRenderEvent(uint64_t viewportId, const glm::vec2 &size)
      : m_ViewportId(viewportId), m_Size(size)
  {
  }

  uint64_t GetSourceWidgetID() const override
  {
    return m_ViewportId;
  }
  glm::vec2 GetSize() const
  {
    return m_Size;
  }

  std::string ToString() const override
  {
    return "ViewportRenderEvent: ID " + std::to_string(m_ViewportId) +
           " Size: " + std::to_string(m_Size.x) + "x" + std::to_string(m_Size.y);
  }

  Event *Clone() const override
  {
    return new ViewportRenderEvent(m_ViewportId, m_Size);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  uint64_t m_ViewportId;
  glm::vec2 m_Size;
};

/**
 * @brief 实体选择事件
 */
class EntitySelectedEvent : public UIEvent {
 public:
  explicit EntitySelectedEvent(uint64_t entityId, const std::string &entityName = "")
      : m_EntityId(entityId), m_EntityName(entityName)
  {
  }

  uint64_t GetEntityId() const
  {
    return m_EntityId;
  }
  const std::string &GetEntityName() const
  {
    return m_EntityName;
  }

  std::string ToString() const override
  {
    return "EntitySelectedEvent: " + m_EntityName + " (ID: " + std::to_string(m_EntityId) + ")";
  }

  Event *Clone() const override
  {
    return new EntitySelectedEvent(m_EntityId, m_EntityName);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  uint64_t m_EntityId;
  std::string m_EntityName;
};

/**
 * @brief 实体取消选择事件
 */
class EntityDeselectedEvent : public UIEvent {
 public:
  explicit EntityDeselectedEvent(uint64_t entityId) : m_EntityId(entityId) {}

  uint64_t GetEntityId() const
  {
    return m_EntityId;
  }

  std::string ToString() const override
  {
    return "EntityDeselectedEvent: ID " + std::to_string(m_EntityId);
  }

  Event *Clone() const override
  {
    return new EntityDeselectedEvent(m_EntityId);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  uint64_t m_EntityId;
};

/**
 * @brief 组件添加事件
 */
class ComponentAddedEvent : public UIEvent {
 public:
  explicit ComponentAddedEvent(uint64_t entityId, const std::string &componentType)
      : m_EntityId(entityId), m_ComponentType(componentType)
  {
  }

  uint64_t GetEntityId() const
  {
    return m_EntityId;
  }
  const std::string &GetComponentType() const
  {
    return m_ComponentType;
  }

  std::string ToString() const override
  {
    return "ComponentAddedEvent: Entity " + std::to_string(m_EntityId) + " + " + m_ComponentType;
  }

  Event *Clone() const override
  {
    return new ComponentAddedEvent(m_EntityId, m_ComponentType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  uint64_t m_EntityId;
  std::string m_ComponentType;
};

/**
 * @brief 组件移除事件
 */
class ComponentRemovedEvent : public UIEvent {
 public:
  explicit ComponentRemovedEvent(uint64_t entityId, const std::string &componentType)
      : m_EntityId(entityId), m_ComponentType(componentType)
  {
  }

  uint64_t GetEntityId() const
  {
    return m_EntityId;
  }
  const std::string &GetComponentType() const
  {
    return m_ComponentType;
  }

  std::string ToString() const override
  {
    return "ComponentRemovedEvent: Entity " + std::to_string(m_EntityId) + " - " + m_ComponentType;
  }

  Event *Clone() const override
  {
    return new ComponentRemovedEvent(m_EntityId, m_ComponentType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  uint64_t m_EntityId;
  std::string m_ComponentType;
};

/**
 * @brief 组件修改事件
 */
class ComponentModifiedEvent : public UIEvent {
 public:
  explicit ComponentModifiedEvent(uint64_t entityId,
                                  const std::string &componentType,
                                  const std::string &propertyName)
      : m_EntityId(entityId), m_ComponentType(componentType), m_PropertyName(propertyName)
  {
  }

  uint64_t GetEntityId() const
  {
    return m_EntityId;
  }
  const std::string &GetComponentType() const
  {
    return m_ComponentType;
  }
  const std::string &GetPropertyName() const
  {
    return m_PropertyName;
  }

  std::string ToString() const override
  {
    return "ComponentModifiedEvent: Entity " + std::to_string(m_EntityId) + " " + m_ComponentType +
           "." + m_PropertyName;
  }

  Event *Clone() const override
  {
    return new ComponentModifiedEvent(m_EntityId, m_ComponentType, m_PropertyName);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  uint64_t m_EntityId;
  std::string m_ComponentType;
  std::string m_PropertyName;
};

/**
 * @brief 材质编辑事件
 */
class MaterialEditEvent : public UIEvent {
 public:
  explicit MaterialEditEvent(uint64_t materialId,
                             const std::string &propertyName,
                             const std::string &newValue)
      : m_MaterialId(materialId), m_PropertyName(propertyName), m_NewValue(newValue)
  {
  }

  uint64_t GetMaterialId() const
  {
    return m_MaterialId;
  }
  const std::string &GetPropertyName() const
  {
    return m_PropertyName;
  }
  const std::string &GetNewValue() const
  {
    return m_NewValue;
  }

  std::string ToString() const override
  {
    return "MaterialEditEvent: Material " + std::to_string(m_MaterialId) + " " + m_PropertyName +
           " = " + m_NewValue;
  }

  Event *Clone() const override
  {
    return new MaterialEditEvent(m_MaterialId, m_PropertyName, m_NewValue);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  uint64_t m_MaterialId;
  std::string m_PropertyName;
  std::string m_NewValue;
};

/**
 * @brief 资源导入事件
 */
class AssetImportEvent : public UIEvent {
 public:
  explicit AssetImportEvent(const std::string &filePath, const std::string &assetType)
      : m_FilePath(filePath), m_AssetType(assetType)
  {
  }

  const std::string &GetFilePath() const
  {
    return m_FilePath;
  }
  const std::string &GetAssetType() const
  {
    return m_AssetType;
  }

  std::string ToString() const override
  {
    return "AssetImportEvent: " + m_AssetType + " from " + m_FilePath;
  }

  Event *Clone() const override
  {
    return new AssetImportEvent(m_FilePath, m_AssetType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  std::string m_FilePath;
  std::string m_AssetType;
};

/**
 * @brief 资源删除事件
 */
class AssetDeleteEvent : public UIEvent {
 public:
  explicit AssetDeleteEvent(uint64_t assetId, const std::string &assetType)
      : m_AssetId(assetId), m_AssetType(assetType)
  {
  }

  uint64_t GetAssetId() const
  {
    return m_AssetId;
  }
  const std::string &GetAssetType() const
  {
    return m_AssetType;
  }

  std::string ToString() const override
  {
    return "AssetDeleteEvent: " + m_AssetType + " ID " + std::to_string(m_AssetId);
  }

  Event *Clone() const override
  {
    return new AssetDeleteEvent(m_AssetId, m_AssetType);
  }

  EVENT_CLASS_CATEGORY(UI_EVENT_CATEGORY_EDITOR)

 private:
  uint64_t m_AssetId;
  std::string m_AssetType;
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
