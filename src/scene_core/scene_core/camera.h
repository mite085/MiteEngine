#ifndef MITE_CORE_CAMERA
#define MITE_CORE_CAMERA

#include "headers/headers.h"

namespace mite {
/**
 * @brief 独立摄像机类，封装视图/投影矩阵计算
 *
 * 职责：
 * 1. 管理摄像机参数（FOV、Clipping Planes等）
 * 2. 计算视图/投影矩阵
 * 3. 支持多种投影模式（透视/正交）
 *
 * 注意：
 * - 不依赖ECS，纯数学工具类
 * - 与TransformComponent协同工作
 */
class Camera {
 public:
  enum class ProjectionType { Perspective, Orthographic };

  Camera();

  // 投影参数设置
  void SetPerspective(float fov, float aspect, float near, float far);
  void SetOrthographic(float size, float aspect, float near, float far);
  void SetProjectionType(ProjectionType type);
  void SetAspectRatio(float aspect);

  // 视图控制
  void LookAt(const glm::vec3 &position,
              const glm::vec3 &target,
              const glm::vec3 &up = glm::vec3(0, 1, 0));
  void SetViewMatrix(const glm::mat4 &view);

  // 矩阵获取
  const glm::mat4 &GetProjectionMatrix() const
  {
    return m_ProjectionMatrix;
  }
  const glm::mat4 &GetViewMatrix() const
  {
    return m_ViewMatrix;
  }
  glm::mat4 GetViewProjectionMatrix() const
  {
    return m_ProjectionMatrix * m_ViewMatrix;
  }

  // 参数访问
  float GetNear() const
  {
    return m_Near;
  }
  float GetFar() const
  {
    return m_Far;
  }
  float GetFOV() const
  {
    return m_FOV;
  }
  float GetAspectRatio() const
  {
    return m_Aspect;
  }

 private:
  void RecalculateProjection();

  ProjectionType m_ProjectionType = ProjectionType::Perspective;

  // 透视参数
  float m_FOV = 45.0f;  // 垂直FOV（度）
  float m_Aspect = 16.0f / 9.0f;

  // 正交参数
  float m_OrthoSize = 10.0f;

  // 公共参数
  float m_Near = 0.1f;
  float m_Far = 100.0f;

  glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
  glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
};
};  // namespace mite

#endif
