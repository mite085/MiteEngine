#ifndef MITE_SCENE_TRANSFORM_COMPONENT
#define MITE_SCENE_TRANSFORM_COMPONENT

#include "scene_core/component_system.h"
#include "basic_data/transform.h"

namespace mite {
// 前向声明
class SceneRegistry;
/**
 * @brief 变换组件，管理实体的位置、旋转和缩放
 *
 * 功能特性：
 * 1. 支持局部空间和世界空间变换
 * 2. 高效的矩阵缓存和更新机制
 * 3. 提供常用的变换操作接口
 * 4. 与HierarchyComponent协同工作处理层次结构
 *
 * 设计考虑：
 * - 使用右手坐标系，Y轴向上
 * - 旋转使用四元数存储避免万向节锁
 * - 提供脏标记系统优化矩阵计算
 */
class TransformComponent
    : public ComponentTraits<TransformComponent, Component::Family::Transform> {
 public:
  /**
   * @brief 默认构造函数
   */
  TransformComponent();

  /**
   * @brief 带初始值的构造函数
   * @param position 位置坐标
   * @param rotation 旋转
   * @param scale 缩放
   */
  explicit TransformComponent(const glm::vec3 &position,
                              const glm::vec3 &rotation = glm::vec3(0.0f),
                              const glm::vec3 &scale = glm::vec3(1.0f));

  /**
   * @brief 使用变换矩阵的构造函数
   * @param matrix 变换矩阵
   */
  explicit TransformComponent(const glm::mat4 &matrix);

  ~TransformComponent() override = default;

  /**
   * @brief 针对dirty对象进行处理
   */
  void ProcessDirty(float deltaTime, SceneRegistry &reg) override;

// ==================== 位置操作 ====================
  const glm::vec3 &GetLocalPosition() const;
  void SetLocalPosition(const glm::vec3 &position);
  glm::vec3 GetWorldPosition(SceneRegistry &reg) const;
  void SetWorldPosition(SceneRegistry &reg, const glm::vec3 &position);

  // ==================== 旋转操作（欧拉角deg、四元数） ====================
  glm::vec3 GetLocalRotation() const;
  glm::quat GetLocalRotationQuat() const;
  void SetLocalRotation(const glm::vec3 &rotation);
  void SetLocalRotation(float x, float y, float z);
  void SetLocalRotationQuat(const glm::quat &rotation);
  glm::vec3 GetWorldRotation(SceneRegistry &reg) const;
  void SetWorldRotation(SceneRegistry &reg, const glm::vec3 &rotation);
  void SetWorldRotationQuat(SceneRegistry &reg, const glm::quat &rotation);
  void Rotate(const glm::vec3 &axis, float angle);
  void RotateAround(SceneRegistry &reg,
                    const glm::vec3 &point,
                    const glm::vec3 &axis,
                    float angle);
  void LookAt(SceneRegistry &reg,
              const glm::vec3 &target,
              const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f));

  // ==================== 缩放操作 ====================
  const glm::vec3 &GetLocalScale() const;
  void SetLocalScale(const glm::vec3 &scale);
  void SetLocalScale(float scale);
  glm::vec3 GetWorldScale(SceneRegistry &reg) const;

   // ==================== 矩阵操作 ====================
  glm::mat4 GetLocalMatrix() const;
  glm::mat4 GetWorldMatrix(SceneRegistry &reg) const;
  void SetLocalMatrix(const glm::mat4 &matrix);
  void SetWorldMatrix(SceneRegistry &reg, const glm::mat4 &matrix);

  // 方向向量 ==============================================
  glm::vec3 Forward() const;  // 正Z轴方向
  glm::vec3 Up() const;       // 正Y轴方向
  glm::vec3 Right() const;    // 正X轴方向

  // ==================== 组件接口 ====================
  std::vector<std::type_index> GetDependencies() const override;
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

  /**
   * @brief 获取底层Transform对象（只读）
   */
  const Transform &GetTransform() const;
  /**
   * @brief 获取底层Transform对象（可修改）
   * @note 修改后需要手动调用MarkDirty()
   */
  Transform &GetTransform();
  /**
   * @brief 标记层级脏标记（当父节点变换改变时调用）
   */
  void MarkHierarchyDirty();
  /**
   * @brief 检查是否需要层级更新
   */
  bool IsHierarchyDirty() const;

 private:
  /**
   * @brief 计算世界变换矩阵(递归)
   */
  void UpdateWorldMatrix(SceneRegistry &reg) const;
  /**
   * @brief 从四元数转换为欧拉角（度）
   */
  static glm::vec3 QuatToEulerDegrees(const glm::quat &quat);
  /**
   * @brief 从欧拉角（度）转换为四元数
   */
  static glm::quat EulerDegreesToQuat(const glm::vec3 &euler);

 private:
  Transform m_Transform;                              // 基础变换对象
  mutable glm::mat4 m_WorldMatrix = glm::mat4(1.0f);  // 世界变换矩阵缓存
  mutable bool m_HierarchyDirty = true;               // 层级脏标记（父节点变换改变）
};

// ==================== 组件系统 ====================
class TransformComponentSystem : public DirtyComponentSystem<TransformComponent> {
  DECLARE_COMPONENT_SYSTEM(TransformComponentSystem)
 
  std::vector<std::type_index> GetSystemDependencies() const override;

 private:
  // 组件添加与移除事件响应函数重写：
  // 后续SceneGraph模块的TransformSceneNodeSystem负责处理Entity和SceneNode的Transform同步，不应当阻断事件传播
  bool OnComponentAdded(ComponentAddedEvent<TransformComponent> &e) override;
  bool OnComponentRemoved(ComponentRemovedEvent<TransformComponent> &e) override;
  void ProcessDirtyComponents(float deltaTime, SceneRegistry &registry) override;
};
// ==================== 事件定义 ====================
/**
 * @class TransformUpdatedEvent
 * @brief Transform组件替换事件
 */
class TransformUpdatedEvent : public ComponentEvent<TransformComponent> {
 public:
  TransformUpdatedEvent(Entity entity, TransformComponent &component)
      : ComponentEvent<TransformComponent>(entity, component)
  {
  }
  EVENT_CLASS_TYPE(TRANSFORM_COMPONENT_UPDATE)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new TransformUpdatedEvent(entity, component);
  }
};
/**
 * @class PositionChangedEvent
 * @brief 位置改变事件
 */
class PositionChangedEvent : public ComponentEvent<TransformComponent> {
 public:
  PositionChangedEvent(Entity entity,
                       TransformComponent &component,
                       glm::vec3 newPosition,
                       bool isWorldSpace)
      : ComponentEvent<TransformComponent>(entity, component),
        m_NewPosition(newPosition),
        m_IsWorldSpace(isWorldSpace)
  {
  }

  EVENT_CLASS_TYPE(TRANSFORM_COMPONENT_POSITION_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new PositionChangedEvent(entity, component, m_NewPosition, m_IsWorldSpace);
  }
  bool IsWorldSpace()
  {
    return m_IsWorldSpace;
  }
 private:
  glm::vec3 m_NewPosition;
  bool m_IsWorldSpace;
};
/**
 * @class RotationChangedEvent
 * @brief 旋转改变事件
 */
class RotationChangedEvent : public ComponentEvent<TransformComponent> {
 public:
  RotationChangedEvent(Entity entity,
                       TransformComponent &component,
                       glm::quat newRotation,
                       bool isWorldSpace)
      : ComponentEvent<TransformComponent>(entity, component),
        m_NewRotation(newRotation),
        m_IsWorldSpace(isWorldSpace)
  {
  }
  bool IsWorldSpace()
  {
    return m_IsWorldSpace;
  }
  EVENT_CLASS_TYPE(TRANSFORM_COMPONENT_ROTATION_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new RotationChangedEvent(entity, component, m_NewRotation, m_IsWorldSpace);
  }

 private:
  glm::quat m_NewRotation;
  bool m_IsWorldSpace;
};
/**
 * @class ScaleChangedEvent
 * @brief 缩放改变事件
 */
class ScaleChangedEvent : public ComponentEvent<TransformComponent> {
 public:
  ScaleChangedEvent(Entity entity,
                    TransformComponent &component,
                    glm::vec3 newPosition,
                    bool isWorldSpace)
      : ComponentEvent<TransformComponent>(entity, component),
        m_NewScale(newPosition),
        m_IsWorldSpace(isWorldSpace)
  {
  }
  bool IsWorldSpace()
  {
    return m_IsWorldSpace;
  }
  EVENT_CLASS_TYPE(TRANSFORM_COMPONENT_SCALE_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new ScaleChangedEvent(entity, component, m_NewScale, m_IsWorldSpace);
  }

 private:
  glm::vec3 m_NewScale;
  bool m_IsWorldSpace;
};
/**
 * @class TransformChangedEvent
 * @brief 变换改变事件
 */
class TransformChangedEvent : public ComponentEvent<TransformComponent> {
 public:
  TransformChangedEvent(Entity entity,
                        TransformComponent &component,
                        glm::mat4 newMatrix,
                        bool isWorldSpace)
      : ComponentEvent<TransformComponent>(entity, component),
        newMatrix(newMatrix),
        isWorldSpace(isWorldSpace)
  {
  }
  bool IsWorldSpace()
  {
    return isWorldSpace;
  }
  EVENT_CLASS_TYPE(TRANSFORM_COMPONENT_TRANSFORM_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new TransformChangedEvent(entity, component, newMatrix, isWorldSpace);
  }

 private:
  glm::mat4 newMatrix;
  bool isWorldSpace;
};
};  // namespace mite

#endif
