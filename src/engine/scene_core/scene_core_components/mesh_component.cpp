#include "mesh_component.h"
#include "material_component.h"
#include "transform_component.h"
namespace mite {
MeshComponent::MeshComponent(std::shared_ptr<Mesh> mesh) : ComponentTraits(), m_Mesh(mesh)
{
}

// 网格操作 ==============================================
std::shared_ptr<Mesh> MeshComponent::GetMesh() const
{
  return m_Mesh;
}

void MeshComponent::SetMesh(std::shared_ptr<Mesh> mesh)
{
  m_Mesh = mesh;
  EventBus::Get().Post(MeshChangedEvent(GetOwnerEntity(), *this));
}

bool MeshComponent::HasMesh() const
{
  return m_Mesh && m_Mesh->GetSection().indexCount != 0 && m_Mesh->GetSection().vertexCount != 0;
}

// 渲染属性控制 ==========================================

void MeshComponent::SetCastShadows(bool castShadows)
{
  m_CastShadows = castShadows;
}

bool MeshComponent::CastsShadows() const
{
  return m_CastShadows;
}

void MeshComponent::SetReceiveShadows(bool receiveShadows)
{
  m_ReceiveShadows = receiveShadows;
}

bool MeshComponent::ReceivesShadows() const
{
  return m_ReceiveShadows;
}

// LOD控制 ==============================================
void MeshComponent::SetLODLevel(int lodLevel)
{
  m_LODLevel = std::max(0, lodLevel);
}

int MeshComponent::GetLODLevel() const
{
  return m_LODLevel;
}

// 组件接口实现 ==========================================
std::vector<std::type_index> MeshComponent::GetDependencies() const
{
  return {typeid(TransformComponent)};
}

bool MeshComponent::Serialize(std::ostream &output) const
{
  Component::Serialize(output);  // 序列化基类数据

  // TODO: 实现网格和材质的序列化
  // 需要考虑资源引用如何序列化

  return !output.fail();
}

bool MeshComponent::Deserialize(std::istream &input)
{
  Component::Deserialize(input);  // 反序列化基类数据

  // TODO: 实现网格和材质的反序列化

  return !input.fail();
}

// Mesh组件系统实现 ======================================
std::vector<std::type_index> MeshComponentSystem::GetSystemDependencies() const
{
  return {typeid(TransformComponentSystem)};  // 需要变换信息
}

void MeshComponentSystem::Initialize()
{
  DirtyComponentSystem<MeshComponent>::Initialize();
  // 初始化系统资源
}

void MeshComponentSystem::Shutdown()
{
  DirtyComponentSystem<MeshComponent>::Shutdown();
  // 清理系统资源
}

void MeshComponentSystem::Update(float deltaTime, SceneRegistry &registry)
{
  std::vector<Entity> view = registry.GetEntitiesWithAllOf<MeshComponent, TransformComponent>();

  for (Entity entity : view) {
    auto &mesh = registry.GetComponent<MeshComponent>(entity);
    auto &transform = registry.GetComponent<TransformComponent>(entity);

    // TODO: 处理每帧更新，如LOD计算等
  }
}
};  // namespace mite