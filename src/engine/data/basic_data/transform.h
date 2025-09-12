#ifndef MITE_DATA_TRANSFORM
#define MITE_DATA_TRANSFORM

#include "headers/headers.h"

namespace mite {
/**
 * @brief 纯数学变换类，管理位置、旋转、缩放和矩阵计算
 *
 * 职责：
 * 1. 管理局部变换属性（位置、旋转、缩放）
 * 2. 计算局部和世界变换矩阵
 * 3. 提供常用的变换操作接口
 * 4. 支持欧拉角（度）和四元数旋转表示
 *
 * 设计特点：
 * - 使用右手坐标系，glm的矩阵计算（包括lookat，quat）均遵循右手系
 * - 相机在局部空间Up为+Y方向，Forward为-Z方向（GetForward等方法相关）
 * - 旋转内部使用四元数存储，外部接口使用欧拉角（度）
 * - 提供高效的矩阵缓存机制（使用脏标记更新）
 * - 不依赖ECS，纯数学工具类，作为最底层设计
 */
class Transform {
 public:
  /**
   * @brief 欧拉角旋转顺序枚举
   */
  enum class EulerOrder {
    XYZ,  // 默认顺序
    XZY,
    YXZ,
    YZX,
    ZXY,
    ZYX
  };

  /**
   * @brief 默认构造函数（单位变换）
   * @param order 欧拉角顺序
   */
  Transform(EulerOrder order = EulerOrder::XYZ);

  /**
   * @brief 带初始值的构造函数
   * @param position 位置坐标
   * @param rotation 欧拉角旋转（度）
   * @param scale 缩放
   * @param order 欧拉角顺序
   */
  Transform(const glm::vec3 &position,
            const glm::vec3 &rotationEuler = glm::vec3(0.0f),
            const glm::vec3 &scale = glm::vec3(1.0f),
            EulerOrder order = EulerOrder::XYZ);

  /**
   * @brief 使用变换矩阵的构造函数
   * @param matrix 变换矩阵
   * @param order 欧拉角顺序
   */
  explicit Transform(const glm::mat4 &matrix, EulerOrder order = EulerOrder::XYZ);

  // ==================== 位置相关方法 ====================
  const glm::vec3 &GetPosition() const;
  void SetPosition(const glm::vec3 &position);
  void Translate(const glm::vec3 &direction);

  // ==================== 旋转相关方法 ====================
  // 欧拉角操作（度）
  glm::vec3 GetRotationEuler();
  void SetRotationEuler(const glm::vec3 &eulerDegrees);
  void SetRotationEuler(float x, float y, float z);

  // 四元数操作
  glm::quat GetRotationQuat() const;
  void SetRotationQuat(const glm::quat &rotation);

  // 旋转顺序控制
  EulerOrder GetRotationOrder() const;
  void SetRotationOrder(EulerOrder order);

  // 轴角旋转
  // （注意，这个轴是世界轴，无需遵循EulerOrder，反映到欧拉角也仅有最后）
  void Rotate(const glm::vec3 &axis, float angleDegrees);
  void RotateX(float angleDegrees);
  void RotateY(float angleDegrees);
  void RotateZ(float angleDegrees);

  // 绕点旋转
  void RotateAround(const glm::vec3 &point, const glm::vec3 &axis, float angleDegrees);


    

  // LookAt功能（由调用方指定up方向）
  void LookAt(const glm::vec3 &target, const glm::vec3 &up = glm::vec3(0.0f, 0.0f, 1.0f));

  // ==================== 缩放相关方法 ====================
  const glm::vec3 &GetScale() const;
  void SetScale(const glm::vec3 &scale);
  void SetScale(float uniformScale);

  // ==================== 矩阵相关方法 ====================
  glm::mat4 GetLocalMatrix() const;
  void SetLocalMatrix(const glm::mat4 &matrix);

  // 视图矩阵专用接口
  glm::mat4 GetViewMatrix() const;
  bool IsViewMatrixValid() const;

  // ==================== 方向向量方法 ====================
  glm::vec3 GetForward() const;  // 相机看向的方向（-Z方向）
  glm::vec3 GetUp() const;       // 相机朝上的方向（+Y方向）
  glm::vec3 GetRight() const;    // 相机朝右的方向（+X方向）

  // 基于指定上方向的方向向量
  glm::vec3 GetForward(const glm::vec3 &up) const;
  glm::vec3 GetRight(const glm::vec3 &up) const;

  // ==================== 辅助方法 ====================

  /**
   * @brief 重置变换为单位变换
   */
  void Reset();

  /**
   * @brief 检查变换是否为单位变换
   * @return 是否为单位变换
   */
  bool IsIdentity() const;

 private:
  // ==================== 私有方法 ====================
  /**
   * @brief 更新局部变换矩阵
   */
  void UpdateLocalMatrix() const;

  /**
   * @brief 从欧拉角更新四元数旋转
   */
  void UpdateRotationFromEuler();

  /**
   * @brief 从四元数更新欧拉角
   */
  void UpdateEulerFromRotation();

  // 旋转顺序相关的欧拉角转换
  glm::vec3 QuatToEulerByOrder(const glm::quat &quat, EulerOrder order) const;
  glm::quat EulerToQuatByOrder(const glm::vec3 &eulerDegrees, EulerOrder order) const;

 private:
  // 变换属性
  glm::vec3 m_Position = glm::vec3(0.0f);
  glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);  // 四元数，仅用于变换计算
  glm::vec3 m_RotationEuler = glm::vec3(0.0f);  // 欧拉角（度），仅用于对外接口
  glm::vec3 m_Scale = glm::vec3(1.0f);
  EulerOrder m_RotationOrder = EulerOrder::XYZ;

  // 矩阵缓存
  mutable glm::mat4 m_LocalMatrix = glm::mat4(1.0f);

  // 脏标记：在Set()时Mark，在Get()时执行Update()并消除Mark
  mutable bool m_MatrixDirty = true;    // 矩阵脏标记
  mutable bool m_RotationDirty = true;  // 旋转脏标记
};
}  // namespace mite

#endif  // MITE_DATA_TRANSFORM