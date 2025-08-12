#ifndef MITE_SCENE_VISIBILITY_COMPONENT
#define MITE_SCENE_VISIBILITY_COMPONENT

#include "scene_core/component_system.h"

namespace mite {
// TODO: 占位符，后续完善了基本逻辑后替换
class AABB {
 public:
  AABB() = default;
  AABB(glm::vec3 a, glm::vec3 b) {}
  bool Serialize(std::ostream &output) const
  {
    return false;
  }
  bool Deserialize(std::istream &output) const
  {
    return false;
  }
  bool IsEmpty() const
  {
    return true;
  }
  void Transform(glm::mat4) {}
};
class Frustum {
 public:
  bool Intersects(AABB &) const
  {
    return false;
  }
};

/**
 * @brief 可见性组件，管理实体的渲染可见性和视锥体裁剪
 *
 * 功能特性：
 * 1. 控制实体是否参与渲染
 * 2. 管理视锥体裁剪状态
 * 3. 支持层级可见性控制
 * 4. 提供距离剔除功能
 *
 * 设计考虑：
 * - 与渲染管线紧密配合
 * - 支持静态和动态物体不同优化策略
 * - 提供调试可视化功能
 */
class VisibilityComponent
    : public ComponentTraits<VisibilityComponent, Component::Family::Render> {
 public:
  /**
   * @brief 默认构造函数
   */
  VisibilityComponent();

  ~VisibilityComponent() override = default;

  // 基础可见性控制 ======================================
  /**
   * @brief 设置是否可见
   * @param visible 可见性标志
   */
  void SetVisible(bool visible);

  /**
   * @brief 检查是否可见
   * @return 可见性状态
   */
  bool IsVisible() const;

  /**
   * @brief 切换可见状态
   */
  void ToggleVisible();

  // 视锥体裁剪 ==========================================
  /**
   * @brief 设置是否进行视锥体裁剪
   * @param cull 裁剪标志
   */
  void SetFrustumCulling(bool cull);

  /**
   * @brief 检查是否进行视锥体裁剪
   * @return 裁剪标志
   */
  bool GetFrustumCulling() const;

  /**
   * @brief 获取最后一次裁剪测试结果
   * @return 是否在视锥体内
   */
  bool WasInFrustum() const;

  /**
   * @brief 设置自定义包围盒(用于裁剪测试)
   * @param aabb 轴对齐包围盒
   */
  void SetCustomBounds(const AABB &aabb);

  /**
   * @brief 获取自定义包围盒
   * @return 轴对齐包围盒
   */
  AABB GetCustomBounds() const;

  // 距离剔除 ============================================
  /**
   * @brief 设置最大可见距离
   * @param distance 最大距离(0表示无限)
   */
  void SetMaxVisibleDistance(float distance);

  /**
   * @brief 获取最大可见距离
   * @return 最大距离
   */
  float GetMaxVisibleDistance() const;

  /**
   * @brief 获取最后一次距离测试结果
   * @return 是否在可见距离内
   */
  bool WasInDistance() const;

  // 层级可见性 ==========================================
  /**
   * @brief 设置层级可见性掩码
   * @param mask 可见性掩码(按位)
   */
  void SetLayerMask(uint32_t mask);

  /**
   * @brief 获取层级可见性掩码
   * @return 可见性掩码
   */
  uint32_t GetLayerMask() const;

  // 调试功能 ============================================
  /**
   * @brief 设置是否总是可见(调试用)
   * @param always 总是可见标志
   */
  void SetAlwaysVisible(bool always);

  /**
   * @brief 检查是否总是可见
   * @return 总是可见标志
   */
  bool IsAlwaysVisible() const;

  /**
   * @brief 设置显示包围盒(调试用)
   * @param show 显示标志
   */
  void SetShowBounds(bool show);

  /**
   * @brief 检查是否显示包围盒
   * @return 显示标志
   */
  bool GetShowBounds() const;

  // 组件接口实现 ========================================
  std::vector<std::type_index> GetDependencies() const override;
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

 private:
  bool m_IsVisible = true;       // 基础可见性
  bool m_FrustumCulling = true;  // 是否进行视锥体裁剪
  bool m_WasInFrustum = true;    // 最后一次裁剪测试结果
  bool m_WasInDistance = true;   // 最后一次距离测试结果
  bool m_AlwaysVisible = false;  // 调试用总是可见标志
  bool m_ShowBounds = false;     // 调试用显示包围盒标志

  AABB m_CustomBounds;                // 自定义包围盒(用于裁剪)
  float m_MaxVisibleDistance = 0.0f;  // 最大可见距离(0=无限)
  uint32_t m_LayerMask = 0xFFFFFFFF;  // 层级可见性掩码

  friend class VisibilityComponentSystem;
};

// Visibility组件系统 =========================================
class VisibilityComponentSystem : public DirtyComponentSystem<VisibilityComponent> {
  DECLARE_COMPONENT_SYSTEM(VisibilityComponentSystem)
 public:
  void Initialize(SceneRegistry &registry) override;
  void Shutdown(SceneRegistry &registry) override;
  void Update(float deltaTime, SceneRegistry &registry) override;

  /**
   * @brief 执行视锥体裁剪测试
   * @param frustum 视锥体
   * @param registry 场景注册表
   */
  void PerformFrustumCulling(const Frustum &frustum, SceneRegistry &registry);

  /**
   * @brief 执行距离裁剪测试
   * @param cameraPosition 摄像机位置
   * @param registry 场景注册表
   */
  void PerformDistanceCulling(const glm::vec3 &cameraPosition, SceneRegistry &registry);

 private:
  // 调试绘制
  void DebugDrawBounds(SceneRegistry &registry);
};

// Visibility组件事件 =========================================
/**
 * @class VisibilityChangedEvent
 * @brief 可见性改变事件
 */
class VisibilityChangedEvent : public ComponentEvent<VisibilityComponent> {
 public:
  VisibilityChangedEvent(Entity entity, VisibilityComponent &component, bool isVisible)
      : ComponentEvent<VisibilityComponent>(entity, component), m_IsVisible(isVisible)
  {
  }

  EVENT_CLASS_TYPE(VISIBILITY_COMPONENT_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new VisibilityChangedEvent(entity, component, m_IsVisible);
  }

  bool IsVisible() const
  {
    return m_IsVisible;
  }

 private:
  bool m_IsVisible;
};
};  // namespace mite

#endif
