#ifndef MITE_SCENE_MATERIAL_COMPONENT
#define MITE_SCENE_MATERIAL_COMPONENT

#include "scene_core/component_system.h"

namespace mite {
// TODO: 占位符，后续完善了基本逻辑后替换
class FakeTexture {};
class Shader {};
enum class BlendMode { Opaque };
class Material {
 public:
  void SetShader(std::shared_ptr<Shader> shader)
  {
    m_Shader = shader;
  }
  std::shared_ptr<Shader> GetShader() const
  {
    return m_Shader;
  }
  void SetBaseColor(const glm::vec4& color) {
    m_BaseColor = color;
  }
  glm::vec4 GetBaseColor() const
  {
    return m_BaseColor;
  }
  void SetMetallic(float metallic)
  {
    m_Metallic = metallic;
  }
  float GetMetallic() const
  {
    return m_Metallic;
  }
  void SetRoughness(float roughness)
  {
    m_Roughness = roughness;
  }
  float GetRoughness() const
  {
    return m_Roughness;
  }
  void SetEmissive(glm::vec3 emissive)
  {
    m_Emissive = emissive;
  }
  glm::vec3 GetEmissive() const
  {
    return m_Emissive;
  }
  void SetBaseColorTexture(std::shared_ptr<FakeTexture> texture) {
    m_BaseColorTexture = texture;
  }
  void SetNormalTexture(std::shared_ptr<FakeTexture> texture)
  {
    m_NormalTexture = texture;
  }
  void SetMetallicRoughnessTexture(std::shared_ptr<FakeTexture> texture)
  {
    m_MetallicRoughnessTexture = texture;
  }
  void SetBlendMode(BlendMode blendMode)
  {
    m_BlendMode = blendMode;
  }
  BlendMode GetBlendMode() const
  {
    return m_BlendMode;
  }
  void SetDoubleSided(bool doubleSided)
  {
    m_DoubleSided = doubleSided;
  }
  bool IsDoubleSided() const
  {
    return m_DoubleSided;
  }
 private:
  std::shared_ptr<Shader> m_Shader;
  glm::vec4 m_BaseColor;
  float m_Metallic, m_Roughness;
  glm::vec3 m_Emissive;
  std::shared_ptr<FakeTexture> m_BaseColorTexture, m_NormalTexture, m_MetallicRoughnessTexture;
  BlendMode m_BlendMode;
  bool m_DoubleSided;
};

/**
 * @brief 材质组件，管理实体的渲染材质属性
 *
 * 功能特性：
 * 1. 管理材质属性和着色器
 * 2. 支持PBR(基于物理的渲染)和传统材质
 * 3. 提供材质参数动态修改接口
 * 4. 支持材质实例化
 *
 * 设计考虑：
 * - 使用共享指针管理材质资源
 * - 与RendererSystem协同工作
 * - 支持GPU实例化
 */
class MaterialComponent : public ComponentTraits<MaterialComponent, Component::Family::Render> {
 public:
  // TODO: 为兼容最小RenderableEntity的ID，后续完善了基本逻辑后替换
  uint32_t assetID = 0;
  /**
   * @brief 默认构造函数
   */
  MaterialComponent();

  /**
   * @brief 带初始值的构造函数
   * @param material 材质数据
   */
  explicit MaterialComponent(std::shared_ptr<Material> material);

  ~MaterialComponent() override = default;

  // 材质基础操作 ========================================
  /**
   * @brief 获取材质数据
   * @return 共享指针指向的材质数据
   */
  std::shared_ptr<Material> GetMaterial() const;

  /**
   * @brief 设置材质数据
   * @param material 新的材质数据
   */
  void SetMaterial(std::shared_ptr<Material> material);

  /**
   * @brief 检查是否有有效材质数据
   * @return 是否有效
   */
  bool HasMaterial() const;

  // 着色器控制 ==========================================
  /**
   * @brief 获取关联的着色器
   * @return 着色器指针
   */
  std::shared_ptr<Shader> GetShader() const;

  /**
   * @brief 设置使用的着色器
   * @param shader 新的着色器
   */
  void SetShader(std::shared_ptr<Shader> shader);

  // 材质参数控制 ========================================
  /**
   * @brief 设置基础颜色
   * @param color RGBA颜色值
   */
  void SetBaseColor(const glm::vec4 &color);

  /**
   * @brief 获取基础颜色
   * @return RGBA颜色值
   */
  glm::vec4 GetBaseColor() const;

  /**
   * @brief 设置金属度
   * @param metallic 金属度(0-1)
   */
  void SetMetallic(float metallic);

  /**
   * @brief 获取金属度
   * @return 金属度值
   */
  float GetMetallic() const;

  /**
   * @brief 设置粗糙度
   * @param roughness 粗糙度(0-1)
   */
  void SetRoughness(float roughness);

  /**
   * @brief 获取粗糙度
   * @return 粗糙度值
   */
  float GetRoughness() const;

  /**
   * @brief 设置自发光颜色
   * @param emissive 自发光RGB颜色值
   */
  void SetEmissive(const glm::vec3 &emissive);

  /**
   * @brief 获取自发光颜色
   * @return 自发光RGB颜色值
   */
  glm::vec3 GetEmissive() const;

  // 纹理控制 ============================================
  /**
   * @brief 设置基础颜色贴图
   * @param texture 纹理对象
   */
  void SetBaseColorTexture(std::shared_ptr<FakeTexture> texture);

  /**
   * @brief 设置法线贴图
   * @param texture 纹理对象
   */
  void SetNormalTexture(std::shared_ptr<FakeTexture> texture);

  /**
   * @brief 设置金属粗糙度贴图
   * @param texture 纹理对象
   */
  void SetMetallicRoughnessTexture(std::shared_ptr<FakeTexture> texture);

  // 渲染状态控制 ========================================
  /**
   * @brief 设置混合模式
   * @param blendMode 混合模式枚举
   */
  void SetBlendMode(BlendMode blendMode);

  /**
   * @brief 获取混合模式
   * @return 混合模式枚举
   */
  BlendMode GetBlendMode() const;

  /**
   * @brief 设置是否双面渲染
   * @param doubleSided 双面标志
   */
  void SetDoubleSided(bool doubleSided);

  /**
   * @brief 检查是否双面渲染
   * @return 双面标志
   */
  bool IsDoubleSided() const;

  // 组件接口实现 ========================================
  std::vector<std::type_index> GetDependencies() const override;
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

 private:
  std::shared_ptr<Material> m_Material;  // 材质数据
};

// Material组件系统 ==============================================
class MaterialSystem : public DirtyComponentSystem<MaterialComponent> {
  DECLARE_COMPONENT_SYSTEM(MaterialSystem)
 public:
  void Initialize(SceneRegistry &registry) override;
  void Shutdown(SceneRegistry &registry) override;
  void Update(float deltaTime, SceneRegistry &registry) override;
};

// Material组件事件 ==============================================
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

  EVENT_CLASS_TYPE(COMPONENT_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new MaterialChangedEvent(entity, component);
  }
};

/**
 * @class ShaderChangedEvent
 * @brief 着色器改变事件
 */
class ShaderChangedEvent : public ComponentEvent<MaterialComponent> {
 public:
  ShaderChangedEvent(Entity entity, MaterialComponent &component)
      : ComponentEvent<MaterialComponent>(entity, component)
  {
  }

  EVENT_CLASS_TYPE(COMPONENT_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override
  {
    return new ShaderChangedEvent(entity, component);
  }
};
};  // namespace mite

#endif
