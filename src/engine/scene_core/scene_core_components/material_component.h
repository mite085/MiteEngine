#ifndef MITE_SCENE_MATERIAL_COMPONENT
#define MITE_SCENE_MATERIAL_COMPONENT

#include "basic_instance/material_instance.h"
#include "scene_core/component_system.h"

namespace mite {
/**
 * @brief 材质组件，管理实体的渲染材质属性
 *
 * 功能特性：
 * 1. 管理着色器
 * 2. 支持PBR(基于物理的渲染)和传统材质
 * 3. 提供材质参数动态修改接口
 * 4. 支持材质实例化
 *
 * 设计考虑：
 * - 使用共享指针管理材质资源
 * - 与RendererSystem协同工作
 * - 支持GPU实例化
 */
class MaterialComponent
    : public SnapshotComponentTraits<MaterialInstance,
                                     Component::Family::Geometry> {
 public:
  /**
   * @brief 带初始值的构造函数
   * @param material 材质实例
   */
  explicit MaterialComponent();

  ~MaterialComponent() override = default;
  void Update(float deltaTime, SceneRegistry &registry) override;

  //===================== 材质基础操作 ===================
  /**
   * @brief 获取材质数据
   * @return 材质实例
   */
  std::shared_ptr<MaterialInstance> GetMaterialInstance() const;

  /**
   * @brief 设置材质实例
   */
  void SetMaterialInstance(std::shared_ptr<MaterialInstance> handle);

  //==================== 组件接口实现 ====================
  std::vector<std::type_index> GetDependencies() const override;
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

 private:
  const MaterialInstance &GetSnapshotData() const override;
  void SetSnapshotData(const MaterialInstance &data) override;

  std::shared_ptr<MaterialInstance> m_MaterialInstance;  // 材质实例
};

//====================== Material组件系统 ========================
class MaterialComponentSystem
    : public SnapshotComponentSystem<MaterialComponent> {
  DECLARE_COMPONENT_SYSTEM(MaterialComponentSystem)
 public:
  void Update(float deltaTime, SceneRegistry &registry) override;
  std::vector<std::type_index> GetSystemDependencies() const override;
};

//====================== Material组件事件 ========================
/**
 * @class MaterialChangedEvent
 * @brief 材质改变事件
 */
class MaterialChangedEvent : public ComponentEvent<MaterialComponent> {
 public:
  MaterialChangedEvent(Entity entity, MaterialComponent &component)
      : ComponentEvent<MaterialComponent>(entity, component) {}
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override {
    return new MaterialChangedEvent(this->m_Entity, this->m_Component);
  }
};
};  // namespace mite

#endif
