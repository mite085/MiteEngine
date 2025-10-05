#include "material_component.h"
#include "mesh_component.h"

namespace mite {
MaterialComponent::MaterialComponent(std::shared_ptr<MaterialInstance> handle) : m_MaterialInstance(handle)
{
}

void MaterialComponent::Update(float deltaTime, SceneRegistry &registry)
{
  // TODO: 此处应当为Dirty式更新。等后续制作材质属性页时处理，每次编辑m_MaterialInstance->m_UBOBinding.uboData时执行。
  m_MaterialInstance->UpdateUBO();
}

// =================== 材质基础操作 =====================
std::shared_ptr<MaterialInstance> MaterialComponent::GetMaterialInstanceHandel() const
{
  return m_MaterialInstance;
}

void MaterialComponent::SetMaterialInstanceHandel(std::shared_ptr<MaterialInstance> handle)
{
    m_MaterialInstance = handle;
    EventBus::Publish<MaterialChangedEvent>(MaterialChangedEvent(GetEntity(), *this));
}

// =================== 着色器控制 =======================
std::shared_ptr<OpenGLShader> MaterialComponent::GetShader() const
{
  return m_MaterialInstance ? m_MaterialInstance->GetShader() : nullptr;
}

// ================== 材质参数控制 ======================

void MaterialComponent::SetFloatParam(const std::string &name, float value)
{
  if (!m_MaterialInstance) {
    LOG_WARN("Attempt to set param on null material");
    return;
  }
  m_MaterialInstance->SetFloat(name, value);
}

void MaterialComponent::SetColorParam(const std::string &name, const glm::vec3 &color)
{
  if (!m_MaterialInstance)
    return;
  m_MaterialInstance->SetVector3(name, color);
}

void MaterialComponent::SetTextureParam(const std::string &name, TextureGPUSlot texture)
{
  if (!m_MaterialInstance)
    return;
  m_MaterialInstance->SetTexture(name, texture);
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

std::shared_ptr<MaterialInstance> MaterialComponent::GetSnapshotData() const
{
  return m_MaterialInstance;
}

void MaterialComponent::SetSnapshotData(const std::shared_ptr<MaterialInstance> &data)
{
  m_MaterialInstance = data;
  // 发布更新事件
  EventBus::Publish<MaterialChangedEvent>(MaterialChangedEvent(GetEntity(), *this));
}

void MaterialComponentSystem::Update(float deltaTime, SceneRegistry &registry) {
  for (auto &component: m_AllComponents) {
	component.second->Update(deltaTime, registry);
  }
}

// ================= Material组件系统实现 =================
std::vector<std::type_index> MaterialComponentSystem::GetSystemDependencies() const
{
  return {typeid(MeshComponentSystem)};  // 通常与Mesh配合使用
}

};  // namespace mite