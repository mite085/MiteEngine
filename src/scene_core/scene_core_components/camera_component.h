#ifndef MITE_SCENE_CAMERA_COMPONENT
#define MITE_SCENE_CAMERA_COMPONENT

#include "scene_core/camera.h"
#include "scene_core/component_system.h"

namespace mite {
/**
 * @brief 摄像机组件，将Camera与ECS集成
 *
 * 职责：
 * 1. 关联TransformComponent实现世界空间摄像机
 * 2. 管理主摄像机标记
 * 3. 处理视口适配事件
 *
 * 协作关系：
 * - 必须与TransformComponent共存
 * - SceneView通过此组件获取渲染用摄像机
 */
class CameraComponent : public ComponentTraits<CameraComponent, Component::Family::Core> {
 public:
  CameraComponent();

  // 基础参数控制
  void SetPerspective(float fov, float near, float far);
  void SetOrthographic(float size, float near, float far);

  // 主摄像机标记
  bool IsMain() const
  {
    return m_IsMain;
  }
  void SetMain(bool isMain)
  {
    m_IsMain = isMain;
  }

  // 矩阵获取（需结合Transform）
  glm::mat4 GetViewMatrix(SceneRegistry &reg) const;
  glm::mat4 GetProjectionMatrix() const;

  // 视口适配
  void SetViewportSize(uint32_t width, uint32_t height);

  // 序列化
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

  std::vector<std::type_index> GetDependencies() const override;

 private:
  Camera m_Camera;
  bool m_IsMain = false;  // 是否为主摄像机
};

// 摄像机组件系统
class CameraSystem : public DirtyComponentSystem<CameraComponent> {
  DECLARE_COMPONENT_SYSTEM(CameraSystem)
 protected:
  void ProcessDirtyComponents(float deltaTime, SceneRegistry &registry) override;
};

};  // namespace mite

#endif
