#ifndef MITE_MATERIAL_PURE_COLOR
#define MITE_MATERIAL_PURE_COLOR

#include "material_template.h"

namespace mite {
/**
 * @brief 纯色材质模板（仅用于测试）
 * @note 职责：
 * 1. 仅接受一个Color("u_Color")参数并将其显示出来的简单着色模型
 * 2. 关联Basic着色器程序
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
    return "PureColorMaterial";
  }
  std::string GetMaterialType() const override
  {
    return StaticType();
  }

  // ---- 核心接口 ----
  std::unique_ptr<MaterialInstance> CreateInstance() const override;
  void ApplyParameters(MaterialInstance &instance) const override;
  
  // ---- 参数设置 ----
  void SetColor(const glm::vec3 &color)
  {
    m_Color = color;
  }

 protected:
  // ---- 默认参数 ----
  std::shared_ptr<OpenGLShader> m_Shader;  // 关联的PBR着色器程序
  glm::vec3 m_Color;                  // 默认基础颜色
};

/**
 * @brief PBR材质模板（基于物理的渲染）
 * @note 职责：
 * 1. 定义PBR材质的默认参数（albedo/roughness/metallic等）
 * 2. 关联PBR着色器程序
 * 3. 提供材质实例的创建和参数应用接口
 */
class PBRMaterialTemplate : public MaterialTemplate {
 public:
  /**
   * @brief 构造函数（需传入已编译的PBR Shader）
   * @param shader 关联的PBR着色器程序
   * @param defaultAlbedo 默认漫反射颜色（sRGB空间）
   * @param defaultRoughness 默认粗糙度（0.0-1.0）
   * @param defaultMetallic 默认金属度（0.0-1.0）
   */
  explicit PBRMaterialTemplate(std::shared_ptr<OpenGLShader> shader,
                               const glm::vec3 &defaultAlbedo = glm::vec3(0.8f),
                               float defaultRoughness = 0.5f,
                               float defaultMetallic = 0.0f);

  // ---- 类型声明 ----
  static std::string StaticType()
  {
    return "DefaultPBR";
  }
  std::string GetMaterialType() const override
  {
    return StaticType();
  }

  // ---- 核心接口 ----
  std::unique_ptr<MaterialInstance> CreateInstance() const override;
  void ApplyParameters(MaterialInstance &instance) const override;

  // ---- 参数设置 ----
  void SetDefaultAlbedo(const glm::vec3 &albedo)
  {
    m_DefaultAlbedo = albedo;
  }
  void SetDefaultRoughness(float roughness)
  {
    m_DefaultRoughness = roughness;
  }
  void SetDefaultMetallic(float metallic)
  {
    m_DefaultMetallic = metallic;
  }
  void SetDefaultTexture(const std::string &paramName, TextureGPUHandle texture);

 protected:
  // ---- 默认参数 ----
  std::shared_ptr<OpenGLShader> m_Shader;  // 关联的PBR着色器程序
  glm::vec3 m_DefaultAlbedo;               // 默认漫反射颜色
  float m_DefaultRoughness;                // 默认粗糙度
  float m_DefaultMetallic;                 // 默认金属度
  std::unordered_map<std::string, TextureGPUHandle> m_DefaultTextures;  // 默认纹理绑定
};

/**
 * @brief 透明材质模板（继承自PBR材质，扩展Alpha混合支持）
 */
class TransparentMaterialTemplate : public PBRMaterialTemplate {
 public:
  explicit TransparentMaterialTemplate(std::shared_ptr<OpenGLShader> shader,
                                       float defaultAlpha = 0.5f);

  // ---- 类型声明 ----
  static std::string StaticType()
  {
    return "TransparentPBR";
  }
  std::string GetMaterialType() const override
  {
    return StaticType();
  }

  std::unique_ptr<MaterialInstance> CreateInstance() const override;
  void ApplyParameters(MaterialInstance &instance) const override;

  void SetDefaultAlpha(float alpha)
  {
    m_DefaultAlpha = alpha;
  }

 private:
  float m_DefaultAlpha;  // 默认透明度（0.0-1.0）
};
};  // namespace mite

#endif
