#include "material_component.h"

namespace mite {
MaterialComponent::MaterialComponent() : ComponentTraits() {}

MaterialComponent::MaterialComponent(std::shared_ptr<Material> material)
    : ComponentTraits(), m_Material(material ? material : std::make_shared<Material>())
{
}

// 材质基础操作 ========================================
std::shared_ptr<Material> MaterialComponent::GetMaterial() const
{
  return m_Material;
}

void MaterialComponent::SetMaterial(std::shared_ptr<Material> material)
{
  if (m_Material != material) {
    m_Material = material ? material : std::make_shared<Material>();
    EventBus::Get().Post(MaterialChangedEvent(GetOwnerEntity(), *this));
  }
}

bool MaterialComponent::HasMaterial() const
{
  return m_Material != nullptr;
}

// 着色器控制 ==========================================
std::shared_ptr<Shader> MaterialComponent::GetShader() const
{
  return m_Material ? m_Material->GetShader() : nullptr;
}

void MaterialComponent::SetShader(std::shared_ptr<Shader> shader)
{
  if (m_Material && m_Material->GetShader() != shader) {
    m_Material->SetShader(shader);
    EventBus::Get().Post(ShaderChangedEvent(GetOwnerEntity(), *this));
  }
}

// 材质参数控制 ========================================
void MaterialComponent::SetBaseColor(const glm::vec4 &color)
{
  if (m_Material) {
    m_Material->SetBaseColor(color);
  }
}

glm::vec4 MaterialComponent::GetBaseColor() const
{
  return m_Material ? m_Material->GetBaseColor() : glm::vec4(1.0f);
}

void MaterialComponent::SetMetallic(float metallic)
{
  if (m_Material) {
    m_Material->SetMetallic(metallic);
  }
}

float MaterialComponent::GetMetallic() const
{
  return m_Material ? m_Material->GetMetallic() : 0.0f;
}

void MaterialComponent::SetRoughness(float roughness)
{
  if (m_Material) {
    m_Material->SetRoughness(roughness);
  }
}

float MaterialComponent::GetRoughness() const
{
  return m_Material ? m_Material->GetRoughness() : 0.5f;
}

void MaterialComponent::SetEmissive(const glm::vec3 &emissive)
{
  if (m_Material) {
    m_Material->SetEmissive(emissive);
  }
}

glm::vec3 MaterialComponent::GetEmissive() const
{
  return m_Material ? m_Material->GetEmissive() : glm::vec3(0.0f);
}

// 纹理控制 ============================================
void MaterialComponent::SetBaseColorTexture(std::shared_ptr<Texture> texture)
{
  if (m_Material) {
    m_Material->SetBaseColorTexture(texture);
  }
}

void MaterialComponent::SetNormalTexture(std::shared_ptr<Texture> texture)
{
  if (m_Material) {
    m_Material->SetNormalTexture(texture);
  }
}

void MaterialComponent::SetMetallicRoughnessTexture(std::shared_ptr<Texture> texture)
{
  if (m_Material) {
    m_Material->SetMetallicRoughnessTexture(texture);
  }
}

// 渲染状态控制 ========================================
void MaterialComponent::SetBlendMode(BlendMode blendMode)
{
  if (m_Material) {
    m_Material->SetBlendMode(blendMode);
  }
}

BlendMode MaterialComponent::GetBlendMode() const
{
  return m_Material ? m_Material->GetBlendMode() : BlendMode::Opaque;
}

void MaterialComponent::SetDoubleSided(bool doubleSided)
{
  if (m_Material) {
    m_Material->SetDoubleSided(doubleSided);
  }
}

bool MaterialComponent::IsDoubleSided() const
{
  return m_Material ? m_Material->IsDoubleSided() : false;
}

// 组件接口实现 ========================================
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

// Material组件系统实现 ==================================
void MaterialSystem::Initialize(SceneRegistry &registry)
{
  DirtyComponentSystem<MaterialComponent>::Initialize(registry);
  // 初始化材质系统资源
}

void MaterialSystem::Shutdown(SceneRegistry &registry)
{
  DirtyComponentSystem<MaterialComponent>::Shutdown(registry);
  // 清理材质系统资源
}

void MaterialSystem::Update(float deltaTime, SceneRegistry &registry)
{
  // 处理材质参数动画等每帧更新
  auto view = registry.GetEntitiesWith<MaterialComponent>();

  for (auto entity : view) {
    auto &material = registry.GetComponent<MaterialComponent>(entity);
    // 可以在这里处理材质动画等逻辑
  }
}
};
