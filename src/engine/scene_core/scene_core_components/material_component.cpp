#include "material_component.h"
#include "mesh_component.h"

namespace mite {
MaterialComponent::MaterialComponent(std::shared_ptr<MaterialInstance> material)
    : ComponentTraits(), m_Material(material)
{
}

// =================== 材质基础操作 =====================
std::shared_ptr<MaterialInstance> MaterialComponent::GetMaterial() const
{
  return m_Material;
}

void MaterialComponent::SetMaterial(std::shared_ptr<MaterialInstance> material)
{
  if (m_Material != material) {
    m_Material = material;
    EventBus::Publish<MaterialChangedEvent>(MaterialChangedEvent(GetEntity(), *this));
  }
}

bool MaterialComponent::HasMaterial() const
{
  return m_Material != nullptr;
}

// =================== 着色器控制 =======================
std::shared_ptr<OpenGLShader> MaterialComponent::GetShader() const
{
  return m_Material ? m_Material->GetShader() : nullptr;
}

// ================== 材质参数控制 ======================

void MaterialComponent::SetFloatParam(const std::string &name, float value)
{
  if (!m_Material) {
    LOG_WARN("Attempt to set param on null material");
    return;
  }
  m_Material->SetFloat(name, value);
}

void MaterialComponent::SetColorParam(const std::string &name, const glm::vec3 &color)
{
  if (!m_Material)
    return;
  m_Material->SetVector3(name, color);
}

void MaterialComponent::SetTextureParam(const std::string &name, std::shared_ptr<Texture> texture)
{
  if (!m_Material)
    return;
  m_Material->SetTexture(name, std::move(texture));
}

// ================== 组件接口实现 ======================
std::vector<std::type_index> MaterialComponent::GetDependencies() const
{
  return {};
}

bool MaterialComponent::Serialize(std::ostream &output) const
{
  Component::Serialize(output);  // 序列化基类数据

  // TODO: 实现材质参数的序列化
  // 需要考虑纹理资源的引用序列化

  return !output.fail();
}

bool MaterialComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);  // 反序列化基类数据

  // TODO: 实现材质参数的反序列化

  return !input.fail();
}

// ================= Material组件系统实现 =================
std::vector<std::type_index> MaterialComponentSystem::GetSystemDependencies() const
{
  return {typeid(MeshComponentSystem)};  // 通常与Mesh配合使用
}

};  // namespace mite