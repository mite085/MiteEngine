#ifndef MITE_MATERIAL_PURE_COLOR
#define MITE_MATERIAL_PURE_COLOR

#include "material_template.h"

namespace mite {
/**
 * @brief 自发光材质模板
 * @note 直接继承自MaterialTemplate，使用统一的UBO系统
 */
class EmissionMaterialTemplate : public MaterialTemplate {
 public:
  explicit EmissionMaterialTemplate(const glm::vec3 &emissionColor = glm::vec3(0.8f, 0.0f, 0.0f));
  // ---- 核心接口实现 ----
  MaterialType GetMaterialType() const override { return MaterialType::EMISSION; }
  static MaterialType StaticType() { return MaterialType::EMISSION; }
  std::string GetMaterialTypeName() const override { return "Emission Material"; }

  // ---- 颜色设置 ----
  void SetEmissionColor(const glm::vec3 &color) { m_EmissionColor = color; }
  const glm::vec3 &GetEmissionColor() const { return m_EmissionColor; }

 protected:
  // ---- 默认值重写 ----
  glm::vec3 GetDefaultEmissionColor() const override { return m_EmissionColor; }
  float GetDefaultEmissionIntensity() const override { return 1.0f; }
 private:
  glm::vec3 m_EmissionColor;

};
};  // namespace mite

#endif
