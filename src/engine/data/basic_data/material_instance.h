#ifndef MITE_MATERIAL_INSTANCE
#define MITE_MATERIAL_INSTANCE

#include "basic_shader/shader.h"
#include "basic_shader/shader_ssbo.h"
#include "basic_shader/shader_ubo.h"
#include "basic_type/material_param_variant.h"

namespace mite {
// 定义纹理绑定函数类型
using TextureBindFunc = std::function<void(TextureGPUHandle, size_t)>;

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

  // --------------------- 参数设置 ---------------------
  void SetFloat(const std::string &name, float value);
  void SetInt(const std::string &name, int value);
  void SetVector2(const std::string &name, const glm::vec2 &value);
  void SetVector3(const std::string &name, const glm::vec3 &value);
  void SetVector4(const std::string &name, const glm::vec4 &value);
  void SetMatrix3(const std::string &name, const glm::mat3 &value);
  void SetMatrix4(const std::string &name, const glm::mat4 &value);

  // --------------------- 参数数组设置 ---------------------
  void SetIntArray(const std::string &name, const int *values, size_t count);
  void SetFloatArray(const std::string &name, const float *values, size_t count);
  void SetVector3Array(const std::string &name, const glm::vec3 *values, size_t count);

  // --------------------- 纹理设置 ---------------------
  void SetTexture(const std::string &name, TextureGPUSlot texture);

  // --------------------- UBO设置 ---------------------
  /**
   * @brief 新增/删除/查询UBO
   * @param uniformBlockName Uniform块名称（在Shader中定义）
   * @param ubo UBO对象
   * @param bindingPoint 绑定点索引
   */
  void SetupUBO(const std::string &uniformBlockName,
                std::shared_ptr<ShaderUBO> ubo,
                uint32_t bindingPoint);
  void UninstallUBO(const std::string &uniformBlockName);
  bool HasUBO(const std::string &uniformBlockName) const;

  // --------------------- SSBO设置 ---------------------
  /**
   * @brief 新增/删除/查询SSBO
   * @param storageBlockName 存储块名称（在Shader中定义）
   * @param ssbo SSBO对象
   * @param bindingPoint 绑定点索引
   */
  void SetupSSBO(const std::string &storageBlockName,
                 std::shared_ptr<ShaderSSBO> ssbo,
                 uint32_t bindingPoint);
  void UninstallSSBO(const std::string &storageBlockName);
  bool HasSSBO(const std::string &storageBlockName) const;

  // --------------------- 绑定相关 ---------------------
  /**
   * @brief 仅绑定着色器程序（不设置Uniforms）
   * @param overrideShader 可选覆盖着色器
   */
  void BindShaderOnly(OpenGLShader *overrideShader = nullptr) const;
  /**
   * @brief 仅上传Uniform参数（假设着色器已绑定）
   */
  void UploadUniformsOnly(OpenGLShader *overrideShader = nullptr) const;
  /**
   * @brief 仅绑定纹理（假设着色器已绑定）（原则上不存在OverrideShader，纹理采样器是特定材质专用的）
   * @param textureBindFunc 纹理绑定函数
   * @param startSlot 起始纹理槽位
   * @return 使用的纹理槽位数量
   */
  size_t BindTexturesOnly(TextureBindFunc textureBindFunc,
                          size_t startSlot = 0,
                          OpenGLShader *overrideShader = nullptr) const;
  /**
   * @brief 仅绑定UBO/SSBO（不存在OverrideShader，UBO/SSBO已经在Shader中注册好的BindingPoint）
   */
  void BindBuffersOnly() const;
  /**
   * @brief 前向渲染专用的Apply方法，按照顺序执行绑定操作。延迟渲染不应当使用该方法
   * @param textureBindFunc 纹理绑定函数
   */
  void Apply(TextureBindFunc textureBindFunc,
             size_t startSlot = 0,
             OpenGLShader *overrideShader = nullptr) const;

  // --------------------- 属性访问 ---------------------
  std::shared_ptr<OpenGLShader> GetShader() const;
  std::string GetName() const;
  void SetName(const std::string &name);

  // --------------------- 辅助方法 ---------------------
  bool HasTextures() const
  {
    return !m_Textures.empty();
  }
  size_t GetTextureCount() const
  {
    return m_Textures.size();
  }
  const auto &GetUniforms() const
  {
    return m_Uniforms;
  }
  const auto &GetTextures() const
  {
    return m_Textures;
  }

 private:
  std::string m_Name = "";
  std::shared_ptr<OpenGLShader> m_Shader;                      // 关联的Shader程序
  std::unordered_map<std::string, UniformVariant> m_Uniforms;  // Uniform值存储
  std::unordered_map<std::string, TextureGPUSlot>
      m_Textures;  // 纹理采样器绑定（仅外部纹理，ShadowMap和Gbuffer等内部纹理不使用此接口）

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
