#include "material_component.h"

namespace mite {
MaterialComponent::MaterialComponent(std::shared_ptr<MaterialInstance> material)
    : ComponentTraits(), m_Material(std::move(material))
{
}

// 材质基础操作 ========================================
std::shared_ptr<MaterialInstance> MaterialComponent::GetMaterial() const
{
  return m_Material;
}

void MaterialComponent::SetMaterial(std::shared_ptr<MaterialInstance> material)
{
  if (m_Material != material) {
    m_Material = material;
    EventBus::Get().Post(MaterialChangedEvent(GetOwnerEntity(), *this));
    MarkDirty();
  }
}

void MaterialComponent::SetMaterialFromTemplate(const std::string &templateName)
{
  try {
    auto newMaterial = MaterialSystem::Get().CreateInstance(templateName);
    SetMaterial(newMaterial);
  }
  catch (const std::exception &e) {
    LOG_ERROR("Failed to create material from template '{}': {}", templateName, e.what());
    throw;
  }
}

bool MaterialComponent::HasMaterial() const
{
  return m_Material != nullptr;
}

// 着色器控制 ==========================================
std::shared_ptr<OpenGLShader> MaterialComponent::GetShader() const
{
  return m_Material ? m_Material->GetShader() : nullptr;
}

// 材质参数控制 ========================================

void MaterialComponent::SetFloatParam(const std::string &name, float value)
{
  if (!m_Material) {
    LOG_WARN("Attempt to set param on null material");
    return;
  }
  m_Material->SetFloat(name, value);
  MarkDirty();
}

void MaterialComponent::SetColorParam(const std::string &name, const glm::vec3 &color)
{
  if (!m_Material)
    return;
  m_Material->SetVector3(name, color);
  MarkDirty();
}

void MaterialComponent::SetTextureParam(const std::string &name, std::shared_ptr<Texture> texture)
{
  if (!m_Material)
    return;
  m_Material->SetTexture(name, std::move(texture));
  MarkDirty();
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
void MaterialComponentSystem::Initialize(SceneRegistry &registry)
{
  DirtyComponentSystem<MaterialComponent>::Initialize(registry);
  // 初始化材质系统资源
}

void MaterialComponentSystem::Shutdown(SceneRegistry &registry)
{
  DirtyComponentSystem<MaterialComponent>::Shutdown(registry);
  // 清理材质系统资源
}

void MaterialComponentSystem::Update(float deltaTime, SceneRegistry &registry)
{
  // 处理材质参数动画等每帧更新
  auto view = registry.GetEntitiesWith<MaterialComponent>();

  // 按材质分组以减少状态切换
  std::unordered_map<MaterialInstance *, std::vector<Entity>> materialGroups;
  for (auto entity : view) {
    auto &matComp = registry.GetComponent<MaterialComponent>(entity);
    if (matComp.HasMaterial()) {
      materialGroups[matComp.GetMaterial().get()].push_back(entity);
    }
  }

  // 批量提交到渲染器
  for (const auto &[material, entities] : materialGroups) {
    // 绑定材质状态
    //material->Apply();

    // 提交关联实体
    for (Entity entity : entities) {
      // 示例：基于实体位置，修改u_Model材质参数，（可用于实现不同海拔高度下不同色彩表现）
      // if (registry.HasComponent<TransformComponent>(entity)) {
      //  const auto &transform = registry.GetComponent<TransformComponent>(entity);
      //  material->GetShader()->SetMat4("u_Model", transform.GetWorldMatrix(registry));
      //}
    }
  }
}

};  // namespace mite
