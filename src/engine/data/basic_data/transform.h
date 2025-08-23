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
 * - 使用右手坐标系，Y轴向上
 * - 旋转内部使用四元数存储，外部接口使用欧拉角（度）
 * - 提供高效的矩阵缓存机制
 * - 不依赖ECS，纯数学工具类
 */
class Transform {
 public:
  /**
   * @brief 默认构造函数
   */
  Transform();

  /**
   * @brief 带初始值的构造函数
   * @param position 位置坐标
   * @param rotation 欧拉角旋转（度）
   * @param scale 缩放
   */
  Transform(const glm::vec3 &position,
            const glm::vec3 &rotation = glm::vec3(0.0f),
            const glm::vec3 &scale = glm::vec3(1.0f));

  /**
   * @brief 使用变换矩阵的构造函数
   * @param matrix 变换矩阵
   */
  explicit Transform(const glm::mat4 &matrix);

  // ==================== 位置相关方法 ====================

  /**
   * @brief 获取位置
   * @return 位置坐标
   */
  const glm::vec3 &GetPosition() const;

  /**
   * @brief 设置位置
   * @param position 位置坐标
   */
  void SetPosition(const glm::vec3 &position);

  /**
   * @brief 沿指定方向移动
   * @param direction 移动方向
   */
  void Translate(const glm::vec3 &direction);

  // ==================== 旋转相关方法 ====================

  /**
   * @brief 获取欧拉角旋转（度）
   * @return 欧拉角旋转（度）
   */
  glm::vec3 GetRotation() const;

  /**
   * @brief 设置欧拉角旋转（度）
   * @param rotation 欧拉角旋转（度）
   */
  void SetRotation(const glm::vec3 &rotation);

  /**
   * @brief 设置欧拉角旋转（度）
   * @param x X轴旋转角度（度）
   * @param y Y轴旋转角度（度）
   * @param z Z轴旋转角度（度）
   */
  void SetRotation(float x, float y, float z);

  /**
   * @brief 绕指定轴旋转实体
   * @param axis 旋转轴（世界坐标）
   * @param angle 旋转角度（度）
   *
   * 注意：
   * - 会自动标准化旋转轴
   * - 旋转顺序为世界空间
   */
  void Rotate(const glm::vec3 &axis, float angle);

  /**
   * @brief 绕X轴旋转
   * @param angle 旋转角度（度）
   */
  void RotateX(float angle);

  /**
   * @brief 绕Y轴旋转
   * @param angle 旋转角度（度）
   */
  void RotateY(float angle);

  /**
   * @brief 绕Z轴旋转
   * @param angle 旋转角度（度）
   */
  void RotateZ(float angle);

  /**
   * @brief 绕空间中某一点旋转实体
   * @param point 旋转中心点（世界坐标）
   * @param axis 旋转轴（世界坐标）
   * @param angle 旋转角度（度）
   *
   * 实现原理：
   * 1. 计算从旋转中心到实体的向量
   * 2. 绕轴旋转该向量
   * 3. 更新实体位置
   * 4. 同时应用旋转到实体朝向
   */
  void RotateAround(const glm::vec3 &point, const glm::vec3 &axis, float angle);

  /**
   * @brief 旋转至看向某个目标
   * @param position 当前位置
   * @param target 目标位置
   * @param up 上方向向量
   */
  void LookAt(const glm::vec3 &position,
              const glm::vec3 &target,
              const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f));

  // ==================== 缩放相关方法 ====================

  /**
   * @brief 获取缩放
   * @return 缩放向量
   */
  const glm::vec3 &GetScale() const;

  /**
   * @brief 设置缩放
   * @param scale 缩放向量
   */
  void SetScale(const glm::vec3 &scale);

  /**
   * @brief 设置均匀缩放
   * @param scale 缩放值
   */
  void SetScale(float scale);

  // ==================== 矩阵相关方法 ====================

  /**
   * @brief 获取局部变换矩阵
   * @return 局部变换矩阵
   */
  glm::mat4 GetLocalMatrix() const;

  /**
   * @brief 设置局部变换矩阵
   * @param matrix 局部变换矩阵
   */
  void SetLocalMatrix(const glm::mat4 &matrix);

  /**
   * @brief 获取世界变换矩阵（对于独立Transform，等同于局部矩阵）
   * @return 世界变换矩阵
   */
  glm::mat4 GetWorldMatrix() const;

  // ==================== 方向向量方法 ====================

  /**
   * @brief 获取前向向量
   * @return 前向向量（Z轴负方向）
   */
  glm::vec3 GetForward() const;

  /**
   * @brief 获取上向量
   * @return 上向量（Y轴正方向）
   */
  glm::vec3 GetUp() const;

  /**
   * @brief 获取右向量
   * @return 右向量（X轴正方向）
   */
  glm::vec3 GetRight() const;

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

 private:
  // 变换属性
  glm::vec3 m_Position = glm::vec3(0.0f);
  glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 m_RotationEuler = glm::vec3(0.0f);  // 欧拉角（度）
  glm::vec3 m_Scale = glm::vec3(1.0f);

  // 矩阵缓存
  mutable glm::mat4 m_LocalMatrix = glm::mat4(1.0f);
  mutable bool m_MatrixDirty = true;  // 矩阵脏标记
};

}  // namespace mite

#endif  // MITE_DATA_TRANSFORM