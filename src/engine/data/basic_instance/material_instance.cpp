#include "material_instance.h"

namespace mite {
MaterialInstance::MaterialInstance(std::shared_ptr<OpenGLShader> shader)
    : m_Shader(std::move(shader))
{
  if (!m_Shader) {
    LOG_ERROR("MaterialInstance created with null shader!");
  }
}

MaterialInstance::~MaterialInstance() {}

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
void MaterialInstance::SetTexture(const std::string &name, TextureGPUSlot texture)
{
  m_Textures[name] = texture;
}

// void MaterialInstance::SetTextureArray(const std::string &name,
//                                        const std::vector<TextureGPUHandle> &textures)
//{
//   if (!textures.empty()) {
//     m_TextureArrays[name] = textures;
//   }
//   else {
//     LOG_WARN("Empty texture array assigned to slot: {}", name);
//   }
// }

// ===================== UBO绑定方法 =====================
void MaterialInstance::SetupUBO(const std::string &uniformBlockName,
                               std::shared_ptr<ShaderUBO> ubo,
                               uint32_t bindingPoint)
{
  if (!ubo) {
    LOG_ERROR("Cannot bind null UBO to material instance");
    return;
  }
  if (!ubo->IsInitialized()) {
    LOG_ERROR("Cannot bind uninitialized UBO to material instance");
    return;
  }
  // 设定绑定点
  ubo->SetupShaderBinding(m_Shader, uniformBlockName, bindingPoint);

  // 存储绑定信息
  m_UBOBindings[uniformBlockName] = {ubo, bindingPoint};

  LOG_DEBUG("UBO bound to material instance - Block: '{}', Binding Point: {}",
            uniformBlockName,
            bindingPoint);
}
void MaterialInstance::UninstallUBO(const std::string &uniformBlockName)
{
  auto it = m_UBOBindings.find(uniformBlockName);
  if (it != m_UBOBindings.end()) {
    m_UBOBindings.erase(it);
    LOG_DEBUG("UBO unbound from material instance - Block: '{}'", uniformBlockName);
  }
  else {
    LOG_WARN("Attempted to unbind UBO that was not bound: '{}'", uniformBlockName);
  }
}
bool MaterialInstance::HasUBO(const std::string &uniformBlockName) const
{
  return m_UBOBindings.find(uniformBlockName) != m_UBOBindings.end();
}
std::shared_ptr<ShaderUBO> MaterialInstance::GetUBO(const std::string &uniformBlockName) const
{
  auto it = m_UBOBindings.find(uniformBlockName);
  if (it != m_UBOBindings.end()) {
    return it->second.ubo;
  }
  return nullptr;
}

// ===================== SSBO绑定方法（新增）=====================
void MaterialInstance::SetupSSBO(const std::string &storageBlockName,
                                std::shared_ptr<ShaderSSBO> ssbo,
                                uint32_t bindingPoint)
{
  if (!ssbo) {
    LOG_ERROR("Cannot bind null SSBO to material instance");
    return;
  }
  if (!ssbo->IsInitialized()) {
    LOG_ERROR("Cannot bind uninitialized SSBO to material instance");
    return;
  }
  if (ssbo->IsMapped()) {
    LOG_ERROR("Cannot bind mapped SSBO to material instance");
    return;
  }
  // 设置着色器绑定
  ssbo->SetupShaderBinding(m_Shader, storageBlockName, bindingPoint);

  // 存储绑定信息
  m_SSBOBindings[storageBlockName] = {ssbo, bindingPoint};

  LOG_DEBUG("SSBO bound to material instance - Block: '{}', Binding Point: {}",
            storageBlockName,
            bindingPoint);
}
void MaterialInstance::UninstallSSBO(const std::string &storageBlockName)
{
  auto it = m_SSBOBindings.find(storageBlockName);
  if (it != m_SSBOBindings.end()) {
    m_SSBOBindings.erase(it);
    LOG_DEBUG("SSBO unbound from material instance - Block: '{}'", storageBlockName);
  }
  else {
    LOG_WARN("Attempted to unbind SSBO that was not bound: '{}'", storageBlockName);
  }
}
bool MaterialInstance::HasSSBO(const std::string &storageBlockName) const
{
  return m_SSBOBindings.find(storageBlockName) != m_SSBOBindings.end();
}

// ===================== 重构的原子绑定操作 =====================
void MaterialInstance::BindShaderOnly(OpenGLShader *overrideShader) const
{
  OpenGLShader *targetShader = overrideShader ? overrideShader : m_Shader.get();
  if (!targetShader) {
    LOG_ERROR("MaterialInstance has no valid shader to bind!");
    return;
  }
  targetShader->Bind();
}
void MaterialInstance::UploadUniformsOnly(OpenGLShader *overrideShader) const
{
  OpenGLShader *targetShader = overrideShader ? overrideShader : m_Shader.get();
  if (!targetShader) {
    LOG_ERROR("MaterialInstance has no valid shader for uniform upload!");
    return;
  }
  // 上传uniform值，不包含纹理
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
}
size_t MaterialInstance::BindTexturesOnly(TextureBindFunc textureBindFunc,
                                          size_t startSlot,
                                          OpenGLShader *overrideShader) const
{
  OpenGLShader *targetShader = overrideShader ? overrideShader : m_Shader.get();
  if (!targetShader) {
    LOG_ERROR("MaterialInstance has no valid shader for texture binding!");
    return 0;
  }
  // ---- 绑定纹理（纹理单独处理部分） ----
  size_t currentSlot = startSlot;
  // 注意：每个材质实例的Apply都是从 GL_TEXTURE0 + slot = 0
  // 开始计数的，若需要多材质渲染，需要不同材质实例共享textureSlot
  for (const auto &[name, texture] : m_Textures) {
    // 使用传入的纹理绑定函数进行纹理绑定
    textureBindFunc(texture.gpuHandle, currentSlot);
    targetShader->SetInt(name, static_cast<int>(currentSlot));

    // TODO: 需要将Solt的offset和scale传入Shader
    currentSlot++;
  }

  return m_Textures.size();  // 返回使用的纹理槽位数量
}
void MaterialInstance::BindBuffersOnly() const
{
  // ---- 绑定UBO（在绑定Shader后立即执行）----
  for (const auto &[blockName, uboBinding] : m_UBOBindings) {
    if (uboBinding.ubo && uboBinding.ubo->IsInitialized()) {
      uboBinding.ubo->Bind(uboBinding.bindingPoint);
    }
  }

  // ---- 绑定SSBO（在UBO绑定后执行）----
  for (const auto &[blockName, ssboBinding] : m_SSBOBindings) {
    if (ssboBinding.ssbo && ssboBinding.ssbo->IsInitialized() && !ssboBinding.ssbo->IsMapped()) {
      ssboBinding.ssbo->Bind(ssboBinding.bindingPoint);
    }
  }
}

void MaterialInstance::Apply(TextureBindFunc textureBindFunc,
                             size_t startSlot,
                             OpenGLShader *overrideShader) const
{
  BindShaderOnly(overrideShader);
  BindBuffersOnly();
  UploadUniformsOnly(overrideShader);
  BindTexturesOnly(textureBindFunc, startSlot, overrideShader);
}



std::shared_ptr<OpenGLShader> MaterialInstance::GetShader() const
{
  if (m_Shader)
    return m_Shader;
  else {
    LOG_ERROR("Invaid Shader");
    return nullptr;
  }
}
std::string MaterialInstance::GetName() const
{
  return m_Name;
}
void MaterialInstance::SetName(const std::string &name)
{
  m_Name = name;
}
};  // namespace mite