#ifndef MITE_SCENE_CAMERA_COMPONENT
#define MITE_SCENE_CAMERA_COMPONENT

#include "basic_instance/camera_instance.h"
#include "scene_core/component_system.h"

namespace mite {
/**
 * @brief 相机用途标签
 * 
 * 目前仅用于区分主相机和闲置相机，后续可以添加：
 * Player1View,  // 分屏玩家1
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
 * 定位：场景中的摄像机实体表示
 * 
 * 职责：
 * 1. 关联TransformComponent实现世界空间摄像机
 * 2. 管理主摄像机标记（可扩展摄像机用途标签）
 * 3. 处理视口适配事件
 * 4. 基于相机可见性掩码实现分层渲染
 * 
 * 协作关系：
 * - 必须与TransformComponent共存
 * - SceneView通过此组件获取渲染用摄像机
 */
class CameraComponent : public SnapshotComponentTraits<std::shared_ptr<Camera>, Component::Family::Render> {
 public:
  CameraComponent(CameraProjectionType type = CameraProjectionType::PERSPECTIVE);

  // ==================== 投影参数控制 ====================
  void SetPerspective(float fov, float near, float far);
  void SetOrthographic(float size, float near, float far);
  void SetAspectRatio(float aspect);
  void SetProjectionType(CameraProjectionType type);
  void Zoom(float amount);

  // ==================== 主摄像机与摄像机标记 ====================
  CameraUsage GetUsage() const;
  void SetUsage(CameraUsage usage);

  // ==================== 矩阵获取 ====================
  glm::mat4 GetProjectionMatrix() const;

  // ==================== UBO获取 ====================
  void UpdateUBOViewMatrix(const glm::mat4 &viewMatrix);
  CameraInstance& GetCameraInstance();

  // ==================== 视口适配 ====================
  void SetViewportSize(uint32_t width, uint32_t height);

  // ==================== 组件接口 ====================
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;
  std::vector<std::type_index> GetDependencies() const override;

 private:
  std::shared_ptr<Camera> GetSnapshotData() const override;
  void SetSnapshotData(const std::shared_ptr<Camera> &data) override;

  CameraInstance m_CameraInstance; // 摄像机实例，管理std::shared_ptr<Camera>和CameraUBO
  CameraUsage m_Usage = CameraUsage::FreeView;
};

// 摄像机组件系统
class CameraComponentSystem : public SnapshotComponentSystem<CameraComponent> {
  DECLARE_COMPONENT_SYSTEM(CameraComponentSystem)
 public:
  std::vector<std::type_index> GetSystemDependencies() const override;

  // 获取Main相机实体
  Entity GetMainCameraEntity() const;

  // 设置主相机实体（确保唯一性）
  void SetMainCameraEntity(Entity mainCamera);
};

/**
 * @class MainCameraChangedEvent
 * @brief 主摄像机修改事件（暂未启用）
 */
class MainCameraChangedEvent : public ComponentEvent<CameraComponent> {
 public:
  MainCameraChangedEvent(Entity entity, CameraComponent &newMainCamera)
      : ComponentEvent<CameraComponent>(entity, component)
  {
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new MainCameraChangedEvent(entity, component);
  }
};

/**
 * @class CameraChangedEvent
 * @brief 摄像机修改事件
 */
class CameraChangedEvent : public ComponentEvent<CameraComponent> {
 public:
  CameraChangedEvent(Entity entity, CameraComponent &component)
      : ComponentEvent<CameraComponent>(entity, component)
  {
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new CameraChangedEvent(entity, component);
  }
};
};  // namespace mite

#endif
