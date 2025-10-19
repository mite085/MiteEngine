#ifndef MITE_CAMERA_INSTANCE_H
#define MITE_CAMERA_INSTANCE_H

#include "basic_data/camera.h"
#include "basic_data/transform.h"
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
  explicit CameraInstance(std::shared_ptr<Camera> camera);
  ~CameraInstance();

  // ==================== UBO管理接口 ====================
  /**
   * @brief 初始化相机UBO（创建时执行一次即可）
   * @return 是否初始化成功
   */
  bool InitializeUBO();
  /**
   * @brief 设置着色器绑定（着色器初始化之后，执行一次即可）
   * @param shader 着色器对象
   *
   * (使用固定的绑定点执行显示绑定，无需手动管理)
   */
  // void SetupShaderBinding(std::shared_ptr<OpenGLShader> shader);
  /**
   * @brief 更新相机UBO数据（SceneView负责每帧Update）
   * @param viewMatrix 视图矩阵
   * @return 是否更新成功
   */
  bool UpdateUBO(const Transform cameraTransform);
  /**
   * @brief 绑定相机UBO到当前渲染状态（DrawCall之前绑定）
   */
  void BindUBO() const;

  // ==================== 相机访问接口 ====================
  /**
   * @brief 获取关联的相机对象
   */
  std::shared_ptr<Camera> GetCamera() const { return m_Camera; }
  void SetCamera(std::shared_ptr<Camera> camera) { m_Camera = camera; }
  /**
   * @brief 获取相机变换/投影矩阵（原则上相机仅接受Zoom修改）
   */
  const glm::mat4 GetProjectionMatrix() const { return m_Camera->GetProjectionMatrix(); }
  const Transform &GetCameraTransform() const { return m_CameraTransform; }
  /**
   * @brief 获取UBO对象（用于外部管理）
   */
  std::shared_ptr<ShaderUBO> GetUBO() const { return m_CameraUBO; }

 private:
  std::shared_ptr<Camera> m_Camera;        // 关联的相机对象
  Transform m_CameraTransform;             // 相机世界空间变换（缓存UBO数据）
  std::shared_ptr<ShaderUBO> m_CameraUBO;  // 相机UBO实例

  // 禁用拷贝构造和赋值
  CameraInstance(const CameraInstance &) = delete;
  CameraInstance &operator=(const CameraInstance &) = delete;
};
}  // namespace mite

#endif  // MITE_CAMERA_INSTANCE_H
