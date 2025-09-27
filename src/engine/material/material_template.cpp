#include "material_template.h"

namespace mite {
MaterialTemplate::MaterialTemplate(std::shared_ptr<OpenGLShader> shader) : m_Shader(shader)
{
  assert(m_Shader != nullptr && "MaterialTemplate: Shader cannot be nullptr");
}

std::shared_ptr<MaterialInstance> MaterialTemplate::CreateInstance()
{
  auto instance = std::make_unique<MaterialInstance>(m_Shader);

  // 直接使用默认参数
  ApplyDefaultParams(*instance);
  return instance;
}

void MaterialTemplate::SetName(const std::string &name)
{
  m_Name = name;
}
const std::string &MaterialTemplate::GetName() const
{
  return m_Name;
}

const TextureGPUSlot *MaterialTemplate::GetTextureSlot(const MaterialSourceData &sourceData,
                                                       const std::string &slotName)
{
  auto it = sourceData.textureSlots.find(slotName);
  if (it != sourceData.textureSlots.end()) {
    return &it->second;
  }
  return nullptr;
}

bool MaterialTemplate::HasParameter(const MaterialSourceData &sourceData, const std::string &key)
{
  return sourceData.parameters.find(key) != sourceData.parameters.end();
}

bool MaterialTemplate::HasTextureSlot(const MaterialSourceData &sourceData,
                                      const std::string &slotName)
{
  return sourceData.textureSlots.find(slotName) != sourceData.textureSlots.end();
}
void MaterialTemplate::ApplySourceDataToInstance(MaterialInstance &instance,
                                                 const MaterialSourceData &sourceData)
{
  // 应用标量参数
  for (const auto &[key, variant] : sourceData.parameters) {
    switch (variant.GetType()) {
      case UniformVariant::Type::Float:
        instance.SetFloat(key, variant.Get<float>());
        break;
      case UniformVariant::Type::Int:
        instance.SetInt(key, variant.Get<int>());
        break;
      case UniformVariant::Type::Vector2:
        instance.SetVector2(key, variant.Get<glm::vec2>());
        break;
      case UniformVariant::Type::Vector3:
        instance.SetVector3(key, variant.Get<glm::vec3>());
        break;
      case UniformVariant::Type::Vector4:
        instance.SetVector4(key, variant.Get<glm::vec4>());
        break;
      case UniformVariant::Type::Matrix3:
        instance.SetMatrix3(key, variant.Get<glm::mat3>());
        break;
      case UniformVariant::Type::Matrix4:
        instance.SetMatrix4(key, variant.Get<glm::mat4>());
        break;
      case UniformVariant::Type::IntArray: {
        auto [ptr, count] = variant.GetArray<int>();
        instance.SetIntArray(key, ptr, count);
        break;
      }
      case UniformVariant::Type::FloatArray: {
        auto [ptr, count] = variant.GetArray<float>();
        instance.SetFloatArray(key, ptr, count);
        break;
      }
      case UniformVariant::Type::Vector3Array: {
        auto [ptr, count] = variant.GetArray<glm::vec3>();
        instance.SetVector3Array(key, ptr, count);
        break;
      }
      default:
        // 跳过不支持的参数类型（如Texture）
        break;
    }
  }
  // 应用纹理参数
  for (const auto &[slotName, textureSlot] : sourceData.textureSlots) {
    instance.SetTexture(slotName, textureSlot);
  }
}

AlphaMode MaterialTemplate::GetAlphaMode(const MaterialSourceData &sourceData)
{
  return sourceData.alphaMode;
}
float MaterialTemplate::GetAlphaCutoff(const MaterialSourceData &sourceData)
{
  return sourceData.alphaCutoff;
}
bool MaterialTemplate::IsDoubleSided(const MaterialSourceData &sourceData)
{
  return sourceData.doubleSided;
}
};  // namespace mite