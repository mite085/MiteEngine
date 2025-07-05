#include "material_instance.h"

namespace mite {
MaterialInstance::MaterialInstance(std::shared_ptr<Shader> shader) : m_Shader(std::move(shader))
{
  if (!m_Shader) {
    LOG_ERROR("MaterialInstance created with null shader!");
  }
}

// ===================== 参数设置方法 =====================
void MaterialInstance::SetFloat(const std::string &name, float value)
{
  m_Uniforms[name] = UniformValue(value);
}

void MaterialInstance::SetInt(const std::string &name, int value)
{
  m_Uniforms[name] = UniformValue(value);
}

void MaterialInstance::SetVector2(const std::string &name, const glm::vec2 &value)
{
  m_Uniforms[name] = UniformValue(value);
}

void MaterialInstance::SetVector3(const std::string &name, const glm::vec3 &value)
{
  m_Uniforms[name] = UniformValue(value);
}

void MaterialInstance::SetVector4(const std::string &name, const glm::vec4 &value)
{
  m_Uniforms[name] = UniformValue(value);
}

void MaterialInstance::SetMatrix4(const std::string &name, const glm::mat4 &value)
{
  m_Uniforms[name] = UniformValue(value);
}

// ===================== 纹理设置方法 =====================
void MaterialInstance::SetTexture(const std::string &name, std::shared_ptr<Texture> texture)
{
  if (texture) {
    m_Textures[name] = std::move(texture);
  }
  else {
    LOG_WARN("Null texture assigned to slot: {}", name);
    m_Textures.erase(name);
  }
}

void MaterialInstance::SetTextureArray(const std::string &name,
                                       const std::vector<std::shared_ptr<Texture>> &textures)
{
  if (!textures.empty()) {
    m_TextureArrays[name] = textures;
  }
  else {
    LOG_WARN("Empty texture array assigned to slot: {}", name);
    m_TextureArrays.erase(name);
  }
}

// ===================== 核心Apply方法 =====================
void MaterialInstance::Apply(Shader *overrideShader) const
{
  Shader *targetShader = overrideShader ? overrideShader : m_Shader.get();
  if (!targetShader) {
    LOG_ERROR("MaterialInstance has no valid shader to apply!");
    return;
  }

  targetShader->Bind();

  // ---- 上传Uniform值 ----
  for (const auto &[name, value] : m_Uniforms) {
    switch (value.type) {
      case UniformValue::Type::Float:
        targetShader->SetFloat(name, value.asFloat);
        break;
      case UniformValue::Type::Int:
        targetShader->SetInt(name, value.asInt);
        break;
      case UniformValue::Type::Vector2:
        targetShader->SetVec2(name, value.asVec2);
        break;
      case UniformValue::Type::Vector3:
        targetShader->SetVec3(name, value.asVec3);
        break;
      case UniformValue::Type::Vector4:
        targetShader->SetVec4(name, value.asVec4);
        break;
      case UniformValue::Type::Matrix4:
        targetShader->SetMat4(name, value.asMat4);
        break;
      default:
        LOG_ERROR("Invalid OpenGL uniform item: {};", name);
        break;
    }
  }

  // ---- 绑定纹理 ----
  uint32_t textureSlot = 0;
  for (const auto &[name, texture] : m_Textures) {
    texture->Bind(textureSlot);
    targetShader->SetInt(name, static_cast<int>(textureSlot));
    textureSlot++;
  }

  // ---- 绑定纹理数组 ----
  for (const auto &[name, textures] : m_TextureArrays) {
    std::vector<int> slots;
    for (const auto &texture : textures) {
      texture->Bind(textureSlot);
      slots.push_back(static_cast<int>(textureSlot));
      textureSlot++;
    }
    targetShader->SetIntArray(name, slots.data(), static_cast<int>(slots.size()));
  }
}
};  // namespace mite