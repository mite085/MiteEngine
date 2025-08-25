#ifndef MITE_VISIBILITY_COMPONENT_H
#define MITE_VISIBILITY_COMPONENT_H

#include "bounding_volumes_types.h"
#include "frustum.h"
#include "scene_core/component_system.h"

namespace mite {
/**
 * @class VisibilityComponent
 * @brief 可见性组件，用于管理实体的可见性状态和空间剔除
 *
 * 功能特性：
 * 1. 支持视锥体裁剪和遮挡剔除
 * 2. 维护世界空间包围盒用于快速相交测试
 * 3. 提供可见性掩码支持分层渲染
 * 4. 与SceneGraph协同工作实现高效的空间查询
 */
class VisibilityComponent
    : public ComponentTraits<VisibilityComponent, Component::Family::Geometry> {
 public:
  /**
   * @brief 默认构造函数
   */
  VisibilityComponent();

  /**
   * @brief 带初始包围盒的构造函数
   * @param localAABB 局部空间包围盒
   */
  explicit VisibilityComponent(const AABB &localAABB);

  ~VisibilityComponent() override = default;

  /**
   * @brief 处理脏标记，更新可见性状态
   */
  void ProcessDirty(float deltaTime, SceneRegistry &reg) override;

  // ==================== 可见性操作 ====================

  /**
   * @brief 获取当前可见性状态
   * @return 是否可见
   */
  bool IsVisible() const
  {
    return isVisible;
  }

  /**
   * @brief 设置可见性状态（手动覆盖）
   * @param visible 是否可见
   */
  void SetVisible(bool visible);

  /**
   * @brief 获取上一帧的可见性状态
   * @return 上一帧是否可见
   */
  bool WasVisible() const
  {
    return wasVisible;
  }

  /**
   * @brief 检查可见性状态是否发生变化
   * @return 是否发生变化
   */
  bool VisibilityChanged() const
  {
    return isVisible != wasVisible;
  }

  // ==================== 包围盒操作 ====================

  /**
   * @brief 获取局部空间包围盒
   * @return 局部AABB
   */
  const AABB &GetLocalAABB() const
  {
    return localAABB;
  }

  /**
   * @brief 设置局部空间包围盒
   * @param aabb 新的局部AABB
   */
  void SetLocalAABB(const AABB &aabb);

  /**
   * @brief 获取世界空间包围盒
   * @return 世界AABB
   */
  const AABB &GetWorldAABB() const
  {
    return worldAABB;
  }

  /**
   * @brief 获取世界空间包围球（用于快速剔除）
   * @return 世界包围球
   */
  Sphere GetWorldSphere() const;

  // ==================== 掩码操作 ====================

  /**
   * @brief 获取可见性掩码
   * @return 32位掩码
   */
  uint32_t GetVisibilityMask() const
  {
    return visibilityMask;
  }

  /**
   * @brief 设置可见性掩码
   * @param mask 新的掩码
   */
  void SetVisibilityMask(uint32_t mask);

  /**
   * @brief 检查是否与给定掩码匹配
   * @param cameraMask 相机掩码
   * @return 是否匹配
   */
  bool MatchesMask(uint32_t cameraMask) const
  {
    return (visibilityMask & cameraMask) != 0;
  }

  // ==================== 组件接口 ====================

  std::vector<std::type_index> GetDependencies() const override;
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

  /**
   * @brief 标记包围盒为脏状态（当变换改变时调用）
   */
  void MarkBoundsDirty();

  /**
   * @brief 检查包围盒是否需要更新
   * @return 是否为脏状态
   */
  bool IsBoundsDirty() const
  {
    return boundsDirty;
  }

 private:
  /**
   * @brief 更新世界空间包围盒
   * @param reg 场景注册表
   */
  void UpdateWorldAABB(SceneRegistry &reg);

  /**
   * @brief 执行视锥体裁剪测试
   * @param frustum 相机视锥体
   * @return 相交类型
   */
  IntersectionType TestFrustum(const Frustum &frustum) const;

 private:
  AABB localAABB;  ///< 局部空间包围盒
  AABB worldAABB;  ///< 世界空间包围盒（缓存）

  bool isVisible = true;    ///< 当前可见性状态
  bool wasVisible = false;  ///< 上一帧可见性状态（用于检测变化）

  uint32_t visibilityMask = 0xFFFFFFFF;  ///< 可见性掩码
  bool boundsDirty = true;               ///< 包围盒脏标记
  bool manualOverride = false;           ///< 手动覆盖标志
};

// ==================== 组件系统 ====================

/**
 * @class VisibilityComponentSystem
 * @brief 可见性组件系统，负责批量处理可见性计算和剔除
 */
class VisibilityComponentSystem : public DirtyComponentSystem<VisibilityComponent> {
  DECLARE_COMPONENT_SYSTEM(VisibilityComponentSystem)

 public:
  /**
   * @brief 设置主相机视锥体
   * @param frustum 相机视锥体
   */
  void SetMainCameraFrustum(const Frustum &frustum);

  /**
   * @brief 设置相机可见性掩码
   * @param mask 相机掩码
   */
  void SetCameraVisibilityMask(uint32_t mask);

  /**
   * @brief 获取当前可见实体数量
   * @return 可见实体数
   */
  size_t GetVisibleCount() const
  {
    return visibleCount;
  }

 protected:
  void ProcessDirtyComponents(float deltaTime, SceneRegistry &registry) override;

 private:
  Frustum mainCameraFrustum;                   ///< 主相机视锥体
  uint32_t cameraVisibilityMask = 0xFFFFFFFF;  ///< 相机可见性掩码
  size_t visibleCount = 0;                     ///< 当前可见实体计数
};

// ==================== 事件定义 ====================

/**
 * @class VisibilityChangedEvent
 * @brief 可见性改变事件
 */
class VisibilityChangedEvent : public ComponentEvent<VisibilityComponent> {
 public:
  VisibilityChangedEvent(Entity entity, VisibilityComponent &component, bool newVisibility)
      : ComponentEvent<VisibilityComponent>(entity, component), newVisibility(newVisibility)
  {
  }

  EVENT_CLASS_TYPE(VISIBILITY_COMPONENT_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)

  Event *Clone() const override
  {
    return new VisibilityChangedEvent(entity, component, newVisibility);
  }

  bool GetNewVisibility() const
  {
    return newVisibility;
  }

 private:
  bool newVisibility;
};
}  // namespace mite

#endif  // MITE_VISIBILITY_COMPONENT_H
