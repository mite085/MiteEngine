#ifndef MITE_MATERIAL_INSTANCE
#define MITE_MATERIAL_INSTANCE

#include "basic_shader/shader.h"
#include "basic_shader/shader_ssbo.h"
#include "basic_shader/shader_ubo.h"
#include "basic_type/material_type.h"
#include "basic_type/material_param_variant.h"

namespace mite {
// 定义纹理绑定函数类型
using TextureBindFunc = std::function<void(TextureGPUHandle, size_t)>;

/**
 * @brief 材质实例（运行时绑定具体Shader和参数）
 * @note 职责：
 * 1. 关联Material逻辑参数与具体Shader
 * 2. 管理Uniform状态和纹理绑定（不再支持单独的uniform变量，所有数据通过UBO传递）
 * 3. 提供渲染前的Apply接口
 *
 * 数据存储：
 * AssetManager：std::shared_ptr<MaterialInstance>（材质资产化）
 * MaterialComponent：std::shared_ptr<MaterialInstance>（直接引用）
 * RenderableItem：std::shared_ptr<MaterialInstance>（渲染时访问）
 * MaterialInstance.textures：TextureInstance（轻量化句柄+参数，基本无开销）
 */
class MaterialInstance {
 public:
  explicit MaterialInstance(std::shared_ptr<OpenGLShader> shader);
  ~MaterialInstance();
  // ===================== UBO管理 =====================
  /**
   * @brief 初始化材质UBO（自动分配绑定点）
   */
  void InitializeUBO();
  /**
   * @brief 更新UBO数据到GPU
   */
  void UpdateUBO();
  std::shared_ptr<ShaderUBO> GetUBO() const
  {
    return m_UBO;
  }

  // ===================== MaterialUniformBuffer 设置接口 =====================
  // 
  // 获取材质数据引用（直接修改，修改后需要手动调用UpdateUBO）
  MaterialUniformBuffer &GetMaterialData()
  {
	return m_MaterialData;
  }

  // ---- 基础PBR参数 ----
  void SetMaterialInfo(MaterialType type);
  void SetBaseColor(const glm::vec4 &color);
  void SetMetallic(float metallic);
  void SetRoughness(float roughness);
  void SetAO(float ao);
  void SetEmission(const glm::vec4 &emission);  // RGB + Intensity(w)
  void SetNormalScale(float scale);

  // ---- NPR参数 (未启用) ----
  void SetNPRParameters(
      const glm::vec4 &params);  // rampThreshold, rampSmoothness, specularSize, outlineWidth
  void SetNPRColors(const glm::vec4 &colors);  // shadowTint(xyz), rimPower(w)
  void SetRampThreshold(float threshold);      // 单独的NPR参数设置
  void SetRampSmoothness(float smoothness);
  void SetSpecularSize(float size);
  void SetOutlineWidth(float width);
  void SetShadowTint(const glm::vec3 &tint);
  void SetRimPower(float power);

  // ---- 纹理标志设置 ----
  void SetBaseColorTextureEnabled(bool enabled);
  void SetNormalTextureEnabled(bool enabled);
  void SetMetallicRoughnessTextureEnabled(bool enabled);
  void SetOcclusionTextureEnabled(bool enabled);
  void SetEmissiveTextureEnabled(bool enabled);

  // ---- 纹理参数设置 ----
  void SetBaseColorTexParams(const glm::vec4 &params);  // xy: scale, zw: offset
  void SetNormalTexParams(const glm::vec4 &params);
  void SetMRTexParams(const glm::vec4 &params);
  void SetEmissiveTexParams(const glm::vec4 &params);
  void SetOcclusionTexParams(const glm::vec4 &params);

  // ---- 渲染属性设置 ----
  void SetAlphaCutoff(float cutoff);
  void SetDoubleSided(bool doubleSided);
  void SetAlphaMode(int mode);  // ALPHA_MODE_OPAQUE, ALPHA_MODE_MASK, ALPHA_MODE_BLEND

  // ===================== 纹理绑定 =====================
  /**
   * @brief 设置纹理（使用预定义的绑定点）
   */
  void SetBaseColorTexture(TextureGPUSlot texture);
  void SetNormalTexture(TextureGPUSlot texture);
  void SetMetallicRoughnessTexture(TextureGPUSlot texture);
  void SetEmissiveTexture(TextureGPUSlot texture);
  void SetOcclusionTexture(TextureGPUSlot texture);

  // --------------------- 绑定相关 ---------------------
  /**
   * @brief 仅绑定着色器程序（不设置Uniforms）
   * @param overrideShader 可选覆盖着色器
   */
  void BindShaderOnly(OpenGLShader *overrideShader = nullptr) const;
  /**
   * @brief
   * 仅绑定纹理（假设着色器已绑定）（原则上不存在OverrideShader，纹理采样器是特定材质专用的）
   * @param textureBindFunc 纹理绑定函数
   * @return 使用的纹理槽位数量
   */
  size_t BindTexturesOnly(TextureBindFunc textureBindFunc,
                          OpenGLShader *overrideShader = nullptr) const;
  /**
   * @brief 仅绑定UBO（不存在OverrideShader，UBO已经在Shader中注册好的BindingPoint）
   */
  void BindBuffersOnly() const;
  /**
   * @brief 前向渲染专用的Apply方法，按照顺序执行绑定操作。延迟渲染不应当使用该方法
   * @param textureBindFunc 纹理绑定函数
   */
  void Apply(TextureBindFunc textureBindFunc,
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

 private:
  std::string m_Name = "";
  std::shared_ptr<OpenGLShader> m_Shader;  // 关联的Shader程序
  std::shared_ptr<ShaderUBO> m_UBO;        // 关联的UBO对象
  MaterialUniformBuffer m_MaterialData;    // 材质参数数据

  // 纹理存储（使用预定义的绑定点）
  // 注意：仅外部纹理，ShadowMap和Gbuffer等内部纹理不使用此接口
  struct TextureSlot {
    TextureGPUSlot texture;
    uint32_t bindingPoint;
    std::string samplerName;
  };
  std::vector<TextureSlot> m_Textures;

  // 内部方法
  void SetupTextureBinding(TextureGPUSlot texture,
                           uint32_t bindingPoint,
                           const std::string &samplerName);
};
};  // namespace mite

#endif
