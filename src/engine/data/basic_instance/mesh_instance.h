#ifndef MITE_MESH_INSTANCE_H
#define MITE_MESH_INSTANCE_H

#include "basic_data/mesh.h"
#include "basic_data/transform.h"
#include "basic_shader/shader_ubo.h"

namespace mite {
/**
 * @brief 网格实例类，负责管理模型UBO的生命周期
 * @note 设计理念：
 * - 与相机、材质系统保持一致：Mesh -> MeshInstance
 * - 每个网格实例持有独立的模型UBO，支持实例化渲染
 * - 网格实例负责模型UBO的创建、更新和绑定
 */
class MeshInstance {
 public:
  /**
   * @brief 构造函数
   * @param mesh 关联的网格对象
   * @param name 网格实例名称
   */
  explicit MeshInstance(std::shared_ptr<Mesh> mesh);
  ~MeshInstance();

  // ==================== UBO管理接口 ====================
  /**
   * @brief 初始化模型UBO
   * @return 是否初始化成功
   */
  bool InitializeUBO();
  /**
   * @brief 设置着色器绑定
   * @param shader 着色器对象
   * @note Initialize之后发布事件，由管理Shader的RenderContext接手负责绑定即可
   * (使用固定的绑定点执行显示绑定，无需手动管理)
   */
  //void SetupShaderBinding(std::shared_ptr<OpenGLShader> shader);
  /**
   * @brief 更新模型UBO数据
   * @param worldTransform 世界变换矩阵
   * @return 是否更新成功
   */
  void UpdateUBO(const Transform &worldTransform);
  /**
   * @brief 绑定模型UBO到当前渲染状态
   */
  void BindUBO() const;

  // ==================== 网格访问接口 ====================
  std::shared_ptr<Mesh> GetMesh() const { return m_Mesh; }
  void SetMesh(std::shared_ptr<Mesh> mesh) { m_Mesh = mesh; }

  /**
   * @brief 获取当前世界变换
   */
  const Transform &GetWorldTransform() const { return m_WorldTransform; }

  /**
   * @brief 获取UBO对象
   */
  std::shared_ptr<ShaderUBO> GetUBO() const { return m_ModelUBO; }

  // ==================== LOD设定接口 ====================
  void SetMeshLODLevel(uint32_t lodLevel) { m_LODLevel = lodLevel; }
  uint32_t GetMeshLodLevel() { return m_LODLevel; }

  // ==================== 实例属性管理 ====================
  /**
   * @brief 获取材质索引（用于关联MaterialInstance）
   */
  uint32_t GetMaterialIndex() const;

  /**
   * @brief 获取包围盒（世界空间）
   */
  std::pair<glm::vec3, glm::vec3> GetWorldBoundingBox() const;

 private:
  std::shared_ptr<Mesh> m_Mesh;           // 关联的网格对象
  uint32_t m_LODLevel;                    // 网格的LOD级别
  Transform m_WorldTransform;             // 世界空间变换（缓存）
  std::shared_ptr<ShaderUBO> m_ModelUBO;  // 模型UBO实例

  // 禁用拷贝构造和赋值
  MeshInstance(const MeshInstance &) = delete;
  MeshInstance &operator=(const MeshInstance &) = delete;
};
}  // namespace mite

#endif  // MITE_MESH_INSTANCE_H
