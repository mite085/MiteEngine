#ifndef MITE_SCENE_LIGHT_COMPONENT
#define MITE_SCENE_LIGHT_COMPONENT

#include "light_core/light.h"
#include "scene_core/component_system.h"
namespace mite {
/**
 * @brief 光源组件，管理实体的光源属性
 *
 * 功能特性：
 * 1. 维护std::shared_ptr<Light>，管理光源的光学属性
 * 2. 提供光源属性的便捷访问接口
 * 3. 与变换组件配合，为渲染系统提供完整的光源数据
 */
class LightComponent
    : public SnapshotComponentTraits<Light, Component::Family::Geometry> {
 public:
  /**
   * @brief 默认构造函数
   */
  LightComponent();
  /**
   * @brief 使用现有光源指针的构造函数
   * @param light 光源指针
   */
  explicit LightComponent(std::shared_ptr<Light> light);
  ~LightComponent() override = default;
  std::vector<std::type_index> GetDependencies() const override;

  // ==================== 光源管理 ====================
  void SetLight(std::shared_ptr<Light> light);
  std::shared_ptr<Light> GetLight() const;
  bool HasLight() const;

  // ==================== 类型相关 ====================
  LightType GetLightType() const;
  std::string GetLightTypeName() const;

  // ==================== 基础属性访问 ====================
  void SetColor(const glm::vec3 &color);
  const glm::vec3 &GetColor() const;
  void SetIntensity(float intensity);
  float GetIntensity() const;
  void SetEnabled(bool enabled);
  bool IsEnabled() const;

  // ==================== 完整属性访问 ====================
  LightProperties &GetProperties();
  const LightProperties &GetProperties() const;
  void SetProperties(const LightProperties &props);

  // ==================== 序列化接口 ====================
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

 protected:
  // ==================== 快照接口 ====================
  const Light &GetSnapshotData() const override;
  void SetSnapshotData(const Light &data) override;

 private:
  std::shared_ptr<Light> m_Light;  // 光源对象指针
};
// ==================== 组件系统 ====================
class LightComponentSystem : public ComponentSystem<LightComponent> {
  DECLARE_COMPONENT_SYSTEM(LightComponentSystem)
};
// ==================== 事件定义 ====================
/**
 * @class LightUpdatedEvent
 * @brief Light组件更新事件
 */
class LightUpdatedEvent : public ComponentEvent<LightComponent> {
 public:
  LightUpdatedEvent(Entity entity, LightComponent &component)
      : ComponentEvent<LightComponent>(entity, component)
  {
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new LightUpdatedEvent(this->m_Entity, this->m_Component);
  }
};
}  // namespace mite
#endif  // MITE_SCENE_LIGHT_COMPONENT
