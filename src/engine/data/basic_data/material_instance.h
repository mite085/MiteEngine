#ifndef MITE_MATERIAL_INSTANCE
#define MITE_MATERIAL_INSTANCE

#include "basic_shader/shader.h"
#include "basic_shader/shader_ubo.h"
#include "basic_shader/shader_ssbo.h"
#include "basic_data/texture.h"
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

  // ---- UBO绑定设置 ----
  /**
   * @brief 绑定UBO到材质实例
   * @param uniformBlockName Uniform块名称（在Shader中定义）
   * @param ubo UBO对象
   * @param bindingPoint 绑定点索引
   */
  void BindUBO(const std::string &uniformBlockName,
               std::shared_ptr<ShaderUBO> ubo,
               uint32_t bindingPoint);
  /**
   * @brief 解绑UBO
   * @param uniformBlockName Uniform块名称
   */
  void UnbindUBO(const std::string &uniformBlockName);
  /**
   * @brief 检查是否已绑定指定UBO
   * @param uniformBlockName Uniform块名称
   * @return 是否已绑定
   */
  bool HasUBO(const std::string &uniformBlockName) const;

  // ---- SSBO绑定设置（新增）----
  /**
   * @brief 绑定SSBO到材质实例
   * @param storageBlockName 存储块名称（在Shader中定义）
   * @param ssbo SSBO对象
   * @param bindingPoint 绑定点索引
   */
  void BindSSBO(const std::string &storageBlockName,
                std::shared_ptr<ShaderSSBO> ssbo,
                uint32_t bindingPoint);
  /**
   * @brief 解绑SSBO
   * @param storageBlockName 存储块名称
   */
  void UnbindSSBO(const std::string &storageBlockName);
  /**
   * @brief 检查是否已绑定指定SSBO
   * @param storageBlockName 存储块名称
   * @return 是否已绑定
   */
  bool HasSSBO(const std::string &storageBlockName) const;

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

  // UBO绑定存储
  struct UBOBinding {
    std::shared_ptr<ShaderUBO> ubo;
    uint32_t bindingPoint;
  };
  std::unordered_map<std::string, UBOBinding> m_UBOBindings;  // UBO绑定
  // SSBO绑定存储
  struct SSBOBinding {
    std::shared_ptr<ShaderSSBO> ssbo;
    uint32_t bindingPoint;
  };
  std::unordered_map<std::string, SSBOBinding> m_SSBOBindings;  // SSBO绑定
};
};  // namespace mite

#endif
