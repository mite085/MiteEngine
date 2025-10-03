#include "material_template_gltf_pbr.h"

namespace mite {

GLTFPBRMaterialTemplate::GLTFPBRMaterialTemplate(std::shared_ptr<OpenGLShader> shader)
    : GBufferMaterialTemplate(std::move(shader))
{
}

std::shared_ptr<MaterialInstance> GLTFPBRMaterialTemplate::CreateInstance(
    const MaterialSourceData &sourceData) const
{
  // 使用基类的CreateInstance，负责会设置UBO和基础纹理
  auto instance = GBufferMaterialTemplate::CreateInstance(sourceData);

  // 应用GLTF特定的纹理槽位（如果有特殊处理）
  ApplyGLTFTextureSlots(instance, sourceData);

  return instance;
}

void GLTFPBRMaterialTemplate::FillUBOData(MaterialUniformBuffer &uboData,
                                          const MaterialSourceData &sourceData) const
{
  // 直接调用基类的填充方法（基类就是按照GLTFPBR模板创建的）
  GBufferMaterialTemplate::FillUBOData(uboData, sourceData);
}

void GLTFPBRMaterialTemplate::ApplyGLTFTextureSlots(std::shared_ptr<MaterialInstance> instance,
                                                    const MaterialSourceData &sourceData) const
{
  // 基础颜色纹理 - 使用统一命名
  if (const TextureGPUSlot *slot = GetTextureSlot(sourceData,
                                                  MaterialParamKeys::BASE_COLOR_TEXTURE))
  {
    instance->SetTexture(MaterialParamKeys::BASE_COLOR_TEXTURE, *slot);
  }

  // 金属粗糙度纹理 - 使用统一命名
  if (const TextureGPUSlot *slot = GetTextureSlot(sourceData,
                                                  MaterialParamKeys::METALLIC_ROUGHNESS_TEXTURE))
  {
    instance->SetTexture(MaterialParamKeys::METALLIC_ROUGHNESS_TEXTURE, *slot);
  }

  // 法线纹理 - 使用统一命名
  if (const TextureGPUSlot *slot = GetTextureSlot(sourceData, MaterialParamKeys::NORMAL_TEXTURE)) {
    instance->SetTexture(MaterialParamKeys::NORMAL_TEXTURE, *slot);
  }

  // 自发光纹理 - 使用统一命名
  if (const TextureGPUSlot *slot = GetTextureSlot(sourceData, MaterialParamKeys::EMISSIVE_TEXTURE))
  {
    instance->SetTexture(MaterialParamKeys::EMISSIVE_TEXTURE, *slot);
  }

  // 环境光遮蔽纹理 - 使用统一命名
  if (const TextureGPUSlot *slot = GetTextureSlot(sourceData,
                                                  MaterialParamKeys::OCCLUSION_TEXTURE))
  {
    instance->SetTexture(MaterialParamKeys::OCCLUSION_TEXTURE, *slot);
  }
}

}  // namespace mite
