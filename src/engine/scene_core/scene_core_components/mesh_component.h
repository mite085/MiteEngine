#ifndef MITE_SCENE_MESH_COMPONENT
#define MITE_SCENE_MESH_COMPONENT

#include "basic_data/mesh.h"
#include "scene_core/component_system.h"

namespace mite {
/**
 * @brief 网格组件，管理实体的网格渲染数据
 *
 * 功能特性：
 * 1. 管理网格数据引用
 * 2. 关联材质数据
 * 3. 支持LOD(细节层次)控制
 * 4. 提供可见性控制
 *
 * 设计考虑：
 * - 使用共享指针管理网格和材质资源
 * - 与RendererSystem协同工作
 * - 支持实例化渲染
 */
class MeshComponent : public SnapshotComponentTraits<Mesh, Component::Family::Geometry> {
 public:
  /**
   * @brief 带初始值的构造函数
   * @param mesh 网格数据
   * @param material 材质数据
   */
  explicit MeshComponent();

  ~MeshComponent() override = default;

  // ==================== 网格操作 ==========================
  /**
   * @brief 获取网格数据
   * @return 共享指针指向的网格数据
   */
  std::shared_ptr<Mesh> GetMesh() const;

  /**
   * @brief 设置网格数据
   * @param mesh 新的网格数据
   */
  void SetMesh(std::shared_ptr<Mesh> mesh);

  // =================== 组件接口实现 =======================
  std::vector<std::type_index> GetDependencies() const override;
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

 private:
  const Mesh &GetSnapshotData() const override;
  void SetSnapshotData(const Mesh &data) override;

  std::shared_ptr<Mesh> m_Mesh;  // 网格数据
};

// ========================= Mesh组件系统 ============================
class MeshComponentSystem : public SnapshotComponentSystem<MeshComponent> {
  DECLARE_COMPONENT_SYSTEM(MeshComponentSystem)
 public:
  std::vector<std::type_index> GetSystemDependencies() const override;
};

// ======================== Mesh组件事件 =============================
/**
 * @class MeshChangedEvent
 * @brief 网格改变事件
 */
class MeshChangedEvent : public ComponentEvent<MeshComponent> {
 public:
  MeshChangedEvent(Entity entity, MeshComponent &component)
      : ComponentEvent<MeshComponent>(entity, component)
  {
  }
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_RENDER)
  Event *Clone() const override { return new MeshChangedEvent(this->m_Entity, this->m_Component); }
};
};  // namespace mite

#endif
