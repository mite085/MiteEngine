#ifndef MITE_SCENE_TRANSFORM_COMPONENT
#define MITE_SCENE_TRANSFORM_COMPONENT

#include "scene_core/component_system.h"

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
class TransformComponent : public ComponentTraits<TransformComponent, Component::Family::Core> {
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
                              const glm::quat &rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
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
  void ProcessDirty(SceneRegistry &reg) override;

  // 位置操作 ==============================================
  /**
   * @brief 获取局部空间位置
   * @return 局部空间三维位置坐标
   */
  const glm::vec3 &GetLocalPosition() const;
  /**
   * @brief 设定局部空间位置
   * @param position 局部空间三维位置坐标
   */
  void SetLocalPosition(const glm::vec3 &position);

  /**
   * @brief 获取世界空间位置(需要父变换计算)
   * @return 世界空间三维位置坐标
   */
  glm::vec3 GetWorldPosition(SceneRegistry &reg) const;
  /**
   * @brief 设定世界空间位置
   * @param position 世界空间三维位置坐标
   */
  void SetWorldPosition(SceneRegistry &reg, const glm::vec3 &position);

  // 旋转操作 ==============================================
  /**
   * @brief 获取局部空间旋转(四元数)
   * @return 局部空间旋转四元数
   */
  const glm::quat &GetLocalRotation() const;
  /**
   * @brief 设定局部空间旋转(四元数)
   * @param rotation 局部空间旋转四元数
   */
  void SetLocalRotation(const glm::quat &rotation);

  /**
   * @brief 获取局部空间旋转(欧拉角)(弧度制)
   * @return 局部空间旋转(欧拉角)(弧度制)
   */
  glm::vec3 GetLocalEulerAngles() const;
  /**
   * @brief 设定局部空间旋转(欧拉角)(弧度制)
   * @param eulerAngles 局部空间旋转(欧拉角)(弧度制)
   */
  void SetLocalEulerAngles(const glm::vec3 &eulerAngles);
  /**
   * @brief 设定局部空间旋转(欧拉角)(弧度制)
   * @param x 局部空间旋转欧拉角x分量(弧度制)
   * @param y 局部空间旋转欧拉角y分量(弧度制)
   * @param z 局部空间旋转欧拉角z分量(弧度制)
   */
  void SetLocalEulerAngles(float x, float y, float z);

  /**
   * @brief 获取世界空间旋转(四元数)
   * @return 世界空间旋转四元数
   */
  glm::quat GetWorldRotation(SceneRegistry &reg) const;
  /**
   * @brief 设定世界空间旋转(四元数)
   * @param rotation 世界空间旋转四元数
   */
  void SetWorldRotation(SceneRegistry &reg, const glm::quat &rotation);

  // 旋转方法
  /**
   * @brief 旋转实体
   * @param rotation 局部空间旋转四元数
   */
  void Rotate(const glm::quat &rotation);
  /**
   * @brief 绕指定轴旋转实体
   * @param axis 旋转轴（单位向量）
   * @param angle 旋转角度(弧度制)
   *
   * 注意：
   * - 会自动标准化旋转轴
   * - 旋转顺序为局部空间
   */
  void Rotate(const glm::vec3 &axis, float angle);
  /**
   * @brief 绕空间中某一点旋转实体
   * @param point 旋转中心点（世界坐标）
   * @param axis 旋转轴（世界坐标）
   * @param angle 旋转角度（弧度）
   *
   * 实现原理：
   * 1. 计算从旋转中心到实体的向量
   * 2. 绕轴旋转该向量
   * 3. 更新实体位置
   * 4. 同时应用旋转到实体朝向
   *
   * 典型应用场景：
   * - 行星绕太阳公转
   * - 摄像机绕目标旋转
   */
  void RotateAround(SceneRegistry &reg,
                    const glm::vec3 &point,
                    const glm::vec3 &axis,
                    float angle);
  /**
   * @brief 旋转至看向某个目标
   * @param target 目标的世界空间三维位置坐标
   * @param up 观察的Up方向
   */
  void LookAt(SceneRegistry &reg,
              const glm::vec3 &target,
              const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f));

  // 缩放操作 ==============================================
  /**
   * @brief 获取局部空间缩放
   * @return 局部空间三轴缩放值
   */
  const glm::vec3 &GetLocalScale() const;
  /**
   * @brief 设定局部空间缩放
   * @param scale 局部空间三轴缩放值
   */
  void SetLocalScale(const glm::vec3 &scale);
  /**
   * @brief 设定局部空间缩放
   * @param scale 局部空间缩放值(三轴一致)
   */
  void SetLocalScale(float scale);

  /**
   * @brief 根据GetWorldMatrix近似计算世界空间缩放
   * @return 世界空间三轴缩放值
   */
  glm::vec3 GetWorldScale(SceneRegistry &reg) const;

  // 变换矩阵操作 ==========================================
  /**
   * @brief 获取局部空间变换矩阵
   * @return 局部空间变换矩阵
   */
  glm::mat4 GetLocalMatrix() const;
  /**
   * @brief 获取世界空间变换矩阵
   * @return 世界空间变换矩阵
   */
  glm::mat4 GetWorldMatrix(SceneRegistry &reg) const;
  /**
   * @brief 设定局部空间变换矩阵
   * @param matrix 局部空间变换矩阵
   */
  void SetLocalMatrix(const glm::mat4 &matrix);
  /**
   * @brief 设定世界空间变换矩阵
   * @param matrix 世界空间变换矩阵
   */
  void SetWorldMatrix(SceneRegistry &reg, const glm::mat4 &matrix);

  // 方向向量 ==============================================
  glm::vec3 Forward() const;  // 正Z轴方向
  glm::vec3 Up() const;       // 正Y轴方向
  glm::vec3 Right() const;    // 正X轴方向

  // 组件接口实现 ==========================================
  /**
   * @brief TransformComponent直接依赖于
   * 场景树核心组件HierarchyComponent。
   *
   * @return {typeid(HierarchyComponent)}
   */
  std::vector<std::type_index> GetDependencies() const override;
  /**
   * @brief TODO：序列化与反序列化
   * @param output
   * @return
   */
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

  /**
   * @brief 获取TransformComponent专属的脏标记
   * 标记位使用uint8_t节省内存
   */
  mutable uint8_t dirtyFlags : 3;
  static constexpr uint8_t LOCAL_DIRTY = 0x1;
  static constexpr uint8_t WORLD_DIRTY = 0x2;
  static constexpr uint8_t HIERARCHY_DIRTY = 0x4;

 private:
  /**
   * @brief 计算局部变换矩阵
   */
  void UpdateLocalMatrix() const;
  /**
   * @brief 计算世界变换矩阵(递归)
   */
  void UpdateWorldMatrix(SceneRegistry &reg) const;

 private:
  // 局部空间变换属性
  glm::vec3 m_Position = glm::vec3(0.0f);
  glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 m_Scale = glm::vec3(1.0f);

  // 矩阵缓存
  mutable glm::mat4 m_LocalMatrix = glm::mat4(1.0f);
  mutable glm::mat4 m_WorldMatrix = glm::mat4(1.0f);
};

// Transform组件系统--用于批量处理脏数据 =====================================================

class TransformSystem : public DirtyComponentSystem<TransformComponent> {
  DECLARE_COMPONENT_SYSTEM(TransformSystem)
 public:
  void Initialize(SceneRegistry &registry) override {}
  void Shutdown(SceneRegistry &registry) override {}

 private:
  void ProcessDirtyComponents(float deltaTime, SceneRegistry &registry) override;
};

// Transform组件事件--用于在数据变更时发布事件
// =====================================================

/**
 * @class TransformUpdatedEvent
 * @brief 变换更新事件
 */
class TransformUpdatedEvent : public ComponentEvent<TransformComponent> {
 public:
  TransformUpdatedEvent(Entity entity, TransformComponent &component)
      : ComponentEvent<TransformComponent>(entity, component)
  {
  }

  EVENT_CLASS_TYPE(COMPONENT_CHANGED)
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
        newPosition(newPosition),
        isWorldSpace(isWorldSpace)
  {
  }

  EVENT_CLASS_TYPE(COMPONENT_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new PositionChangedEvent(entity, component, newPosition, isWorldSpace);
  }

 private:
  glm::vec3 newPosition;
  bool isWorldSpace;
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
        newRotation(newRotation),
        isWorldSpace(isWorldSpace)
  {
  }

  EVENT_CLASS_TYPE(COMPONENT_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new RotationChangedEvent(entity, component, newRotation, isWorldSpace);
  }

 private:
  glm::quat newRotation;
  bool isWorldSpace;
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
        newScale(newPosition),
        isWorldSpace(isWorldSpace)
  {
  }

  EVENT_CLASS_TYPE(COMPONENT_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new ScaleChangedEvent(entity, component, newScale, isWorldSpace);
  }

 private:
  glm::vec3 newScale;
  bool isWorldSpace;
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

  EVENT_CLASS_TYPE(COMPONENT_CHANGED)
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
