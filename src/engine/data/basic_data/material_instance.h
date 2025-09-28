#ifndef MITE_MATERIAL_INSTANCE
#define MITE_MATERIAL_INSTANCE

#include "basic_data/shader.h"
#include "basic_data/texture.h"
#include "basic_type/handle_type.h"
#include "basic_type/material_param_variant.h"

namespace mite {
// 定义纹理绑定函数类型
using TextureBindFunc = std::function<void(TextureGPUHandle, uint32_t)>;

/**
 * @brief 材质实例（运行时绑定具体Shader和参数）
 * @note 职责：
 * 1. 关联Material逻辑参数与具体Shader
 * 2. 管理Uniform状态和纹理绑定
 * 3. 提供渲染前的Apply接口
 *
 * 数据存储：
 * MaterialSystem：map<string, shared_ptr<MaterialInstance>> (材质工厂)
 * MaterialComponent：shared_ptr<MaterialInstance>（直接引用）
 * RenderableItem：MaterialInstance*（渲染时直接访问）
 * MaterialInstance.textures：TextureInstance（轻量化句柄+参数，基本无开销）
 */
class MaterialInstance {
 public:
  explicit MaterialInstance(std::shared_ptr<OpenGLShader> shader);
  ~MaterialInstance();

  // ---- 参数设置 ----
  void SetFloat(const std::string &name, float value);
  void SetInt(const std::string &name, int value);
  void SetVector2(const std::string &name, const glm::vec2 &value);
  void SetVector3(const std::string &name, const glm::vec3 &value);
  void SetVector4(const std::string &name, const glm::vec4 &value);
  void SetMatrix3(const std::string &name, const glm::mat3 &value);
  void SetMatrix4(const std::string &name, const glm::mat4 &value);

  // ---- 参数数组设置 ----
  void SetIntArray(const std::string &name, const int *values, size_t count);
  void SetFloatArray(const std::string &name, const float *values, size_t count);
  void SetVector3Array(const std::string &name, const glm::vec3 *values, size_t count);

  // ---- 纹理设置 ----
  void SetTexture(const std::string &name, TextureGPUSlot texture);
  // void SetTextureArray(const std::string &name, const std::vector<TextureAssetID> &textures);

  // ---- 状态控制 ----
  /**
   * @brief 应用材质到渲染管线（绑定Shader+上传Uniforms+绑定纹理）
   * @param textureBindFunc 纹理绑定函数（与Render device相关）
   * @param overrideShader 可选覆盖Shader（用于特殊渲染效果）
   */
  void Apply(TextureBindFunc textureBindFunc, OpenGLShader *overrideShader = nullptr) const;

  // ---- 属性访问 ----
  std::shared_ptr<OpenGLShader> GetShader() const;
  std::string GetName() const;
  void SetName(const std::string &name);

 private:
  std::string m_Name = "";
  std::shared_ptr<OpenGLShader> m_Shader;                       // 关联的Shader程序
  std::unordered_map<std::string, UniformVariant> m_Uniforms;   // Uniform值存储
  std::unordered_map<std::string, TextureGPUSlot> m_Textures;  // 纹理绑定
};
};  // namespace mite

#endif
