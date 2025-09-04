#ifndef MITE_SCENE_MESH_COMPONENT
#define MITE_SCENE_MESH_COMPONENT

#include "scene_core/component_system.h"
#include "basic_data/mesh.h"

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
class MeshComponent : public ComponentTraits<MeshComponent, Component::Family::Geometry> {
 public:

  /**
   * @brief 带初始值的构造函数
   * @param mesh 网格数据
   * @param material 材质数据
   */
  explicit MeshComponent(std::shared_ptr<Mesh> mesh);

  ~MeshComponent() override = default;

  /**
   * @brief 针对dirty对象进行处理
   */
  void ProcessDirty(float deltaTime, SceneRegistry &reg) override {}

  // 网格操作 ==============================================
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

  /**
   * @brief 检查是否有有效网格数据
   * @return 是否有效
   */
  bool HasMesh() const;

  /**
   * @brief 获取Mesh的包围盒
   * @return 
   */
  const std::pair<glm::vec3, glm::vec3> GetBoundingBox() const
  {
    return m_Mesh->GetBoundingBox();
  }

  // 渲染属性控制 ==========================================

  /**
   * @brief 设置是否投射阴影
   * @param castShadows 阴影标志
   */
  void SetCastShadows(bool castShadows);

  /**
   * @brief 检查是否投射阴影
   * @return 阴影标志
   */
  bool CastsShadows() const;

  /**
   * @brief 设置是否接收阴影
   * @param receiveShadows 接收阴影标志
   */
  void SetReceiveShadows(bool receiveShadows);

  /**
   * @brief 检查是否接收阴影
   * @return 接收阴影标志
   */
  bool ReceivesShadows() const;

  // LOD控制 ==============================================
  /**
   * @brief 设置LOD级别
   * @param lodLevel LOD级别(0为最高细节)
   */
  void SetLODLevel(int lodLevel);

  /**
   * @brief 获取当前LOD级别
   * @return LOD级别
   */
  int GetLODLevel() const;

  // 组件接口实现 ==========================================
  std::vector<std::type_index> GetDependencies() const override;
  bool Serialize(std::ostream &output) const override;
  bool Deserialize(std::istream &input) override;

 private:
  std::shared_ptr<Mesh> m_Mesh;  // 网格数据

  bool m_IsVisible = true;       // 可见性标志
  bool m_CastShadows = true;     // 是否投射阴影
  bool m_ReceiveShadows = true;  // 是否接收阴影
  int m_LODLevel = 0;            // LOD级别
};

// Mesh组件系统 =====================================================
class MeshComponentSystem : public DirtyComponentSystem<MeshComponent> {
  DECLARE_COMPONENT_SYSTEM(MeshComponentSystem)
 public:
  std::vector<std::type_index> GetSystemDependencies() const override;

  void Initialize() override;
  void Shutdown() override;
  void Update(float deltaTime, SceneRegistry &registry) override;
};

// Mesh组件事件 =====================================================
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
  Event *Clone() const override
  {
    return new MeshChangedEvent(entity, component);
  }
};
};  // namespace mite

#endif
