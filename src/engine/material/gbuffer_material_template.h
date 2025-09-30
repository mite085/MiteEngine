#ifndef MITE_GBUFFER_MATERIAL_TEMPLATE_H
#define MITE_GBUFFER_MATERIAL_TEMPLATE_H

#include "basic_shader/shader_binding_point_manager.h"
#include "basic_shader/shader_ubo.h"
#include "basic_type/asset_type.h"
#include "material_template.h"

namespace mite {
/**
 * @brief GBuffer材质参数UBO结构体
 * @note 按照std140布局规则对齐，包含所有材质参数
 */
struct alignas(16) GBufferMaterialUBO {
  // ---- 基础PBR参数 ----
  glm::vec4 baseColor;            // RGB + Alpha (w分量)
  glm::vec4 metallicRoughnessAO;  // x: metallic, y: roughness, z: AO, w: unused
  glm::vec4 emission;             // RGB + Intensity (w分量)
  glm::vec4 normalScale;          // x: normal scale, yzw: unused

  // ---- 纹理标识和参数 ----
  glm::vec4 textureFlags;  // x: hasBaseColorTex, y: hasNormalTex, z: hasMRTex, w: hasEmissiveTex
  glm::vec4 baseColorTexParams;  // xy: scale, zw: offset
  glm::vec4 normalTexParams;     // xy: scale, zw: offset
  glm::vec4 mrTexParams;         // xy: scale, zw: offset
  glm::vec4 emissiveTexParams;   // xy: scale, zw: offset
  glm::vec4 occlusionTexParams;  // xy: scale, zw: offset

  // ---- 渲染属性 ----
  glm::vec4 renderProperties;  // x: alphaCutoff, y: doubleSided, z: alphaMode, w: unused

  // 填充到256字节对齐
  glm::vec4 padding[3];
};
/**
 * @brief 完全基于UBO的GBuffer材质模板
 * @note 所有材质参数都通过UBO传递，不再使用单独的uniform
 */
class GBufferMaterialTemplate : public MaterialTemplate {
 public:
  static constexpr const char *UBO_BLOCK_NAME = "MaterialUBO";

  explicit GBufferMaterialTemplate(std::shared_ptr<OpenGLShader> shader);
  virtual ~GBufferMaterialTemplate();
  // ---- 核心接口重写 ----
  std::shared_ptr<MaterialInstance> CreateInstance(
      const MaterialSourceData &sourceData) const override;

  void ApplyDefaultParams(MaterialInstance &instance) const override;
  // ---- UBO管理 ----
  void SetupMaterialUBO(std::shared_ptr<MaterialInstance> instance) const;
  void UpdateMaterialUBO(const MaterialSourceData &sourceData) const;

  // ---- 绑定点信息 ----
  uint32_t GetBindingPoint() const
  {
    return m_BindingPoint;
  }

 protected:
  // ---- 参数获取工具方法 ----
  glm::vec4 GetBaseColor(const MaterialSourceData &sourceData) const;
  float GetMetallic(const MaterialSourceData &sourceData) const;
  float GetRoughness(const MaterialSourceData &sourceData) const;
  float GetAO(const MaterialSourceData &sourceData) const;
  glm::vec3 GetEmissionColor(const MaterialSourceData &sourceData) const;
  float GetEmissionIntensity(const MaterialSourceData &sourceData) const;
  float GetNormalScale(const MaterialSourceData &sourceData) const;
  float GetAlphaCutoff(const MaterialSourceData &sourceData) const;
  bool GetDoubleSided(const MaterialSourceData &sourceData) const;
  float GetAlphaMode(const MaterialSourceData &sourceData) const;
  // ---- 纹理处理工具方法 ----
  void SetupTextures(std::shared_ptr<MaterialInstance> instance,
                     const MaterialSourceData &sourceData) const;

  void SetupTextureSlot(std::shared_ptr<MaterialInstance> instance,
                        const std::string &slotName,
                        const MaterialSourceData &sourceData) const;
  // ---- 默认值设置（派生类可重写） ----
  virtual glm::vec4 GetDefaultBaseColor() const
  {
    return glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
  }
  virtual float GetDefaultMetallic() const
  {
    return 0.0f;  // 非金属
  }
  virtual float GetDefaultRoughness() const
  {
    return 1.0f;  // 完全粗糙，避免镜面反射
  }
  virtual float GetDefaultAO() const
  {
    return 1.0f;  // 无环境光遮蔽
  }
  virtual glm::vec3 GetDefaultEmissionColor() const
  {
    return glm::vec3(0.0f); // 无自发光
  }
  virtual float GetDefaultEmissionIntensity() const
  {
    return 0.0f;  // 无自发光强度
  }
  virtual float GetDefaultNormalScale() const
  {
    return 1.0f;  // 默认法线缩放
  }
  virtual float GetDefaultAlphaCutoff() const
  {
    return 0.5f;  // Alpha测试阈值
  }
  virtual bool GetDefaultDoubleSided() const
  {
    return false;  // 关闭双面渲染
  }
  virtual float GetDefaultAlphaMode() const
  {
    return 0.0f;
  }  // 0 = OPAQUE
  // ---- UBO数据填充 ----
  virtual void FillUBOData(GBufferMaterialUBO &uboData,
                           const MaterialSourceData &sourceData) const;

 private:
  // UBO实例和绑定点
  mutable std::shared_ptr<ShaderUBO> m_MaterialUBO;
  mutable std::mutex m_UBOMutex;
  uint32_t m_BindingPoint;
  void InitializeUBO() const;
};
}  // namespace mite

#endif  // MITE_GBUFFER_MATERIAL_TEMPLATE_H
