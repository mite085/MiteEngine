#ifndef MITE_MATERIAL_TEMPLATE_GLTF_PBR
#define MITE_MATERIAL_TEMPLATE_GLTF_PBR

#include "material_template.h"

namespace mite {

/**
 * @brief GLTF PBR材质模板（基于GLTF标准的物理渲染）
 * @note 职责：
 * 1. 支持GLTF PBR标准的所有参数（baseColorFactor, metallicFactor, roughnessFactor等）
 * 2. 支持GLTF标准纹理槽位（baseColorTexture, normalTexture等）
 * 3. 支持透明度模式（OPAQUE, MASK, BLEND）
 * 4. 支持双面渲染
 */
class GLTFPBRMaterialTemplate : public MaterialTemplate {
 public:
  /**
   * @brief 构造函数
   * @param shader 关联的PBR着色器程序
   */
  explicit GLTFPBRMaterialTemplate(std::shared_ptr<OpenGLShader> shader);

  // ---- 类型声明 ----
  static std::string StaticType()
  {
    return "GLTFPBRMaterialTemplate";
  }
  std::string GetMaterialType() const override
  {
    return StaticType();
  }

  // ---- 核心接口 ----
  std::shared_ptr<MaterialInstance> CreateInstance(
      const MaterialSourceData &sourceData) const override;
  void ApplyDefaultParams(MaterialInstance &instance) const override;

  // ---- 着色器访问 ----
  std::shared_ptr<OpenGLShader> GetShader() const
  {
    return m_Shader;
  }

  // ---- 默认参数设置 ----
  void SetDefaultBaseColor(const glm::vec4 &color)
  {
    m_DefaultBaseColor = color;
  }
  void SetDefaultMetallic(float metallic)
  {
    m_DefaultMetallic = metallic;
  }
  void SetDefaultRoughness(float roughness)
  {
    m_DefaultRoughness = roughness;
  }
  void SetDefaultEmissive(const glm::vec3 &emissive)
  {
    m_DefaultEmissive = emissive;
  }
  void SetDefaultAlphaMode(AlphaMode mode)
  {
    m_DefaultAlphaMode = mode;
  }
  void SetDefaultAlphaCutoff(float cutoff)
  {
    m_DefaultAlphaCutoff = cutoff;
  }
  void SetDefaultDoubleSided(bool doubleSided)
  {
    m_DefaultDoubleSided = doubleSided;
  }

 protected:
  // ---- 默认参数 ----
  
  glm::vec4 m_DefaultBaseColor = glm::vec4(1.0f);  // RGBA
  float m_DefaultMetallic = 0.0f;
  float m_DefaultRoughness = 1.0f;
  glm::vec3 m_DefaultEmissive = glm::vec3(0.0f);
  AlphaMode m_DefaultAlphaMode = AlphaMode::OPAQUE;
  float m_DefaultAlphaCutoff = 0.5f;
  bool m_DefaultDoubleSided = false;

  // ---- 内部辅助方法 ----

  /**
   * @brief 应用GLTF PBR参数到材质实例
   * @param instance 材质实例
   * @param sourceData 源数据
   */
  void ApplyPBRParameters(MaterialInstance &instance, const MaterialSourceData &sourceData) const;

  /**
   * @brief 应用纹理槽位到材质实例
   * @param instance 材质实例
   * @param sourceData 源数据
   */
  void ApplyTextureSlots(MaterialInstance &instance, const MaterialSourceData &sourceData) const;

  /**
   * @brief 应用渲染属性到材质实例
   * @param instance 材质实例
   * @param sourceData 源数据
   */
  void ApplyRenderProperties(MaterialInstance &instance,
                             const MaterialSourceData &sourceData) const;

  /**
   * @brief 设置透明度相关参数
   * @param instance 材质实例
   * @param alphaMode 透明度模式
   * @param alphaCutoff Alpha测试阈值
   */
  void SetupAlphaBlending(MaterialInstance &instance,
                          AlphaMode alphaMode,
                          float alphaCutoff) const;
};

}  // namespace mite

#endif
