#ifndef MITE_SCENE_CAMERA_COMPONENT
#define MITE_SCENE_CAMERA_COMPONENT

#include "basic_data/camera.h"
#include "scene_core/component_system.h"

namespace mite {
/**
 * @brief 相机用途标签
 *
 * 后续可以添加：
 *
 * Player1View,  // 分屏玩家1
 * Player2View,  // 分屏玩家2
 * ShadowMap,    // 阴影贴图
 * UI,           // UI相机
 * Debug         // 调试视图
 */
enum class CameraUsage {
  FreeView,  // 闲置相机
  MainView,  // 主视图
};

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
  CameraComponent(std::shared_ptr<Camera> camera);

  // TODO: Camera如何处理脏标记?
  void ProcessDirty(float deltaTime, SceneRegistry &reg) override {}

  // 基础参数控制
  void SetPerspective(float fov, float near, float far);
  void SetOrthographic(float size, float near, float far);

  // 主摄像机标记
  CameraUsage GetUsage() const
  {
    return m_Usage;
  }
  void SetUsage(CameraUsage usage)
  {
    m_Usage = usage;
    MarkDirty();
  }

  std::shared_ptr<Camera> GetCamera() {
    return m_Camera;
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
  std::shared_ptr<Camera> m_Camera;
  CameraUsage m_Usage = CameraUsage::FreeView;
};

// 摄像机组件系统
class CameraComponentSystem : public DirtyComponentSystem<CameraComponent> {
  DECLARE_COMPONENT_SYSTEM(CameraComponentSystem)

  // 获取指定用途的相机实体
  std::optional<Entity> GetMainCameraEntity() const;

  // 设置主相机实体（确保唯一性）
  void SetMainCameraEntity(Entity main_camera);

 protected:
  void ProcessDirtyComponents(float deltaTime, SceneRegistry &registry) override;
};
};  // namespace mite

#endif
