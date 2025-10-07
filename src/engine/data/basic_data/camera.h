#ifndef MITE_DATA_CAMERA
#define MITE_DATA_CAMERA

#include "basic_shader/shader_ubo.h"
#include "headers/headers.h"

namespace mite {
/**
 * @brief 相机类型枚举
 */
enum class CameraProjectionType {
  PERSPECTIVE,   // 透视相机
  ORTHOGRAPHIC,  // 正交相机
};

/**
 * @brief 相机清除标志
 */
enum class CameraClearFlags {
  SKYBOX,       // 清除为天空盒
  SOLID_COLOR,  // 清除为纯色
  DEPTH_ONLY,   // 只清除深度
  DONT_CLEAR    // 不清除
};
/**
 * @brief 独立摄像机类，封装投影矩阵计算
 *
 * 视图矩阵完全交给Transform负责。
 * （架构层面：ECS架构下，相机实体同时绑定CameraComponent和TransformComponent，View和Transform互相冲突）
 * （数学层面：View负责将坐标从World空间转换到相机Local空间，与WorldMatrix作用刚好相反，即相机WorldMatrix的逆，为视图矩阵）
 *
 * 定位：纯数学计算工具，与ProjectionMatrix状态管理
 *
 * 职责：
 * 1. 管理摄像机参数（FOV、Clipping Planes等）
 * 2. 矩阵计算（View/Projection）
 * 3. 数学变换（旋转、平移、缩放）
 *
 * 注意：
 * - 使用右手坐标系
 * - 相机在局部空间Up为 +Y方向，Forward为 -Z方向，Right为 +X方向
 * - glm::perspective和 glm::ortho生成右手坐标系的透视/正交投影矩阵
 * - GLM的mat4使用了列主序
 *   如：
 *   m_ProjectionMatrix[2]或者glm::column(m_ProjectionMatrix,2)表示第三列[0,  0,  -(f+n)/(f-n), -1]
 *
 * - 标准的右手系透视投影矩阵
 *   [ n/r   0     0             0        ]
 *   [ 0     n/t   0             0        ]
 *   [ 0     0    -(f+n)/(f-n)  -2fn/(f-n)]
 *   [ 0     0    -1             0        ]
 *   其中：近平面：n = near，远平面：f = far
 */
class Camera {
 public:
  Camera();

  // ==================== 投影参数设置 ====================
  void SetPerspective(float fov, float near, float far);    // 设定为透视相机
  void SetOrthographic(float size, float near, float far);  // 设定为正交相机
  void SetProjectionType(CameraProjectionType type);        // 设定投影类型：透视/正交
  void SetAspectRatio(float aspect);                        // 设置宽高比

  // ==================== 矩阵获取 ====================
  const glm::mat4 &GetProjectionMatrix() const;  // 获取投影矩阵

  // ==================== 参数访问 ====================
  CameraProjectionType GetProjectionType() const;  // 获取投影类型
  float GetNear() const;                           // 近平面
  float GetFar() const;                            // 远平面
  float GetFOV() const;                            // 视场角（deg，透视相机专属）
  float GetAspectRatio() const;                    // 宽高比
  float GetOrthoSize() const;                      // 正交尺寸

  // ==================== 投影控制方法 ====================
  void Zoom(float amount);

  /**
   * @brief 填充UBO Data
   */
  CameraUniformBuffer FillUBOData(const glm::mat4 &viewMatrix) const;
    
 private:
  // 辅助方法
  void UpdateProjection() const;  // 更新投影矩阵，清理脏标记

  // 投影类型，默认透视
  CameraProjectionType m_ProjectionType = CameraProjectionType::PERSPECTIVE;

  // 透视参数
  float m_FOV = 45.0f;  // 垂直FOV（度）
  float m_Aspect = 16.0f / 9.0f;

  // 正交参数
  float m_OrthoSize = 10.0f;

  // 公共参数
  float m_Near = 0.1f;
  float m_Far = 100.0f;

  // 投影矩阵
  mutable glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);

  // 脏标记：在Set()时Mark，在Get()时执行Update()并消除Mark
  mutable bool m_ProjectionDirty = true;
};
};  // namespace mite

#endif
