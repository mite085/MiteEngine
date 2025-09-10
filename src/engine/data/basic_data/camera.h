#ifndef MITE_DATA_CAMERA
#define MITE_DATA_CAMERA

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
 * - 默认朝向Z负方向，Up为Y轴正方向，欧拉角顺序为YXZ（更符合OpenGL标准）
 * - GLM使用了右手坐标系
 * - GLM的mat4使用了列主序
 *   如:
 *   m_ViewMatrix[0]或者glm::column(m_ViewMatrix,1)   表示第一列[right.x,  up.x,  -forward.x,  0]
 *   glm::row(vpMatrix,0)   表示[ right.x  right.y  right.z  -dot(right, eye)]
 *   如：
 *   m_ProjectionMatrix[2]或者glm::column(m_ProjectionMatrix,2)表示第三列[0,  0,  -(f+n)/(f-n),  -1]
 * 
 * 对于标准的右手系视图矩阵
 * [ right.x     right.y     right.z     -dot(right, eye)  ]
 * [ up.x        up.y        up.z        -dot(up, eye)     ]
 * [ -forward.x -forward.y  -forward.z    dot(forward, eye)]
 * [ 0           0           0            1                ]

 * 
 * 标准的右手系透视投影矩阵
 * [ n/r   0     0             0        ]
 * [ 0     n/t   0             0        ]
 * [ 0     0    -(f+n)/(f-n)  -2fn/(f-n)]
 * [ 0     0    -1             0        ]
 * 其中
 * 近平面：n = near
 * 远平面：f = far
 * 宽高比：a = aspect
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
  const glm::mat4 &GetProjectionMatrix() const;
  const glm::mat4 &GetViewMatrix() const;
  glm::mat4 GetViewProjectionMatrix() const;

  // 参数访问
  ProjectionType GetProjectionType() const;
  float GetNear() const;
  float GetFar() const;
  float GetFOV() const;
  float GetAspectRatio() const;
  glm::vec3 GetPosition() const;
  glm::vec3 GetRotationEuler() const;
  glm::vec3 GetRightVector() const;
  glm::vec3 GetUpVector() const;
  glm::vec3 GetForwardVector() const;
  float GetDistance() const;

  // 相机控制
  void Translate(const glm::vec3 position);  // 世界空间定位
  void Rotate(const glm::vec3 rotation);  // 欧拉角旋转（角度制，偏航/俯仰/滚转）
  void Rotate(float yaw, float pitch, float roll = 0.0f);  // 欧拉角旋转（角度制，偏航/俯仰/滚转）
  void Pan(float right, float up);                         // 屏幕空间平移
  void Zoom(float amount);                                 // 视野缩放
  void Move(const glm::vec3 direction);                    // 世界空间移动

 private:
  // 辅助方法
  void RecalculateViewFromRotation();
  void RecalculateProjection();

  // 投影类型，默认透视
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

  // 存储当前的位置与欧拉角(degree)
  glm::vec3 m_Position = glm::vec3(0.0f);
  glm::vec3 m_RotationEuler = glm::vec3(0.0f);
};
};  // namespace mite

#endif
