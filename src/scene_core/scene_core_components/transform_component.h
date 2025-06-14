#ifndef MITE_SCENE_TRANSFORM_COMPONENT
#define MITE_SCENE_TRANSFORM_COMPONENT

#include "scene_core/component.h"

namespace mite {
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
  // 默认构造函数
  TransformComponent(std::weak_ptr<Entity> owner);

  // 带初始值的构造函数
  explicit TransformComponent(std::weak_ptr<Entity> owner,
                              const glm::vec3 &position,
                              const glm::quat &rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                              const glm::vec3 &scale = glm::vec3(1.0f));

  // 从变换矩阵构造
  explicit TransformComponent(std::weak_ptr<Entity> owner, const glm::mat4 &matrix);

  ~TransformComponent() override = default;

  // 位置操作 ==============================================
  const glm::vec3 &GetLocalPosition() const
  {
    return m_Position;
  }
  void SetLocalPosition(const glm::vec3 &position);

  // 获取世界空间位置(需要父变换计算)
  glm::vec3 GetWorldPosition() const;
  void SetWorldPosition(const glm::vec3 &position);

  // 相对移动
  void Translate(const glm::vec3 &translation);
  void Translate(float x, float y, float z);

  // 旋转操作 ==============================================
  const glm::quat &GetLocalRotation() const
  {
    return m_Rotation;
  }
  void SetLocalRotation(const glm::quat &rotation);

  // 欧拉角接口(弧度制)
  glm::vec3 GetLocalEulerAngles() const;
  void SetLocalEulerAngles(const glm::vec3 &eulerAngles);
  void SetLocalEulerAngles(float x, float y, float z);

  // 世界空间旋转
  glm::quat GetWorldRotation() const;
  void SetWorldRotation(const glm::quat &rotation);

  // 旋转方法
  void Rotate(const glm::quat &rotation);
  void Rotate(const glm::vec3 &axis, float angle);
  void RotateAround(const glm::vec3 &point, const glm::vec3 &axis, float angle);
  void LookAt(const glm::vec3 &target, const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f));

  // 缩放操作 ==============================================
  const glm::vec3 &GetLocalScale() const
  {
    return m_Scale;
  }
  void SetLocalScale(const glm::vec3 &scale);
  void SetLocalScale(float scale);

  // 世界空间缩放(近似计算)
  glm::vec3 GetWorldScale() const;

  // 变换矩阵操作 ==========================================
  glm::mat4 GetLocalMatrix() const;
  glm::mat4 GetWorldMatrix() const;

  void SetLocalMatrix(const glm::mat4 &matrix);
  void SetWorldMatrix(const glm::mat4 &matrix);

  // 方向向量 ==============================================
  glm::vec3 Forward() const;  // 正Z轴方向
  glm::vec3 Up() const;       // 正Y轴方向
  glm::vec3 Right() const;    // 正X轴方向

  // 组件接口实现 ==========================================
  std::vector<std::type_index> GetDependencies() const override;
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

  // 变换更新标志
  void UpdateTransform();

 private:
  // 计算世界变换矩阵(递归)
  void CalculateWorldMatrix() const;

  // 从矩阵分解变换
  void DecomposeMatrix(const glm::mat4 &matrix);

 private:
  // 局部空间变换属性
  glm::vec3 m_Position = glm::vec3(0.0f);
  glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 m_Scale = glm::vec3(1.0f);

  // 矩阵缓存
  mutable glm::mat4 m_LocalMatrix = glm::mat4(1.0f);
  mutable glm::mat4 m_WorldMatrix = glm::mat4(1.0f);

  // 脏标记
  mutable bool m_LocalMatrixDirty = true;
  mutable bool m_WorldMatrixDirty = true;
};
};

#endif
