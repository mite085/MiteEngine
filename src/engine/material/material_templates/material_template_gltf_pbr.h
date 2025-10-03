#ifndef MITE_MATERIAL_TEMPLATE_GLTF_PBR
#define MITE_MATERIAL_TEMPLATE_GLTF_PBR

#include "gbuffer_material_template.h"

namespace mite {

/**
 * @brief GLTF PBR材质模板（基于GLTF标准的物理渲染）
 * @note 继承GBufferMaterialTemplate，使用UBO方案
 */
class GLTFPBRMaterialTemplate : public GBufferMaterialTemplate {
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

  // ---- 核心接口重写 ----
  std::shared_ptr<MaterialInstance> CreateInstance(
      const MaterialSourceData &sourceData) const override;

  // ---- 参数设置 ----
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
  void SetDefaultNormalScale(float normalScale)
  {
    m_DefaultNormalScale = normalScale;
  }

 protected:
  // ---- 默认参数重写 ----
  glm::vec4 GetDefaultBaseColor() const override
  {
    return m_DefaultBaseColor;
  }
  float GetDefaultMetallic() const override
  {
    return m_DefaultMetallic;
  }
  float GetDefaultRoughness() const override
  {
    return m_DefaultRoughness;
  }
  glm::vec3 GetDefaultEmissionColor() const override
  {
    return m_DefaultEmissive;
  }
  float GetDefaultEmissionIntensity() const override
  {
    return 1.0f; // GLTF中强度整合在EmissionColor里
  }  
  float GetDefaultNormalScale() const override
  {
    return m_DefaultNormalScale;
  }
  float GetDefaultAlphaCutoff() const override
  {
    return m_DefaultAlphaCutoff;
  }
  bool GetDefaultDoubleSided() const override
  {
    return m_DefaultDoubleSided;
  }
  float GetDefaultAlphaMode() const override
  {
    switch (m_DefaultAlphaMode) {
      case AlphaMode::OPAQUE:
        return 0.0f;
      case AlphaMode::MASK:
        return 1.0f;
      case AlphaMode::BLEND:
        return 2.0f;
      default:
        return 0.0f;
    }
  }

  // ---- UBO数据填充重写 ----
  void FillUBOData(MaterialUniformBuffer &uboData,
                   const MaterialSourceData &sourceData) const override;

 private:
  // ---- GLTF特定默认参数 ----
  glm::vec4 m_DefaultBaseColor = glm::vec4(1.0f);  // RGBA
  float m_DefaultMetallic = 0.0f;
  float m_DefaultRoughness = 1.0f;
  glm::vec3 m_DefaultEmissive = glm::vec3(0.0f);
  AlphaMode m_DefaultAlphaMode = AlphaMode::OPAQUE;
  float m_DefaultAlphaCutoff = 0.5f;
  bool m_DefaultDoubleSided = false;
  float m_DefaultNormalScale = 1.0f;

  // ---- 内部辅助方法 ----

  /**
   * @brief 应用GLTF特定的纹理槽位
   * @param instance 材质实例
   * @param sourceData 源数据
   */
  void ApplyGLTFTextureSlots(std::shared_ptr<MaterialInstance> instance,
                             const MaterialSourceData &sourceData) const;
};

}  // namespace mite

#endif  // MITE_MATERIAL_TEMPLATE_GLTF_PBR
