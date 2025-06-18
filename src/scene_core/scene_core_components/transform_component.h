#ifndef MITE_SCENE_TRANSFORM_COMPONENT
#define MITE_SCENE_TRANSFORM_COMPONENT

#include "scene_core/component_system.h"

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
  /**
   * @brief 默认构造函数
   * @param owner 持有该旋转组件的实体
   */
  TransformComponent(Entity owner);

  /**
   * @brief 带初始值的构造函数
   * @param owner 持有该旋转组件的实体
   * @param position 位置坐标
   * @param rotation 旋转
   * @param scale 缩放
   */
  explicit TransformComponent(Entity owner,
                              const glm::vec3 &position,
                              const glm::quat &rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                              const glm::vec3 &scale = glm::vec3(1.0f));

  /**
   * @brief 使用变换矩阵的构造函数
   * @param owner 持有该旋转组件的实体
   * @param matrix 变换矩阵
   */
  explicit TransformComponent(Entity owner, const glm::mat4 &matrix);

  ~TransformComponent() override = default;

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
  glm::vec3 GetWorldPosition() const;
  /**
   * @brief 设定世界空间位置
   * @param position 世界空间三维位置坐标
   */
  void SetWorldPosition(const glm::vec3 &position);

  /**
   * @brief 相对移动
   * @param translation 局部空间移动量
   */
  void Translate(const glm::vec3 &translation);
  /**
   * @brief 相对移动
   * @param x 局部空间移动x分量
   * @param y 局部空间移动y分量
   * @param z 局部空间移动z分量
   */
  void Translate(float x, float y, float z);

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
   * @brief 获取局部空间旋转(欧拉角)
   * @return 局部空间旋转欧拉角(弧度制)
   */
  glm::vec3 GetLocalEulerAngles() const;
  /**
   * @brief 设定局部空间旋转(欧拉角)
   * @param eulerAngles 局部空间旋转欧拉角
   */
  void SetLocalEulerAngles(const glm::vec3 &eulerAngles);
  /**
   * @brief 设定局部空间旋转(欧拉角)
   * @param x 局部空间旋转欧拉角x分量
   * @param y 局部空间旋转欧拉角y分量
   * @param z 局部空间旋转欧拉角z分量
   */
  void SetLocalEulerAngles(float x, float y, float z);

  /**
   * @brief 获取世界空间旋转(四元数)
   * @return 世界空间旋转四元数
   */
  glm::quat GetWorldRotation() const;
  /**
   * @brief 设定世界空间旋转(四元数)
   * @param rotation 世界空间旋转四元数
   */
  void SetWorldRotation(const glm::quat &rotation);

  // 旋转方法
  /**
   * @brief 旋转实体
   * @param rotation 局部空间旋转四元数
   */
  void Rotate(const glm::quat &rotation);
  /**
   * @brief 绕指定轴旋转实体
   * @param axis 旋转轴（单位向量）
   * @param angle 旋转角度（弧度）
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
  void RotateAround(const glm::vec3 &point, const glm::vec3 &axis, float angle);
  /**
   * @brief 旋转至看向某个目标
   * @param target 目标的世界空间三维位置坐标
   * @param up 观察的Up方向
   */
  void LookAt(const glm::vec3 &target, const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f));

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
  glm::vec3 GetWorldScale() const;

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
  glm::mat4 GetWorldMatrix() const;
  /**
   * @brief 设定局部空间变换矩阵
   * @param matrix 局部空间变换矩阵
   */
  void SetLocalMatrix(const glm::mat4 &matrix);
  /**
   * @brief 设定世界空间变换矩阵
   * @param matrix 世界空间变换矩阵
   */
  void SetWorldMatrix(const glm::mat4 &matrix);

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

class TransformSystem : public ComponentSystem {
  DECLARE_COMPONENT_SYSTEM(TransformSystem)
 public:
  Component::Family GetExecutionOrder() const override;
  void Initialize(SceneRegistry &registry) override;
  void Update(SceneRegistry &registry, float deltaTime) override;
  void Shutdown(SceneRegistry &registry) override;
  std::vector<std::type_index> GetComponentTypes() const override;
  std::vector<std::type_index> GetSystemDependencies() const override;
  void OnComponentAdded(Entity entity, Component &component) override;
  void OnComponentRemoved(Entity entity, Component &component) override;
};

};

#endif
