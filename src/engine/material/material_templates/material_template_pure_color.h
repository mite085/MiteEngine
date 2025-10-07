#ifndef MITE_MATERIAL_PURE_COLOR
#define MITE_MATERIAL_PURE_COLOR

#include "material_template.h"

namespace mite {
/**
 * @brief 纯色材质模板（仅用于测试）
 * @note 直接继承自MaterialTemplate，使用统一的UBO系统
 */
class PureColorMaterialTemplate : public MaterialTemplate {
 public:
  explicit PureColorMaterialTemplate(std::shared_ptr<OpenGLShader> shader,
                                     const glm::vec3 &color = glm::vec3(0.8f, 0.0f, 0.0f));
  // ---- 核心接口实现 ----
  std::string GetMaterialType() const override { return "PureColorMaterial"; }
  static std::string StaticType() { return "PureColorMaterial"; }

  // ---- 颜色设置 ----
  void SetColor(const glm::vec3 &color) { m_Color = color; }
  const glm::vec3 &GetColor() const { return m_Color; }

 protected:
  // ---- 默认值重写 ----
  glm::vec4 GetDefaultBaseColor() const override { return glm::vec4(m_Color, 1.0f); }
  glm::vec3 GetDefaultEmissionColor() const override { return m_Color; }
  float GetDefaultEmissionIntensity() const override { return 1.0f; }  // 纯色材质使用自发光
 private:
  glm::vec3 m_Color;

};
};  // namespace mite

#endif
