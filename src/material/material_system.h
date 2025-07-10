#ifndef MITE_MATERIAL_SYSTEM
#define MITE_MATERIAL_SYSTEM

#include "material.h"
#include "material_param_variant.h"

namespace mite {
/**
 * @brief 材质系统核心管理器
 * @职责：
 * 1. 全局材质模板的注册与生命周期管理
 * 2. 材质实例的创建与缓存
 * 3. 材质热重载支持（通过文件监视或手动触发）
 * 4. 错误材质回退机制
 */
class MaterialSystem {
 public:
  // ---- 单例模式----
  static MaterialSystem &Get();

  // ---- 模板管理 ----
  /**
   * @brief 注册材质模板到系统
   * @param name     材质模板名称（唯一标识符）
   * @param template 材质模板对象（所有权转移给系统）
   * @throws std::invalid_argument 如果名称已存在
   */
  void RegisterTemplate(const std::string &name, std::unique_ptr<Material> material);

  /**
   * @brief 检查材质模板是否存在
   * @param name 材质模板名称
   */
  bool HasTemplate(const std::string &name) const;

  // ---- 实例管理 ----
  /**
   * @brief 创建材质实例
   * @param templateName 模板名称
   * @return 共享指针管理的材质实例
   * @throws std::out_of_range 如果模板不存在
   */
  std::shared_ptr<MaterialInstance> CreateInstance(const std::string &templateName);

  /**
   * @brief 创建带有初始参数的材质实例（便捷接口）
   * @param templateName 模板名称
   * @param overrides    参数覆盖键值对（如{{"u_Color", glm::vec3(1,0,0)}}）
   */
  std::shared_ptr<MaterialInstance> CreateInstanceWithOverrides(
      const std::string &templateName,
      const std::unordered_map<std::string, UniformVariant> &overrides);

  // ---- 热重载支持 ----
  /**
   * @brief 重新加载材质模板（用于开发时实时编辑）
   * @param name 模板名称
   * @param newMaterial 新材质模板
   * @note 会触发MaterialReloadedEvent事件
   */
  void ReloadTemplate(const std::string &name, std::unique_ptr<Material> newMaterial);

  // ---- 错误处理 ----
  /**
   * @brief 设置默认回退材质（当模板不存在时使用）
   * @param material 默认材质模板
   */
  void SetFallbackMaterial(std::unique_ptr<Material> material);

 private:
  // 私有构造（强制单例）
  MaterialSystem() = default;

  // ---- 成员变量 ----
  std::unordered_map<std::string, std::unique_ptr<Material>> m_templates;  // 模板存储
  std::unique_ptr<Material> m_fallbackMaterial;                            // 错误回退材质
  std::unordered_map<size_t, std::weak_ptr<MaterialInstance>> m_instanceCache;  // 实例弱引用缓存
};

/**
 * @class MaterialReloadedEvent
 * @brief 材质模板重载修改事件
 */
class MaterialReloadedEvent : public Event {
 public:
  MaterialReloadedEvent(const std::string &templateName,
                        Material *oldMaterial,
                        Material *newMaterial)
      : templateName(templateName), oldMaterial(oldMaterial), newMaterial(newMaterial)
  {
  }

  EVENT_CLASS_TYPE(MATERIAL_CHANGED)
  EVENT_CLASS_CATEGORY(EVENT_CATEGORY_SCENE_CHANGE)
  Event *Clone() const override
  {
    return new MaterialReloadedEvent(templateName, oldMaterial, newMaterial);
  }

 private:
  std::string templateName;         // 被重载的模板名
  Material *oldMaterial = nullptr;  // 旧材质指针（可能已失效）
  Material *newMaterial = nullptr;  // 新材质指针
};
};  // namespace mite

#endif
