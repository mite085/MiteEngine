#ifndef MITE_SCENE_CAMERA_COMPONENT
#define MITE_SCENE_CAMERA_COMPONENT

#include "basic_data/camera.h"
#include "basic_type/camera_type.h"
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
class CameraComponent : public ComponentTraits<CameraComponent, Component::Family::Render> {
 public:
  CameraComponent(std::shared_ptr<Camera> camera);

  // ==================== 投影参数控制 ====================
  void SetPerspective(float fov, float near, float far);
  void SetOrthographic(float size, float near, float far);
  void Zoom(float amount);
  void SetAspectRatio(float aspect);

  // ==================== 主摄像机与摄像机标记 ====================
  CameraUsage GetUsage() const;
  void SetUsage(CameraUsage usage);
  std::shared_ptr<Camera> GetCamera();

  // ==================== 矩阵获取 ====================
  glm::mat4 GetProjectionMatrix() const;

  // ==================== 视口适配 ====================
  void SetViewportSize(uint32_t width, uint32_t height);

  // ==================== 可见性掩码 ====================
  /**
   * @brief 设置相机可见性掩码
   * @param mask 可见性掩码（使用CameraVisibilityMask中的定义）
   */
  void SetVisibilityMask(uint32_t mask);

  /**
   * @brief 获取相机可见性掩码
   * @return 当前可见性掩码
   */
  uint32_t GetVisibilityMask() const;

  /**
   * @brief 添加可见性层级
   * @param mask 要添加的掩码
   */
  void AddVisibilityLayer(uint32_t mask);

  /**
   * @brief 移除可见性层级
   * @param mask 要移除的掩码
   */
  void RemoveVisibilityLayer(uint32_t mask);

  /**
   * @brief 检查是否包含特定可见性层级
   * @param mask 要检查的掩码
   * @return 是否包含
   */
  bool HasVisibilityLayer(uint32_t mask) const;


  // ==================== 组件接口 ====================
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;
  std::vector<std::type_index> GetDependencies() const override;

 private:
  std::shared_ptr<Camera> m_Camera;
  CameraUsage m_Usage = CameraUsage::FreeView;
  uint32_t m_VisibilityMask = CameraVisibilityMask::ALL;  // 默认看到所有
};

// 摄像机组件系统
class CameraComponentSystem : public ComponentSystem<CameraComponent> {
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
 * @class CameraVisibilityMaskChangedEvent
 * @brief 摄像机掩码修改事件（暂未启用）
 */
class CameraVisibilityMaskChangedEvent : public ComponentEvent<CameraComponent> {
 public:
  CameraVisibilityMaskChangedEvent(Entity entity,
                                   CameraComponent &component,
                                   uint32_t newVisibilityMask)
      : ComponentEvent<CameraComponent>(entity, component),
        m_NewVisibilityMask(newVisibilityMask)
  {
  }

  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new CameraVisibilityMaskChangedEvent(entity, component, m_NewVisibilityMask);
  }
  uint32_t GetNewVisibleMask()
  {
    return m_NewVisibilityMask;
  }

 private:
  uint32_t m_NewVisibilityMask;
};
};  // namespace mite

#endif
