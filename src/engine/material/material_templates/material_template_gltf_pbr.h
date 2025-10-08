#ifndef MITE_MATERIAL_TEMPLATE_GLTF_PBR
#define MITE_MATERIAL_TEMPLATE_GLTF_PBR

#include "material_template.h"

namespace mite {
/**
 * @brief GLTF PBR材质模板（基于GLTF标准的物理渲染）
 * @note 继承自MaterialTemplate，使用统一的UBO系统
 */
class GLTFPBRMaterialTemplate : public MaterialTemplate {
 public:
  /**
   * @brief 构造函数
   * @param shader 关联的PBR着色器程序
   */
  explicit GLTFPBRMaterialTemplate();

  // ---- 核心接口重写 ----
  MaterialType GetMaterialType() const override { return MaterialType::PBR; }
  static MaterialType StaticType() { return MaterialType::PBR; }
  std::string GetMaterialTypeName() const override { return "GLTF PBR Material"; }

  // ---- GLTF特定参数设置 ----
  void SetDefaultBaseColor(const glm::vec4 &color) { m_DefaultBaseColor = color; }
  void SetDefaultMetallic(float metallic) { m_DefaultMetallic = metallic; }
  void SetDefaultRoughness(float roughness) { m_DefaultRoughness = roughness; }
  void SetDefaultEmissive(const glm::vec3 &emissive) { m_DefaultEmissive = emissive; }
  void SetDefaultAlphaMode(AlphaMode mode) { m_DefaultAlphaMode = mode; }
  void SetDefaultAlphaCutoff(float cutoff) { m_DefaultAlphaCutoff = cutoff; }
  void SetDefaultDoubleSided(bool doubleSided) { m_DefaultDoubleSided = doubleSided; }
  void SetDefaultNormalScale(float normalScale) { m_DefaultNormalScale = normalScale; }

 protected:
  // ---- 默认值重写 ----
  glm::vec4 GetDefaultBaseColor() const override { return m_DefaultBaseColor; }
  float GetDefaultMetallic() const override { return m_DefaultMetallic; }
  float GetDefaultRoughness() const override { return m_DefaultRoughness; }
  glm::vec3 GetDefaultEmissionColor() const override { return m_DefaultEmissive; }
  float GetDefaultEmissionIntensity() const override { return 1.0f; }
  float GetDefaultNormalScale() const override { return m_DefaultNormalScale; }
  float GetDefaultAlphaCutoff() const override { return m_DefaultAlphaCutoff; }
  bool GetDefaultDoubleSided() const override { return m_DefaultDoubleSided; }
  int GetDefaultAlphaMode() const override { return static_cast<int>(m_DefaultAlphaMode); }

 private:
  // ---- GLTF特定默认参数 ----
  glm::vec4 m_DefaultBaseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  float m_DefaultMetallic = 0.0f;
  float m_DefaultRoughness = 1.0f;
  glm::vec3 m_DefaultEmissive = glm::vec3(0.0f);
  AlphaMode m_DefaultAlphaMode = AlphaMode::OPAQUE;
  float m_DefaultAlphaCutoff = 0.5f;
  bool m_DefaultDoubleSided = false;
  float m_DefaultNormalScale = 1.0f;
};
}  // namespace mite

#endif  // MITE_MATERIAL_TEMPLATE_GLTF_PBR
