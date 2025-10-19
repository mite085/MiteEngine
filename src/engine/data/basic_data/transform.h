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
 * 5. 提供相机ViewMatrix计算，与LookAt、RotateCamera等配套方法
 *
 * 设计特点：
 * - 使用右手坐标系，glm的矩阵计算（包括lookat，quat）均遵循右手系
 * - 相机在局部空间Up为+Y方向，Forward为-Z方向（GetForward等方法相关）
 * - 旋转内部使用四元数存储，外部接口使用欧拉角（度）
 * - 支持yaw - pitch - roll的内旋->外旋方式旋转
 * - 提供高效的矩阵缓存机制（使用脏标记更新）
 * - 不依赖ECS，纯数学工具类，作为最底层设计
 *
 * Blender导出OBJ模型时，选择Up为+Y，Forward为-Z即可。GLTF则默认设置
 * 这样可以保证建模结果的朝上和朝前方向，与导入结果的朝上和朝前方向一致
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
  // 默认的世界向上方向为+Y轴：glm::vec3(0.0f, 1.0f, 0.0f)
  // 默认的世界向前方向为-Z轴：glm::vec3(0.0f, 0.0f,-1.0f)
  static const glm::vec3 s_WorldUp;
  static const glm::vec3 s_WorldForward;
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
  /**
   * @brief 基于相机语义的平移接口
   * @param horizontal 水平方向增量（向右为正）
   * @param vertical 竖直方向增量（向上为正）
   * @param worldUp 世界的向上方向
   */
  void PanCamera(float horizontal, float vertical, const glm::vec3 &worldUp = s_WorldUp);

  // ==================== 旋转相关方法 ====================
  // 旋转顺序控制
  EulerOrder GetRotationOrder() const;
  void SetRotationOrder(EulerOrder order);

  // 欧拉角操作（度）
  glm::vec3 GetRotationEuler() const;
  void SetRotationEuler(const glm::vec3 &eulerDegrees);
  void SetRotationEuler(float x, float y, float z);

  // 四元数操作
  glm::quat GetRotationQuat() const;
  void SetRotationQuat(const glm::quat &rotation);

  // 世界轴旋转
  void RotateWorld(const glm::vec3 &axis, float angleDegrees);
  void RotateWorldX(float angleDegrees);
  void RotateWorldY(float angleDegrees);
  void RotateWorldZ(float angleDegrees);

  // 局部轴旋转
  void RotateLocal(const glm::vec3 &localAxis, float angleDegrees);
  void RotateLocalX(float angleDegrees);
  void RotateLocalY(float angleDegrees);
  void RotateLocalZ(float angleDegrees);

  // 绕点旋转
  void RotateAround(const glm::vec3 &point, const glm::vec3 &axis, float angleDegrees);

  /**
   * @brief 基于相机语义的旋转接口（和组合接口）
   * @param yaw 偏航角（相机左右环视）
   * @param pitch 俯仰角（相机上下俯仰）
   * @param roll 滚转角（相机画面旋转）
   * @param worldUp 世界的向上方向
   *
   * 旋转顺序为：yaw - pitch - roll
   *
   * 假设 WorldUp为 +Z方向，相机执行旋转分以下三步：
   * 1. 首先应当绕着世界 Z轴“左右”旋转yaw，确定 Right方向（世界空间）
   * 2. 随后应当绕着 Right方向所在的轴“上下”旋转pitch，确定 Forward方向（世界空间）
   * 3. 最后应当绕着 Forward方向“顺/逆时针”旋转roll，完成整个旋转过程
   *
   * 注意：
   * 最终旋转结果与EulerOrder无关，因为对于给定的worldUp，经过yaw - pitch - roll三次
   * 计算相机的Up、Right、Forward之后，结果都会是固定的。而这一组yaw - pitch - roll
   * 也不是相机的欧拉角。相机的欧拉角应当是根据当前四元数与EulerOrder解算出的“结果”
   *
   * 若 WorldUp为 +Z方向
   * 1. Yaw表示绕着Z轴逆时针旋转（从Z轴正方向向下看，RotateYaw(30.0f) → 相机向左转30度）
   * 2. Pitch表示绕着Right轴逆时针旋转（从Right轴正方向向下看，RotatePitch(20.0f) →
   * 相机向上抬头20度）
   * 3. Roll表示将画面顺时针旋转（从Forward轴负方向向上看，RotateRoll(15.0f) → 相机向右倾斜15度）
   */
  void RotateCamera(float yaw,
                    float pitch,
                    float roll = 0.0f,
                    const glm::vec3 &worldUp = s_WorldUp);
  void RotateYaw(float degrees, const glm::vec3 &worldUp = s_WorldUp);
  void RotatePitch(float degrees, const glm::vec3 &worldUp = s_WorldUp);
  void RotateRoll(float degrees, const glm::vec3 &worldUp = s_WorldUp);

  // LookAt功能（由调用方指定up方向）
  void LookAt(const glm::vec3 &target, const glm::vec3 &up = s_WorldUp);

  // ==================== 缩放相关方法 ====================
  const glm::vec3 &GetScale() const;
  void SetScale(const glm::vec3 &scale);
  void SetScale(float uniformScale);

  // ==================== 矩阵相关方法 ====================
  glm::mat4 GetLocalMatrix() const;
  void SetLocalMatrix(const glm::mat4 &matrix);

  /**
   * @brief 视图矩阵专用接口
   *
   * - 对于标准的右手系视图矩阵
   *   [ right.x     right.y     right.z     -dot(right, eye)  ]
   *   [ up.x        up.y        up.z        -dot(up, eye)     ]
   *   [ -forward.x -forward.y  -forward.z    dot(forward, eye)]
   *   [ 0           0           0            1                ]
   *
   * GLM的mat4使用了列主序，如:
   * 对于：m_ViewMatrix = GetViewMatrix();
   * 此时：m_ViewMatrix[0]              表示第一列[right.x,  up.x,  -forward.x,  0]
   *       glm::column(m_ViewMatrix,0)  表示第一列（同上）
   *       glm::row(m_ViewMatrix,0)     表示第一行[ right.x  right.y  right.z  -dot(right, eye)]
   */
  glm::mat4 GetViewMatrix() const;
  bool IsViewMatrixValid() const;

  // ==================== 方向向量方法（相机专用） ====================
  glm::vec3 GetForward() const;  // 在世界空间，相机看向的方向（-Z方向）
  glm::vec3 GetUp() const;       // 在世界空间，相机朝上的方向（+Y方向）
  glm::vec3 GetRight() const;    // 在世界空间，相机朝右的方向（+X方向）

  // 获取防翻滚（固定Up方向）后的方向向量，与RotateWithUpConstraint配合使用
  glm::vec3 GetConstrainedUp(const glm::vec3 &worldUp = s_WorldUp) const;
  glm::vec3 GetConstrainedRight(const glm::vec3 &worldUp = s_WorldUp) const;
  glm::vec3 GetConstrainedForward(const glm::vec3 &worldUp = s_WorldUp) const;

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

  /**
   * @brief 清理脏标记
   */
  void CleanDirty();

 private:
  // ==================== 私有方法 ====================
  /**
   * @brief 更新局部变换矩阵
   */
  void UpdateLocalMatrix() const;

  /**
   * @brief 从欧拉角更新四元数旋转
   */
  void UpdateRotationFromEuler() const;

  /**
   * @brief 从四元数更新欧拉角
   */
  void UpdateEulerFromRotation() const;

  // 旋转顺序相关的欧拉角转换
  glm::vec3 QuatToEulerByOrder(const glm::quat &quat, EulerOrder order) const;
  glm::quat EulerToQuatByOrder(const glm::vec3 &eulerDegrees, EulerOrder order) const;

 private:
  // 脏标记
  // 在Set()时Mark，在Get()时执行Update()并消除Mark
  //
  // 注意：
  // 使用脏标记维护的属性（Rotation和Matrix）也应当是mutable的
  mutable bool m_MatrixDirty = true;    // 矩阵脏标记
  mutable bool m_RotationDirty = true;  // 旋转脏标记

  // 变换属性
  mutable glm::vec3 m_Position = glm::vec3(0.0f);
  mutable glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);  // 四元数，仅用于变换计算
  mutable glm::vec3 m_RotationEuler = glm::vec3(0.0f);  // 欧拉角（度），仅用于对外接口
  mutable glm::vec3 m_Scale = glm::vec3(1.0f);
  mutable EulerOrder m_RotationOrder = EulerOrder::XYZ;

  // 矩阵缓存
  mutable glm::mat4 m_LocalMatrix = glm::mat4(1.0f);
};
}  // namespace mite

#endif  // MITE_DATA_TRANSFORM