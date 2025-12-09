#ifndef MITE_LIGHT_MANAGER_H
#define MITE_LIGHT_MANAGER_H

#include "basic_instance/light_ssbo.h"
#include "shadow_instance.h"

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
   * @param maxLights 支持的最大光源数量，默认128个
   */
  explicit LightManager(size_t maxLights = 128);
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
  std::shared_ptr<Light> CreatePointLight();
  std::shared_ptr<Light> CreateSpotLight();
  std::shared_ptr<Light> CreateDirectionalLight();
  // std::shared_ptr<Light> CreateAreaRectLight();
  // std::shared_ptr<Light> CreateAreaEllipseLight();
  /**
   * @brief 创建指定类型的光源
   * @param type 光源类型
   * @return 光源共享指针，如果类型无效返回nullptr
   */
  std::shared_ptr<Light> CreateLight(LightType type);
  /**
   * @brief 添加光源到管理器
   * @param light 要添加的光源指针
   * @return 是否添加成功
   * @note 光源所有权由调用方管理，管理器只持有引用
   */
  bool AddLight(std::shared_ptr<Light> light);
  /**
   * @brief 移除指定光源
   * @param light 要移除的光源指针
   * @return 是否移除成功
   */
  bool RemoveLight(std::shared_ptr<Light> light);
  /**
   * @brief 移除所有光源
   */
  void ClearAllLights();
  /**
   * @brief 获取所有光源列表
   * @return 光源指针列表
   */
  const std::vector<std::shared_ptr<Light>> &GetAllLights() const;
  /**
   * @brief 获取启用的光源列表
   * @return 启用的光源指针列表
   */
  std::vector<std::shared_ptr<Light>> GetEnabledLights() const;

  // ---- 数据更新 ----
  /**
   * @brief 更新所有光源数据到GPU
   * @param worldTransforms 光源世界变换映射表，key为光源指针，value为变换矩阵
   * @return 是否更新成功
   * @note 需要提供光源的当前世界变换矩阵
   */
  bool UpdateLightData(const std::unordered_map<Light *, Transform> &worldTransforms);

  // ---- 光源SSBO管理 ----

  /**
   * @brief 获取LightSSBO实例
   * @return LightSSBO共享指针
   */
  std::shared_ptr<LightShaderStorgeBuffer> GetLightSSBO() const;

  // ---- 阴影管理 ----
  /**
   * @brief 初始化阴影实例
   * @return 是否初始化成功
   */
  bool InitializeShadowInstance();
  /**
   * @brief 更新光源阴影数据
   * @param cameraInstance 相机实例，用于阴影计算
   * @return 是否更新成功
   * @note 使用UpdateLightData阶段缓存的光源变换数据
   */
  bool UpdateLightShadowUBO(std::shared_ptr<CameraInstance> cameraInstance,
                            glm::vec4 shadowParams);
  /**
   * @brief 获取阴影实例
   * @return 阴影实例共享指针
   */
  std::shared_ptr<ShadowInstance> GetShadowInstance() const { return m_ShadowInstance; }

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

  /**
   * @brief 基于每帧缓存获取光源变换
   */
  Transform GetLightTransform(Light *lightPtr) const;

  /**
   * @brief 获取按类型统计的光源数量
   */
  size_t GetLightCountByType(LightType type) const;
  std::vector<std::shared_ptr<Light>> GetLightsByType(LightType type) const;

 private:
  // ---- 内部方法 ----
  /**
   * @brief 准备GPU光源数据
   * @param worldTransforms 光源世界变换映射表
   * @return GPU光源数据列表
   */
  std::vector<GPULightData> PrepareGPULightData(
      const std::unordered_map<Light *, Transform> &worldTransforms) const;
  /**
   * @brief 验证光源是否可以添加
   * @param light 要验证的光源
   * @return 是否可以添加
   */
  bool CanAddLight(std::shared_ptr<Light> light) const;

  // ---- 成员变量 ----
  std::vector<std::shared_ptr<Light>> m_Lights;                          // 所有光源列表
  mutable std::unordered_map<Light *, Transform> m_LightTransformCache;  // 光源变换缓存
  std::shared_ptr<LightShaderStorgeBuffer> m_LightSSBO;                  // 光源统一的SSBO管理器
  std::shared_ptr<ShadowInstance> m_ShadowInstance;                      // 阴影实例
  size_t m_MaxLights;                                                    // 最大光源数量
  bool m_IsInitialized = false;                                          // 初始化状态标志
};
}  // namespace mite

#endif  // MITE_LIGHT_MANAGER_H
