#ifndef MITE_SCENE_TRANSFORM_COMPONENT
#define MITE_SCENE_TRANSFORM_COMPONENT

#include "basic_data/transform.h"
#include "scene_core/component_system.h"

namespace mite {
/**
 * @brief 变换组件，管理实体的位置、旋转和缩放
 *
 * 功能特性：
 * 1. 维护Transform，并常用的变换操作接口
 * 2. 仅支持局部空间变换，世界空间变换由SceneNode负责传递
 * 3. 提供相机View矩阵创建功能
 */
class TransformComponent
    : public SnapshotComponentTraits<Transform, Component::Family::Transform> {
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
   * @param order 旋转顺序
   */
  explicit TransformComponent(const glm::vec3 position,
                              const glm::vec3 rotation = glm::vec3(0.0f),
                              const glm::vec3 scale = glm::vec3(1.0f),
                              const Transform::EulerOrder order = Transform::EulerOrder::XYZ);

  /**
   * @brief 使用变换矩阵的构造函数
   * @param matrix 变换矩阵
   * @param order 旋转顺序
   */
  explicit TransformComponent(const glm::mat4 &matrix,
                              const Transform::EulerOrder order = Transform::EulerOrder::XYZ);

  ~TransformComponent() override = default;
  std::vector<std::type_index> GetDependencies() const override;

  // ==================== 位置操作 ====================
  const glm::vec3 &GetLocalPosition() const;
  void SetLocalPosition(const glm::vec3 &position);

  // ==================== 旋转操作（欧拉角deg、四元数） ====================
  Transform::EulerOrder GetRotationOrder() const;
  glm::vec3 GetLocalRotationEuler() const;
  glm::quat GetLocalRotationQuat() const;
  void SetLocalRotation(const glm::vec3 &rotation);
  void SetLocalRotation(float x, float y, float z);
  void SetLocalRotationQuat(const glm::quat &rotation);
  void LookAt(const glm::vec3 &target, const glm::vec3 &up = Transform::s_WorldUp);

  // ==================== 缩放操作 ====================
  const glm::vec3 &GetLocalScale() const;
  void SetLocalScale(const glm::vec3 &scale);
  void SetLocalScale(float scale);

  // ==================== 矩阵操作 ====================
  glm::mat4 GetLocalMatrix() const;
  void SetLocalMatrix(const glm::mat4 &matrix);
  /**
   * @brief 创建视图矩阵（相机专用）
   * @note 相机变换矩阵的逆就是View矩阵
   */
  glm::mat4 CreateViewMatrix() const;
  const Transform& GetTransform() const;

  // ===================== 方向向量（相机专用，世界空间） =========================
  glm::vec3 GetForward() const;
  glm::vec3 GetUp() const;
  glm::vec3 GetRight() const;

  // 防翻滚的方向获取，Up直接修正为WorldUp，保持Forward不变，基于这两个向量修正Right
  glm::vec3 GetConstrainedUp(const glm::vec3 &worldUp = Transform::s_WorldUp) const;
  glm::vec3 GetConstrainedRight(const glm::vec3 &worldUp = Transform::s_WorldUp) const;
  glm::vec3 GetConstrainedForward(const glm::vec3 &worldUp = Transform::s_WorldUp) const;

  // ==================== 序列化接口 ====================
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

 protected:
  // ==================== 快照接口 ====================
  Transform GetSnapshotData() const override;
  void SetSnapshotData(const Transform &data) override;

 private:
  Transform m_Transform;  // 基础变换对象
};

// ==================== 组件系统 ====================
class TransformComponentSystem : public SnapshotComponentSystem<TransformComponent> {
  DECLARE_COMPONENT_SYSTEM(TransformComponentSystem)
};
// ==================== 事件定义 ====================
/**
 * @class TransformUpdatedEvent
 * @brief Transform组件更新事件
 */
class TransformUpdatedEvent : public ComponentEvent<TransformComponent> {
 public:
  TransformUpdatedEvent(Entity entity, TransformComponent &component)
      : ComponentEvent<TransformComponent>(entity, component)
  {
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new TransformUpdatedEvent(entity, component);
  }
};
};  // namespace mite

#endif
