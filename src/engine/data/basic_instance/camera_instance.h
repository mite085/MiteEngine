#ifndef MITE_CAMERA_INSTANCE_H
#define MITE_CAMERA_INSTANCE_H

#include "basic_data/camera.h"
#include "basic_shader/shader_ubo.h"

namespace mite {
/**
 * @brief 相机实例类，负责管理相机UBO的生命周期
 * @note 设计理念：
 * - 与材质系统保持一致：CameraTemplate -> CameraInstance
 * - 每个相机实例持有独立的UBO，支持多相机渲染
 * - 相机实例负责UBO的创建、更新和绑定
 */
class CameraInstance {
 public:
  /**
   * @brief 构造函数
   * @param camera 关联的相机对象
   * @param name 相机实例名称
   */
  explicit CameraInstance(std::shared_ptr<Camera> camera, const std::string &name = "");
  ~CameraInstance();

  // ==================== UBO管理接口 ====================
  /**
   * @brief 初始化相机UBO
   * @param shader 需要绑定UBO的着色器程序
   * @return 是否初始化成功
   */
  bool InitializeUBO();

  /**
   * @brief 更新相机UBO数据
   * @param viewMatrix 视图矩阵
   * @return 是否更新成功
   */
  bool UpdateUBO(const glm::mat4 &viewMatrix);

  /**
   * @brief 绑定相机UBO到当前渲染状态
   */
  void BindUBO() const;

  /**
   * @brief 检查UBO是否已初始化
   */
  bool IsUBOInitialized() const
  {
    return m_UBOInitialized;
  }

  // ==================== 相机访问接口 ====================
  /**
   * @brief 获取关联的相机对象
   */
  std::shared_ptr<Camera> GetCamera() const
  {
    return m_Camera;
  }
  void SetCamera(std::shared_ptr<Camera> camera)
  {
    m_Camera = camera;
  }
  /**
   * @brief 获取相机投影矩阵
   */
  const glm::mat4 &GetProjectionMatrix() const
  {
    return m_Camera->GetProjectionMatrix();
  }

  /**
   * @brief 获取相机绑定点
   */
  uint32_t GetBindingPoint() const
  {
    return m_BindingPoint;
  }

  /**
   * @brief 获取UBO对象（用于外部管理）
   */
  std::shared_ptr<ShaderUBO> GetUBO() const
  {
    return m_CameraUBO;
  }

  // ==================== 实例属性管理 ====================
  std::string GetName() const
  {
    return m_Name;
  }
  void SetName(const std::string &name)
  {
    m_Name = name;
  }

  /**
   * @brief 检查相机参数是否发生变化（脏标记）
   */
  bool IsCameraDirty() const
  {
    return m_Camera->IsProjectionDirty();
  }

 private:
  std::shared_ptr<Camera> m_Camera;        // 关联的相机对象
  std::shared_ptr<ShaderUBO> m_CameraUBO;  // 相机UBO实例
  uint32_t m_BindingPoint;                 // UBO绑定点
  std::string m_Name;                      // 实例名称
  bool m_UBOInitialized = false;           // UBO初始化状态

  // 禁用拷贝构造和赋值
  CameraInstance(const CameraInstance &) = delete;
  CameraInstance &operator=(const CameraInstance &) = delete;
};
}  // namespace mite

#endif  // MITE_CAMERA_INSTANCE_H
