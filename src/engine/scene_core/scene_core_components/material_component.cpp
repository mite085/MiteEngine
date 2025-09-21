#include "material_component.h"
#include "mesh_component.h"

namespace mite {
MaterialComponent::MaterialComponent(MaterialInstanceHandle handle) : m_Handle(handle) {}

// =================== 材质基础操作 =====================
MaterialInstanceHandle MaterialComponent::GetMaterialInstanceHandel() const
{
  return m_Handle;
}

void MaterialComponent::SetMaterialInstanceHandel(MaterialInstanceHandle handle)
{
    m_Handle = handle;
    EventBus::Publish<MaterialChangedEvent>(MaterialChangedEvent(GetEntity(), *this));
}

// =================== 着色器控制 =======================
std::shared_ptr<OpenGLShader> MaterialComponent::GetShader() const
{
  MaterialInstance* material = MaterialSystem::Get().GetInstance(m_Handle);
  return material ? material->GetShader() : nullptr;
}

// ================== 材质参数控制 ======================

void MaterialComponent::SetFloatParam(const std::string &name, float value)
{
  MaterialInstance *material = MaterialSystem::Get().GetInstance(m_Handle);
  if (!material) {
    LOG_WARN("Attempt to set param on null material");
    return;
  }
  material->SetFloat(name, value);
}

void MaterialComponent::SetColorParam(const std::string &name, const glm::vec3 &color)
{
  MaterialInstance *material = MaterialSystem::Get().GetInstance(m_Handle);
  if (!material)
    return;
  material->SetVector3(name, color);
}

void MaterialComponent::SetTextureParam(const std::string &name, TextureGPUHandle texture)
{
  MaterialInstance *material = MaterialSystem::Get().GetInstance(m_Handle);
  if (!material)
    return;
  material->SetTexture(name, texture);
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

MaterialInstanceHandle MaterialComponent::GetSnapshotData() const
{
  return m_Handle;
}

void MaterialComponent::SetSnapshotData(const MaterialInstanceHandle &data)
{
  m_Handle = data;
  // 发布更新事件
  EventBus::Publish<MaterialChangedEvent>(MaterialChangedEvent(GetEntity(), *this));
}

// ================= Material组件系统实现 =================
std::vector<std::type_index> MaterialComponentSystem::GetSystemDependencies() const
{
  return {typeid(MeshComponentSystem)};  // 通常与Mesh配合使用
}

};  // namespace mite