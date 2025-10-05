#ifndef MITE_MATERIAL_PURE_COLOR
#define MITE_MATERIAL_PURE_COLOR

#include "gbuffer_material_template.h"

namespace mite {
/**
 * @brief 纯色材质模板（仅用于测试）
 * @note 基于GBufferMaterialTemplate，仅使用自发光颜色
 */
class PureColorMaterialTemplate : public GBufferMaterialTemplate {
 public:
  /**
   * @brief 构造函数
   * @param shader 关联的着色器程序
   * @param color 默认颜色（用于自发光）
   */
  explicit PureColorMaterialTemplate(std::shared_ptr<OpenGLShader> shader,
                                     const glm::vec3 &color = glm::vec3(0.8f, 0.0f, 0.0f));
  // ---- 类型声明 ----
  static std::string StaticType()
  {
    return "PureColorMaterial";
  }

  std::string GetMaterialType() const override
  {
    return StaticType();
  }
  // ---- 默认参数设置 ----
  void SetColor(const glm::vec3 &color)
  {
    m_Color = color;
  }

  const glm::vec3 &GetColor() const
  {
    return m_Color;
  }

 protected:
  // ---- 重写默认值设置 ----
  glm::vec4 GetDefaultBaseColor() const override
  {
    return glm::vec4(m_Color, 1.0f);
  }
  glm::vec3 GetDefaultEmissionColor() const override
  {
    return glm::vec3(m_Color);
  }

 private:
  glm::vec3 m_Color;  // 自发光颜色
};
};  // namespace mite

#endif
