#ifndef MITE_MATERIAL_FACTORY
#define MITE_MATERIAL_FACTORY

#include "basic_event/asset_event.h"
#include "material_template.h"

namespace mite {
/**
 * @brief 材质工厂，负责使用注册好的材质模板创建材质实例
 * @职责：
 * 1. 全局材质模板的注册与生命周期管理
 * 2. 材质实例的创建
 * 3. 材质热重载支持（暂不启用）
 * 4. 错误材质回退机制（模板参数错误也能正常返回实例）
 *
 * 使用示例：
 *
 * // 模板注册阶段
 * auto pbrTemplate = std::make_unique<PBRMaterialTemplate>(shader);
 * MaterialSystem::Get().RegisterTemplate("DefaultPBR", std::move(pbrTemplate));

 * // 实例创建阶段
 * auto material = MaterialSystem::Get().CreateInstanceWithOverrides(
 *     "DefaultPBR",
 *     {{"u_Albedo", glm::vec3(1.0, 0.0, 0.0)}, {"u_Roughness", 0.8f}}
 * );
 */
class MaterialFactory {
 public:
  static MaterialFactory &Get()
  {
    static MaterialFactory system;
    return system;
  }

  // ---- 初始化：注册材质----
  void Initialize();

  // ---- 模板管理 ----
  /**
   * @brief 注册材质模板到系统
   * @param name     材质模板名称（唯一标识符）
   * @param template 材质模板对象（所有权转移给系统）
   * @throws std::invalid_argument 如果名称已存在
   */
  void RegisterTemplate(std::unique_ptr<MaterialTemplate> material);

  /**
   * @brief 检查材质模板是否存在
   * @param name 材质模板名称
   */
  bool HasTemplate(const MaterialType &materialType) const;

  // ---- 实例管理 ----
  /**
   * @brief 创建材质实例
   * @param templateName 模板名称
   * @return 共享指针管理的材质实例
   * @throws std::out_of_range 如果模板不存在
   *
   * 作用：
   * 当在运行时动态决定材质类型时（如从配置文件读取）使用，更便捷且可扩展性更强
   */
  std::shared_ptr<MaterialInstance> CreateInstance(const MaterialType &type,
                                                   const std::string &instanceName = "");
  /**
   * @brief 基于资产模块载入的材质源数据创建材质实例
   * @param sourceData 
   * @return 
   */
  std::shared_ptr<MaterialInstance> CreateInstanceFromMaterialSourceData(
      const MaterialSourceData &sourceData);

  // ---- 热重载支持（预留接口） ----
  /**
   * @brief 重新加载材质模板（用于开发时实时编辑）
   * @param name 模板名称
   * @param newMaterial 新材质模板
   * @note 会触发MaterialReloadedEvent事件
   */
  //void ReloadTemplate(const std::string &name, std::unique_ptr<MaterialTemplate> newMaterial);

  // ---- 错误处理 ----
  /**
   * @brief 设置默认回退材质（当模板不存在时使用）
   * @param material 默认材质模板
   */
  void SetFallbackMaterial(std::unique_ptr<MaterialTemplate> material);

 private:
  // ---- 私有构造函数 ----
  MaterialFactory() = default;
  ~MaterialFactory() = default;

  // 消费MaterialLoad事件，生成材质实例
  void OnMaterialLoaded(MaterialLoadedEvent &event);

  // 日志系统
  Logger m_Logger;

  // 事件订阅
  SubscriptionGroup m_EventSubscription;

  // ---- 成员变量 ----
  std::unordered_map<MaterialType, std::unique_ptr<MaterialTemplate>> m_Templates;  // 模板存储
  std::unique_ptr<MaterialTemplate> m_FallbackMaterial;  // 错误回退材质
};
};  // namespace mite

#endif
