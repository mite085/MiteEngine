#include "mesh_component.h"
#include "material_component.h"
#include "transform_component.h"
namespace mite {
MeshComponent::MeshComponent() : ComponentTraits(), m_Mesh(Mesh(MeshGPUHandle())) {}

MeshComponent::MeshComponent(Mesh mesh) : ComponentTraits(), m_Mesh(mesh) {}

// 网格操作 ==============================================
Mesh MeshComponent::GetMesh() const
{
  return m_Mesh;
}

void MeshComponent::SetMesh(Mesh mesh)
{
  m_Mesh = mesh;
  EventBus::Get().Post(MeshChangedEvent(GetOwnerEntity(), *this));
}

bool MeshComponent::HasMesh() const
{
  return m_Mesh.GetHandle().indexCount != 0 && m_Mesh.GetHandle().vertexCount != 0;
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
void MeshComponentSystem::Initialize(SceneRegistry &registry)
{
  DirtyComponentSystem<MeshComponent>::Initialize(registry);
  // 初始化系统资源
}

void MeshComponentSystem::Shutdown(SceneRegistry &registry)
{
  DirtyComponentSystem<MeshComponent>::Shutdown(registry);
  // 清理系统资源
}

void MeshComponentSystem::Update(float deltaTime, SceneRegistry &registry)
{
  auto view = registry.GetEntitiesWith<MeshComponent, TransformComponent>();

  for (auto entity : view) {
    auto &mesh = registry.GetComponent<MeshComponent>(entity);
    auto &transform = registry.GetComponent<TransformComponent>(entity);

    // TODO: 处理每帧更新，如LOD计算等
  }
}
};  // namespace mite