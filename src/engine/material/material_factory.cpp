#include "material_factory.h"

#include "basic_shader/shader_cache.h"
#include "material_templates/material_template_gltf_pbr.h"
#include "material_templates/material_template_pure_color.h"

namespace mite {
void MaterialFactory::Initialize() {
  // 初始化LOGGER
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Material Factory");
  m_Logger->info("Create logger for material factory");

  // 订阅MaterialLoad事件，创建材质
  // Immediate立即模式：
  // 材质创建可能存在UBO创建与绑定等操作，需要在OpenGL上下文所在的主线程执行
  m_EventSubscription.SubscribeImmediate<MaterialLoadedEvent>(
      BIND_DISPATCH_FN(OnMaterialLoaded));

  // 注册材质
  m_Logger->info("Registering material templates");

  // 注册基础材质
  auto pureColorMaterialTemplate = std::make_unique<EmissionMaterialTemplate>();
  RegisterTemplate(std::move(pureColorMaterialTemplate));

  // 注册PBR材质
  auto pbrMaterialTemplate = std::make_unique<GLTFPBRMaterialTemplate>();
  RegisterTemplate(std::move(pbrMaterialTemplate));
}

void MaterialFactory::RegisterTemplate(
    std::unique_ptr<MaterialTemplate> material) {
  // 使用MaterialType作为Key，不允许重复，
  MaterialType type = material->GetMaterialType();

  auto it = m_Templates.find(type);
  if (it != m_Templates.end()) {
    // 材质模板已存在，跳过注册步骤。
    m_Logger->error("Existing material template, registing failed");
    return;
  }
  // 注册材质模板
  m_Logger->info("Register material template");
  m_Templates.emplace(type, std::move(material));
}

bool MaterialFactory::HasTemplate(const MaterialType &materialType) const {
  return m_Templates.find(materialType) != m_Templates.end();
}

std::shared_ptr<MaterialInstance> MaterialFactory::CreateInstance(
    const MaterialType &type) {
  m_Logger->info("Creating material instance with material template.");
  // 1. 查找模板
  auto it = m_Templates.find(type);
  if (it == m_Templates.end()) {
    // 材质模板不存在, 使用回退材质
    m_Logger->warn(
        "Invalid material template, trying to use fallback material.");
    if (!m_FallbackMaterial) {
      // 无可用回退材质
      m_Logger->error("There has not any fallback material to use.");
      throw std::out_of_range("There has not any fallback material to use.");
    }
    std::shared_ptr<MaterialInstance> fallbackInstance =
        m_FallbackMaterial->CreateInstance();
    return fallbackInstance;
  }

  // 2. 创建实例（通过模板工厂方法）
  std::shared_ptr<MaterialInstance> createInstance =
      it->second->CreateInstance();
  return createInstance;
}

std::shared_ptr<MaterialInstance>
MaterialFactory::CreateInstanceFromMaterialSourceData(
    const MaterialSourceData &sourceData) {
  MaterialType type = sourceData.type;
  std::string instanceName = sourceData.name;
  m_Logger->info("Creating material instance with material template");

  // 1. 查找模板
  auto it = m_Templates.find(type);
  if (it == m_Templates.end()) {
    // 材质模板不存在, 使用回退材质
    m_Logger->warn(
        "Invalid material template, trying to use fallback material.");
    if (!m_FallbackMaterial) {
      // 无可用回退材质
      m_Logger->error("There has not any fallback material to use.");
      throw std::out_of_range("There has not any fallback material to use.");
    }
    std::shared_ptr<MaterialInstance> fallbackInstance =
        m_FallbackMaterial->CreateInstance(sourceData);
    return fallbackInstance;
  }

  // 2. 创建实例（通过模板工厂方法）
  std::shared_ptr<MaterialInstance> createInstance =
      it->second->CreateInstance(sourceData);
  return createInstance;
}

// void MaterialSystem::ReloadTemplate(const std::string &name,
// std::unique_ptr<MaterialTemplate> newMaterial)
//{
//   auto it = m_Templates.find(name);
//   if (it == m_Templates.end()) {
//     // 未能在注册列表中寻找到需要被reload的material
//     m_Logger->error("Reload failed，reloaded material name invalid: {}",
//     name); return;
//   }
//
//   // 1. 触发事件（旧材质即将被替换）
//   MaterialTemplate *oldMaterial = it->second.get();
//   MaterialReloadedEvent event(name, oldMaterial, newMaterial.get());
//   EventBus::Publish<MaterialReloadedEvent>(event);
//
//   // 2. 替换模板
//   it->second = std::move(newMaterial);
//   // 材质模板已重载
//   m_Logger->info("Material template has been reloaded: {}", name);
// }

void MaterialFactory::SetFallbackMaterial(
    std::unique_ptr<MaterialTemplate> material) {
  if (!material) {
    // 回退材质不能为空
    m_Logger->error("Fallback material cannot be nullptr.");
    return;
  }
  m_FallbackMaterial = std::move(material);
  // 设置回退材质
  m_Logger->debug("Setting fallback material: {}",
                  m_FallbackMaterial->GetName());
}

void MaterialFactory::OnMaterialLoaded(MaterialLoadedEvent &event) {
  // 获取sourcedata
  MaterialSourceData sourceData = event.GetSourceData();

  // 使用sourcedata创建材质实例
  std::shared_ptr<MaterialInstance> materialInstance =
      CreateInstanceFromMaterialSourceData(sourceData);

  // 委托Asset模块管理实例
  event.GetMaterialAsset()->instance = materialInstance;

  // 阻断事件传播
  event.SetResult(EventResult::HandledAndStop);
}
};  // namespace mite