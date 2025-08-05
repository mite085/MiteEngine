#include "material_instance.h"

namespace mite {
MaterialInstance::MaterialInstance(std::shared_ptr<OpenGLShader> shader) : m_Shader(std::move(shader))
{
  if (!m_Shader) {
    LOG_ERROR("MaterialInstance created with null shader!");
  }
}

// ===================== 参数设置方法 =====================
void MaterialInstance::SetFloat(const std::string &name, float value)
{
  m_Uniforms[name] = value;
}

void MaterialInstance::SetInt(const std::string &name, int value)
{
  m_Uniforms[name] = value;
}

void MaterialInstance::SetVector2(const std::string &name, const glm::vec2 &value)
{
  m_Uniforms[name] = value;
}

void MaterialInstance::SetVector3(const std::string &name, const glm::vec3 &value)
{
  m_Uniforms[name] = value;
}

void MaterialInstance::SetVector4(const std::string &name, const glm::vec4 &value)
{
  m_Uniforms[name] = value;
}

void MaterialInstance::SetMatrix3(const std::string &name, const glm::mat3 &value)
{
  m_Uniforms[name] = value;
}

void MaterialInstance::SetMatrix4(const std::string &name, const glm::mat4 &value)
{
  m_Uniforms[name] = value;
}

// ===================== 参数数组设置方法 =====================

void MaterialInstance::SetIntArray(const std::string &name, const int *values, size_t count)
{
  m_Uniforms[name] = std::vector<int>(values, values + count);
}

void MaterialInstance::SetFloatArray(const std::string &name, const float *values, size_t count)
{
  m_Uniforms[name] = std::vector<float>(values, values + count);
}

void MaterialInstance::SetVector3Array(const std::string &name,
                                       const glm::vec3 *values,
                                       size_t count)
{
  m_Uniforms[name] = std::vector<glm::vec3>(values, values + count);
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
void MaterialInstance::Apply(TextureBindFunc textureBindFunc, OpenGLShader *overrideShader) const
{
  OpenGLShader *targetShader = overrideShader ? overrideShader : m_Shader.get();
  if (!targetShader) {
    LOG_ERROR("MaterialInstance has no valid shader to apply!");
    return;
  }

  targetShader->Bind();

  // ---- 上传Uniform值（不包含纹理，纹理单独处理） ----
  for (const auto &[name, value] : m_Uniforms) {
    switch (value.GetType()) {
      case UniformVariant::Type::Float:
        targetShader->SetFloat(name, value.Get<float>());
        break;
      case UniformVariant::Type::Int:
        targetShader->SetInt(name, value.Get<int>());
        break;
      case UniformVariant::Type::Vector2:
        targetShader->SetVec2(name, value.Get<glm::vec2>());
        break;
      case UniformVariant::Type::Vector3:
        targetShader->SetVec3(name, value.Get<glm::vec3>());
        break;
      case UniformVariant::Type::Vector4:
        targetShader->SetVec4(name, value.Get<glm::vec4>());
        break;
      case UniformVariant::Type::Matrix3:
        targetShader->SetMat3(name, value.Get<glm::mat3>());
        break;
      case UniformVariant::Type::Matrix4:
        targetShader->SetMat4(name, value.Get<glm::mat4>());
        break;
      case UniformVariant::Type::IntArray: {
        auto [ptr, count] = value.GetArray<int>();
        targetShader->SetIntArray(name, ptr, count);
        break;
      }
      case UniformVariant::Type::FloatArray: {
        auto [ptr, count] = value.GetArray<float>();
        targetShader->SetFloatArray(name, ptr, count);
        break;
      }
      case UniformVariant::Type::Vector3Array: {
        auto [ptr, count] = value.GetArray<glm::vec3>();
        targetShader->SetVector3Array(name, ptr, count);
        break;
      }
      default:
        LOG_ERROR("Invalid OpenGL uniform item: {};", name);
        break;
    }
  }

  // ---- 绑定纹理（纹理单独处理部分） ----
  uint32_t textureSlot = 0;
  for (const auto &[name, texture] : m_Textures) {
    // 使用传入的纹理绑定函数进行纹理绑定
    textureBindFunc(texture->GetHandle(), textureSlot);
    targetShader->SetInt(name, static_cast<int>(textureSlot));
    textureSlot++;
  }

  // ---- 绑定纹理数组 ----
  for (const auto &[name, textures] : m_TextureArrays) {
    std::vector<int> slots;
    for (const auto &texture : textures) {
      // 使用传入的纹理绑定函数进行纹理绑定
      textureBindFunc(texture->GetHandle(), textureSlot);
      slots.push_back(static_cast<int>(textureSlot));
      textureSlot++;
    }
    targetShader->SetIntArray(name, slots.data(), static_cast<int>(slots.size()));
  }
}
};  // namespace mite