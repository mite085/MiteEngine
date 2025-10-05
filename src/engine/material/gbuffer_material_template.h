#ifndef MITE_GBUFFER_MATERIAL_TEMPLATE_H
#define MITE_GBUFFER_MATERIAL_TEMPLATE_H

#include "basic_shader/shader_binding_point_manager.h"
#include "basic_shader/shader_ubo.h"
#include "basic_type/asset_type.h"
#include "material_template.h"

namespace mite {
/**
 * @brief 完全基于UBO的GBuffer材质模板
 * @note 所有材质参数都通过UBO传递，不再使用单独的uniform
 */
class GBufferMaterialTemplate : public MaterialTemplate {
 public:

  explicit GBufferMaterialTemplate(std::shared_ptr<OpenGLShader> shader);
  virtual ~GBufferMaterialTemplate();

  // ==================== 核心接口重写 ====================
  std::shared_ptr<MaterialInstance> CreateInstance(
      const MaterialSourceData &sourceData) const override;

  void ApplyDefaultParams(std::shared_ptr<MaterialInstance> instance) const override;

  // ==================== UBO管理 ====================
  /**
   * @brief 为材质实例创建并设置独立的UBO
   * @param instance 材质实例
   * @param sourceData 材质源数据
   */
  void SetupInstanceUBO(std::shared_ptr<MaterialInstance> instance,
                        const MaterialSourceData &sourceData) const;
  /**
   * @brief 创建并初始化实例专用的UBO
   * @param sourceData 材质源数据
   * @return 初始化好的UBO对象
   */
  std::shared_ptr<ShaderUBO> CreateInstanceUBO(const MaterialUniformBuffer uniformdata) const;
  /**
   * @brief 根据材质源数据生成UBO数据
   * @param sourceData 材质源数据
   * @return 填充好的UBO数据结构
   */
  MaterialUniformBuffer CreateUBOData(const MaterialSourceData &sourceData) const;
  /**
   * @brief 获取绑定点
   */
  uint32_t GetBindingPoint() const
  {
    return m_BindingPoint;
  }
 protected:
  // ==================== 参数获取工具方法（PBR） ====================
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

  // ==================== 纹理处理工具方法 ====================
  void SetupTextures(std::shared_ptr<MaterialInstance> instance,
                     const MaterialSourceData &sourceData) const;

  void SetupTextureSlot(std::shared_ptr<MaterialInstance> instance,
                        const std::string &slotName,
                        const MaterialSourceData &sourceData) const;

  // ==================== 默认值设置（派生类可重写） ====================
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
    return glm::vec3(0.0f);  // 无自发光
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
    return 0.0f;   // 0 = OPAQUE不透明，参考handle_type.h中 enum class AlphaMode
  }

  // ==================== UBO相关 ====================
  virtual void FillUBOData(MaterialUniformBuffer &uboData,
                           const MaterialSourceData &sourceData) const;

 private:
  // UBO绑定点
  uint32_t m_BindingPoint;
};
}  // namespace mite

#endif  // MITE_GBUFFER_MATERIAL_TEMPLATE_H
