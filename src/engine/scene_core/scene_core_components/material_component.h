#ifndef MITE_SCENE_MATERIAL_COMPONENT
#define MITE_SCENE_MATERIAL_COMPONENT

#include "scene_core/component_system.h"
#include "basic_instance/material_instance.h"

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
class MaterialComponent : public SnapshotComponentTraits<std::shared_ptr<MaterialInstance>,
                                                         Component::Family::Geometry> {
 public:
  /**
   * @brief 带初始值的构造函数
   * @param material 材质实例
   */
  explicit MaterialComponent(std::shared_ptr<MaterialInstance> handle);

  ~MaterialComponent() override = default;
  void Update(float deltaTime, SceneRegistry &registry) override;

  //===================== 材质基础操作 ===================
  /**
   * @brief 获取材质数据
   * @return 材质句柄
   */
  std::shared_ptr<MaterialInstance> GetMaterialInstanceHandel() const;

  /**
   * @brief 设置材质数据
   * @param material 新的材质数据
   */
  void SetMaterialInstanceHandel(std::shared_ptr<MaterialInstance> handle);

  //===================== 着色器控制 =====================
  /**
   * @brief 获取关联的着色器
   * @return 着色器指针
   */
  std::shared_ptr<OpenGLShader> GetShader() const;

  //==================== 材质参数快捷设置 ====================
  void SetFloatParam(const std::string &name, float value);
  void SetColorParam(const std::string &name, const glm::vec3 &color);
  void SetTextureParam(const std::string &name, TextureGPUSlot texture);

  //==================== 组件接口实现 ====================
  std::vector<std::type_index> GetDependencies() const override;
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

 private:
  std::shared_ptr<MaterialInstance> GetSnapshotData() const override;
  void SetSnapshotData(const std::shared_ptr<MaterialInstance> &data) override;

  std::shared_ptr<MaterialInstance> m_MaterialInstance;  // 材质实例
};

//====================== Material组件系统 ========================
class MaterialComponentSystem : public SnapshotComponentSystem<MaterialComponent> {
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
      : ComponentEvent<MaterialComponent>(entity, component)
  {
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new MaterialChangedEvent(entity, component);
  }
};

};  // namespace mite

#endif
