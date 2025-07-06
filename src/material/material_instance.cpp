#include "material_instance.h"

namespace mite {
UniformValue::UniformValue(const std::pair<int *, size_t> &v) : type(Type::IntArray)
{
  asIntArray.second = v.second;
  asIntArray.first = new int[v.second];
  std::memcpy(asIntArray.first, v.first, v.second * sizeof(int));
}

UniformValue::UniformValue(const std::pair<float *, size_t> &v) : type(Type::FloatArray)
{
  asFloatArray.second = v.second;
  asFloatArray.first = new float[v.second];
  std::memcpy(asFloatArray.first, v.first, v.second * sizeof(float));
}

UniformValue::UniformValue(const std::pair<glm::vec3 *, size_t> &v) : type(Type::Vector3Array)
{
  asVec3Array.second = v.second;
  asVec3Array.first = new glm::vec3[v.second];
  std::memcpy(asVec3Array.first, v.first, v.second * sizeof(glm::vec3));
}

UniformValue::~UniformValue()
{
  if (type == Type::IntArray)
    delete[] asIntArray.first;
  else if (type == Type::FloatArray)
    delete[] asFloatArray.first;
  else if (type == Type::Vector3Array)
    delete[] asVec3Array.first;
}

UniformValue::UniformValue(UniformValue &&other) noexcept
{
  *this = std::move(other);
}

UniformValue &UniformValue::operator=(UniformValue &&other) noexcept
{
  if (this != &other) {
    this->~UniformValue();  // 清理现有资源

    type = other.type;
    switch (type) {
      case Type::IntArray:
        asIntArray = other.asIntArray;
        break;
      case Type::FloatArray:
        asFloatArray = other.asFloatArray;
        break;
      case Type::Vector3Array:
        asVec3Array = other.asVec3Array;
        break;
      default:
        std::memcpy(this, &other, sizeof(UniformValue));
        break;
    }
    other.type = Type::None;  // 置空原对象
  }
  return *this;
}

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

void MaterialInstance::SetMatrix3(const std::string &name, const glm::mat3 &matrix)
{
  m_Uniforms[name] = UniformValue(matrix);
}

void MaterialInstance::SetMatrix4(const std::string &name, const glm::mat4 &value)
{
  m_Uniforms[name] = UniformValue(value);
}

// ===================== 参数数组设置方法 =====================

void MaterialInstance::SetIntArray(const std::string &name, const int *values, size_t count)
{
  m_Uniforms[name] = UniformValue(std::make_pair(const_cast<int *>(values), count));
}

void MaterialInstance::SetFloatArray(const std::string &name, const float *values, size_t count)
{
  m_Uniforms[name] = UniformValue(std::make_pair(const_cast<float *>(values), count));
}

void MaterialInstance::SetVector3Array(const std::string &name,
                                       const glm::vec3 *values,
                                       size_t count)
{
  m_Uniforms[name] = UniformValue(std::make_pair(const_cast<glm::vec3 *>(values), count));
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
      case UniformValue::Type::Matrix3:
        targetShader->SetMat3(name, value.asMat3);
        break;
      case UniformValue::Type::Matrix4:
        targetShader->SetMat4(name, value.asMat4);
        break;
      case UniformValue::Type::IntArray:
        targetShader->SetIntArray(name, value.asIntArray.first, value.asIntArray.second);
        break;
      case UniformValue::Type::FloatArray:
        targetShader->SetFloatArray(name, value.asFloatArray.first, value.asFloatArray.second);
        break;
      case UniformValue::Type::Vector3Array:
        targetShader->SetVector3Array(name, value.asVec3Array.first, value.asVec3Array.second);
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