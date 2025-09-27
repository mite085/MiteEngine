#ifndef MITE_MATERIAL_PURE_COLOR
#define MITE_MATERIAL_PURE_COLOR

#include "material_template.h"

namespace mite {
/**
 * @brief 纯色材质模板（仅用于测试）
 * @note 职责：
 * 1. 仅接受一个Color("u_Color")参数并将其显示出来的简单着色模型
 * 2. 关联最简单的着色器程序
 */
class PureColorMaterialTemplate : public MaterialTemplate {
 public:
  /**
   * @brief 构造函数（需传入已编译的PBR Shader）
   * @param shader 关联的PBR着色器程序
   * @param defaultAlbedo 默认漫反射颜色（sRGB空间）
   */
  explicit PureColorMaterialTemplate(std::shared_ptr<OpenGLShader> shader,
                                     const glm::vec3 &color = glm::vec3(0.8f));

  // ---- 类型声明 ----
  static std::string StaticType()
  {
    return "PureColorMaterialTemplate";
  }
  std::string GetMaterialType() const override
  {
    return StaticType();
  }

  // ---- 核心接口 ----
  std::shared_ptr<MaterialInstance> CreateInstance(
      const MaterialSourceData &sourceData) const override;
  void ApplyDefaultParams(MaterialInstance &instance) const override;

  // ---- 默认参数设置 ----
  void SetColor(const glm::vec3 &color)
  {
    m_Color = color;
  }

 protected:
  // ---- 默认参数 ----
  glm::vec3 m_Color;  // 默认基础颜色

  // ---- 内部辅助方法 ----
  /**
   * @brief 应用参数到材质实例
   * @param instance 材质实例
   * @param sourceData 源数据
   */
  void ApplyParameters(MaterialInstance &instance, const MaterialSourceData &sourceData) const;


};
};  // namespace mite

#endif
