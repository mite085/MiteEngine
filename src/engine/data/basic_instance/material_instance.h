#ifndef MITE_MATERIAL_INSTANCE
#define MITE_MATERIAL_INSTANCE

#include "basic_shader/shader.h"
#include "basic_shader/shader_ssbo.h"
#include "basic_shader/shader_ubo.h"
#include "basic_type/material_param_variant.h"

namespace mite {
// 定义纹理绑定函数类型
using ExternalTextureBindFunc = std::function<void(ExternalTextureType, TextureGPUHandle, TextureTarget)>;

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
  explicit MaterialInstance() = default;
  ~MaterialInstance() = default;

  // ===================== UBO管理 =====================
  /**
   * @brief 初始化模型UBO（创建时执行一次即可）
   * @return 是否初始化成功
   */
  void InitializeUBO();
  /**
   * @brief 设置着色器绑定（着色器初始化之后，执行一次即可）
   * @param shader 着色器对象
   * @note Initialize之后发布事件，由管理Shader的RenderContext接手负责绑定即可
   * (使用固定的绑定点执行显示绑定，无需手动管理)
   */
  //void SetupShaderBinding(std::shared_ptr<OpenGLShader> shader);
  /**
   * @brief 更新模型UBO数据（组件负责每帧Update）
   */
  void UpdateUBO();
  /**
   * @brief
   * 仅绑定纹理（假设着色器已绑定）（DrawCall之前绑定）
   * @param textureBindFunc 纹理绑定函数
   * @return 使用的纹理槽位数量
   */
  size_t BindTexturesOnly(ExternalTextureBindFunc textureBindFunc) const;
  /**
   * @brief 仅绑定UBO（不存在OverrideShader，UBO已经在Shader中注册好的BindingPoint）
   */
  void BindBuffersOnly() const;
  /**
   * @brief 前向渲染专用的Apply方法，按照顺序执行绑定操作（DrawCall之前绑定）
   * @param textureBindFunc 纹理绑定函数
   */
  void Apply(ExternalTextureBindFunc textureBindFunc) const;

  // ===================== MaterialUniformBuffer 设置接口 =====================
  // ---- 材质数据引用 ----
  MaterialUniformBuffer &GetMaterialData() { return m_MaterialData; }
  const MaterialUniformBuffer &GetMaterialData() const { return m_MaterialData; }

  // ---- 材质类型标识 ----
  void SetMaterialInfo(MaterialType type);
  MaterialType GetMaterialType() const;

  // ---- 基础PBR参数 ----
  void SetBaseColor(const glm::vec4 &color);
  glm::vec4 GetBaseColor() const { return m_MaterialData.baseColor; }
  void SetMetallic(float metallic);
  float GetMetallic() const { return m_MaterialData.metallicRoughnessAO.x; }
  void SetRoughness(float roughness);
  float GetRoughness() const { return m_MaterialData.metallicRoughnessAO.y; }
  void SetAO(float ao);
  float GetAO() const { return m_MaterialData.metallicRoughnessAO.z; }
  void SetEmission(const glm::vec4 &emission);  // RGB + Intensity(w)
  glm::vec4 GetEmission() const { return m_MaterialData.emission; }
  void SetNormalScale(float scale);
  float GetNormalScale() const { return m_MaterialData.normalScale.x; }

  // ---- NPR参数 (未启用) ----
  void SetNPRParameters(
      const glm::vec4 &params);  // rampThreshold, rampSmoothness, specularSize, outlineWidth
  glm::vec4 GetNPRParameters() const { return m_MaterialData.nprParameters; }
  void SetNPRColors(const glm::vec4 &colors);  // shadowTint(xyz), rimPower(w)
  glm::vec4 GetNPRColors() const { return m_MaterialData.nprColors; }
  void SetRampThreshold(float threshold);  // 单独的NPR参数设置
  float GetRampThreshold() const { return m_MaterialData.nprParameters.x; }
  void SetRampSmoothness(float smoothness);
  float GetRampSmoothness() const { return m_MaterialData.nprParameters.y; }
  void SetSpecularSize(float size);
  float GetSpecularSize() const { return m_MaterialData.nprParameters.z; }
  void SetOutlineWidth(float width);
  float GetOutlineWidth() const { return m_MaterialData.nprParameters.w; }
  void SetShadowTint(const glm::vec3 &tint);
  glm::vec3 GetShadowTint() const;
  void SetRimPower(float power);
  float GetRimPower() const { return m_MaterialData.nprColors.w; }

  // ---- 纹理标志设置 ----
  void SetBaseColorTextureEnabled(bool enabled);
  bool IsBaseColorTextureEnabled() const { return m_MaterialData.textureCNMROFlags.x > 0.0f; }
  void SetNormalTextureEnabled(bool enabled);
  bool IsNormalTextureEnabled() const { return m_MaterialData.textureCNMROFlags.y > 0.0f; }
  void SetMetallicRoughnessTextureEnabled(bool enabled);
  bool IsMetallicRoughnessTextureEnabled() const
  {
    return m_MaterialData.textureCNMROFlags.z > 0.0f;
  }
  void SetOcclusionTextureEnabled(bool enabled);
  bool IsOcclusionTextureEnabled() const { return m_MaterialData.textureCNMROFlags.w > 0.0f; }
  void SetEmissiveTextureEnabled(bool enabled);
  bool IsEmissiveTextureEnabled() const { return m_MaterialData.textureEmissionFlag.x > 0.0f; }

  // ---- 纹理参数设置 ----
  void SetBaseColorTexParams(const glm::vec4 &params);  // xy: scale, zw: offset
  glm::vec4 GetBaseColorTexParams() const { return m_MaterialData.baseColorTexParams; }
  void SetNormalTexParams(const glm::vec4 &params);
  glm::vec4 GetNormalTexParams() const { return m_MaterialData.normalTexParams; }
  void SetMRTexParams(const glm::vec4 &params);
  glm::vec4 GetMRTexParams() const { return m_MaterialData.mrTexParams; }
  void SetEmissiveTexParams(const glm::vec4 &params);
  glm::vec4 GetEmissiveTexParams() const { return m_MaterialData.emissiveTexParams; }
  void SetOcclusionTexParams(const glm::vec4 &params);
  glm::vec4 GetOcclusionTexParams() const { return m_MaterialData.occlusionTexParams; }

  // ---- 渲染属性设置 ----
  void SetAlphaCutoff(float cutoff);
  float GetAlphaCutoff() const { return m_MaterialData.renderProperties.x; }
  void SetDoubleSided(bool doubleSided);
  bool IsDoubleSided() const { return m_MaterialData.renderProperties.y > 0.0f; }
  void SetAlphaMode(int mode);  // ALPHA_MODE_OPAQUE, ALPHA_MODE_MASK, ALPHA_MODE_BLEND
  int GetAlphaMode() const { return static_cast<int>(m_MaterialData.renderProperties.z); }

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


  // --------------------- 属性访问 ---------------------
  std::string GetName() const;
  void SetName(const std::string &name);

  // --------------------- 辅助方法 ---------------------
  bool HasTextures() const { return !m_Textures.empty(); }
  size_t GetTextureCount() const { return m_Textures.size(); }

 private:
  std::string m_Name = "";
  std::shared_ptr<ShaderUBO> m_UBO;      // 关联的UBO对象
  MaterialUniformBuffer m_MaterialData;  // 材质参数数据

  // 纹理存储（使用预定义的绑定点）
  // 注意：仅外部纹理，ShadowMap和Gbuffer等内部纹理不使用此接口
  std::unordered_map<ExternalTextureType, TextureGPUSlot> m_Textures;

  // 内部方法
  void SetupTextureBinding(TextureGPUSlot texture,
                           ExternalTextureType type);
};
};  // namespace mite

#endif
