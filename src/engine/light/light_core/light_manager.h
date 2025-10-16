#ifndef MITE_LIGHT_MANAGER_H
#define MITE_LIGHT_MANAGER_H

#include "basic_instance/light_ssbo.h"
#include "light.h"
#include "shadow_map_type.h"

namespace mite {
/**
 * @brief 光源管理器 (暂时使用单例模式，待后续整理调用层次)
 * @note 职责：
 * 1. 统一管理所有光源实例的生命周期
 * 2. 协调光源数据的GPU更新和同步
 * 3. 提供光源的查询和过滤功能
 * 4. 管理阴影数据的收集和分发
 * 5. 支持光源的添加、删除和更新操作
 */
class LightManager {
 public:
  static LightManager &Get()
  {
    static LightManager manager;
    return manager;
  }
  /**
   * @brief 构造函数
   * @param maxLights 支持的最大光源数量，默认1024个
   */
  explicit LightManager(size_t maxLights = 1024);
  ~LightManager() = default;

  // ---- 生命周期管理 ----
  /**
   * @brief 初始化光源管理器
   * @return 是否初始化成功
   * @note 创建LightSSBO并设置默认参数
   */
  bool Initialize();
  /**
   * @brief 销毁光源管理器
   * @note 清理所有光源资源和GPU缓冲区
   */
  void Destroy();
  /**
   * @brief 检查是否已初始化
   * @return 初始化状态
   */
  bool IsInitialized() const;

  // ---- 光源管理 ----
  /**
   * @brief 创建点光源/聚光灯/方向光/矩形与椭圆面光源
   * @return 光源共享指针
   */
  LightPtr CreatePointLight();
  //LightPtr CreateSpotLight();
  //LightPtr CreateDirectionalLight();
  //LightPtr CreateAreaRectLight();
  //LightPtr CreateAreaEllipseLight();

  /**
   * @brief 创建指定类型的光源
   * @param type 光源类型
   * @return 光源共享指针，如果类型无效返回nullptr
   */
  LightPtr CreateLight(LightType type);
  /**
   * @brief 添加光源到管理器
   * @param light 要添加的光源指针
   * @return 是否添加成功
   * @note 光源所有权由调用方管理，管理器只持有引用
   */
  bool AddLight(LightPtr light);
  /**
   * @brief 移除指定光源
   * @param light 要移除的光源指针
   * @return 是否移除成功
   */
  bool RemoveLight(LightPtr light);
  /**
   * @brief 移除所有光源
   */
  void ClearAllLights();
  /**
   * @brief 获取所有光源列表
   * @return 光源指针列表
   */
  const std::vector<LightPtr> &GetAllLights() const;
  /**
   * @brief 获取启用的光源列表
   * @return 启用的光源指针列表
   */
  std::vector<LightPtr> GetEnabledLights() const;
  /**
   * @brief 获取投射阴影的光源列表
   * @return 投射阴影的光源指针列表
   */
  std::vector<LightPtr> GetShadowCastingLights() const;

  // ---- 数据更新 ----
  /**
   * @brief 更新所有光源数据到GPU
   * @param worldTransforms 光源世界变换映射表，key为光源指针，value为变换矩阵
   * @return 是否更新成功
   * @note 需要提供光源的当前世界变换矩阵
   */
  bool UpdateLightData(const std::unordered_map<LightPtr, Transform> &worldTransforms);
  /**
   * @brief 收集所有阴影数据
   * @param worldTransforms 光源世界变换映射表
   * @param cameraView 相机视图矩阵，用于级联阴影计算
   * @param cameraProj 相机投影矩阵，用于级联阴影计算
   * @return 阴影数据列表
   */
  std::vector<ShadowMapData> CollectShadowData(
      const std::unordered_map<LightPtr, Transform> &worldTransforms,
      const Transform &cameraView,
      const glm::mat4 &cameraProj = glm::mat4(1.0f)) const;

  // ---- 统计信息 ----

  /**
   * @brief 获取总光源数量
   * @return 光源总数
   */
  size_t GetTotalLightCount() const;

  /**
   * @brief 获取启用的光源数量
   * @return 启用光源数量
   */
  size_t GetEnabledLightCount() const;

  /**
   * @brief 获取投射阴影的光源数量
   * @return 投射阴影的光源数量
   */
  size_t GetShadowCastingLightCount() const;

  /**
   * @brief 获取按类型统计的光源数量
   * @return 类型到数量的映射表
   */
  std::unordered_map<LightType, size_t> GetLightCountByType() const;

  // ---- SSBO管理 ----

  /**
   * @brief 获取LightSSBO实例
   * @return LightSSBO共享指针
   */
  std::shared_ptr<LightShaderStorgeBuffer> GetLightSSBO() const;

  /**
   * @brief 设置着色器的光源SSBO绑定点
   * @param shader 目标着色器
   * (使用固定的绑定点执行显示绑定，无需手动管理)
   */
  // void SetupShaderBinding(std::shared_ptr<OpenGLShader> shader) const;

  /**
   * @brief 绑定光源SSBO到指定绑定点
   */
  void BindLightSSBO() const;

  // ---- 配置管理 ----

  /**
   * @brief 设置最大光源数量
   * @param maxLights 新的最大光源数量
   * @note 只能在初始化前调用
   */
  void SetMaxLights(size_t maxLights);

  /**
   * @brief 获取最大光源数量
   * @return 最大光源数量
   */
  size_t GetMaxLights() const;

 private:
  // ---- 内部方法 ----
  /**
   * @brief 准备GPU光源数据
   * @param worldTransforms 光源世界变换映射表
   * @return GPU光源数据列表
   */
  std::vector<GPULightData> PrepareGPULightData(
      const std::unordered_map<LightPtr, Transform> &worldTransforms) const;

  /**
   * @brief 验证光源是否可以添加
   * @param light 要验证的光源
   * @return 是否可以添加
   */
  bool CanAddLight(LightPtr light) const;

  // ---- 成员变量 ----
  std::vector<LightPtr> m_Lights;                        // 所有光源列表
  std::shared_ptr<LightShaderStorgeBuffer> m_LightSSBO;  // 光源统一的SSBO管理器
  size_t m_MaxLights;                                    // 最大光源数量
  bool m_IsInitialized = false;                          // 初始化状态标志
};
}  // namespace mite

#endif  // MITE_LIGHT_MANAGER_H
