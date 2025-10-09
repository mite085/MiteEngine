#include "material_component.h"
#include "mesh_component.h"

namespace mite {
MaterialComponent::MaterialComponent(){}

void MaterialComponent::Update(float deltaTime, SceneRegistry &registry)
{
  // TODO: 此处应当为Dirty式更新。等后续制作材质属性页时处理，每次编辑m_MaterialInstance->m_UBOBinding.uboData时执行。
  m_MaterialInstance->UpdateUBO();
}

// =================== 材质基础操作 =====================
std::shared_ptr<MaterialInstance> MaterialComponent::GetMaterialInstance() const
{
  return m_MaterialInstance;
}

void MaterialComponent::SetMaterialInstance(std::shared_ptr<MaterialInstance> handle)
{
    m_MaterialInstance = handle;
    EventBus::Publish<MaterialChangedEvent>(MaterialChangedEvent(GetEntity(), *this));
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