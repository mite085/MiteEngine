#include "mesh_component.h"

#include "material_component.h"
#include "transform_component.h"
namespace mite {
MeshComponent::MeshComponent() {}

// 网格操作 ==============================================
std::shared_ptr<Mesh> MeshComponent::GetMesh() const { return m_Mesh; }

void MeshComponent::SetMesh(std::shared_ptr<Mesh> mesh) {
  m_Mesh = mesh;
  EventBus::Publish<MeshChangedEvent>(GetEntity(), *this);
}

// 组件接口实现 ==========================================
std::vector<std::type_index> MeshComponent::GetDependencies() const {
  return {typeid(TransformComponent)};
}

bool MeshComponent::Serialize(std::ostream &output) const {
  Component::Serialize(output);  // 序列化基类数据

  // TODO: 实现网格和材质的序列化
  // 需要考虑资源引用如何序列化

  return !output.fail();
}

bool MeshComponent::Deserialize(std::istream &input) {
  Component::Deserialize(input);  // 反序列化基类数据

  // TODO: 实现网格和材质的反序列化

  return !input.fail();
}

const Mesh &MeshComponent::GetSnapshotData() const { return *m_Mesh; }

void MeshComponent::SetSnapshotData(const Mesh &data) {
  *m_Mesh = data;
  // 发布更新事件
  EventBus::Publish<MeshChangedEvent>(GetEntity(), *this);
}

// Mesh组件系统实现 ======================================
std::vector<std::type_index> MeshComponentSystem::GetSystemDependencies()
    const {
  return {typeid(TransformComponentSystem)};  // 需要变换信息
}
};  // namespace mite