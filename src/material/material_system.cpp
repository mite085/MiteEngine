#include "material_system.h"
#include "asset_manager.h"

namespace mite {
// 静态单例初始化
MaterialSystem &MaterialSystem::Get()
{
  static MaterialSystem instance;
  return instance;
}

void MaterialSystem::RegisterTemplate(const std::string &name, std::unique_ptr<Material> material)
{
  if (name.empty()) {
    // 材质模板名称不能为空
    LOG_ERROR("Material template name cannot be empty");
    throw std::invalid_argument("Material template name cannot be empty");
  }

  auto it = m_templates.find(name);
  if (it != m_templates.end()) {
    // 材质模板已存在，跳过注册步骤。
    LOG_ERROR("Existing material template: {}, registing failed", name);
    return;
  }
  // 注册材质模板
  LOG_DEBUG("Register material template: {}", name);
  m_templates.emplace(name, std::move(material));
}

bool MaterialSystem::HasTemplate(const std::string &name) const
{
  return m_templates.find(name) != m_templates.end();
}

std::shared_ptr<MaterialInstance> MaterialSystem::CreateInstance(const std::string &templateName)
{
  // 1. 查找模板
  auto it = m_templates.find(templateName);
  if (it == m_templates.end()) {
    // 材质模板不存在, 使用回退材质
    LOG_WARN("Invalid material template: {}, trying to use fallback material.", templateName);
    if (!m_fallbackMaterial) {
      // 无可用回退材质
      LOG_ERROR("There has not any fallback material to use.");
      throw std::out_of_range("There has not any fallback material to use.");
    }
    return m_fallbackMaterial->CreateInstance();
  }

  // 2. 创建实例（通过模板工厂方法）
  return it->second->CreateInstance();
}

std::shared_ptr<MaterialInstance> MaterialSystem::CreateInstanceWithOverrides(
    const std::string &templateName,
    const std::unordered_map<std::string, MaterialParameterVariant> &overrides)
{
  // 1. 创建基础材质实例（复用已有逻辑）
  auto instance = CreateInstance(templateName);
  if (!instance) {
    // 无法创建材质实例
    LOG_ERROR("Cannot create material instance: {}", templateName);
    return nullptr;
  }

  // 2. 应用覆盖参数（类型安全处理）
  for (const auto &[paramName, variant] : overrides) {
    try {
      // 使用visit自动匹配类型
      std::visit(
          [&](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;

            // 基础类型处理
            if constexpr (std::is_same_v<T, bool>) {
              instance->SetInt(paramName, arg ? 1 : 0);  // GLSL中bool用int表示
            }
            else if constexpr (std::is_same_v<T, int>) {
              instance->SetInt(paramName, arg);
            }
            else if constexpr (std::is_same_v<T, unsigned int>) {
              instance->SetInt(paramName, static_cast<int>(arg));  // 降级处理
            }
            else if constexpr (std::is_same_v<T, float>) {
              instance->SetFloat(paramName, arg);
            }
            // 向量/矩阵类型
            else if constexpr (std::is_same_v<T, glm::vec2>) {
              instance->SetVector2(paramName, arg);
            }
            else if constexpr (std::is_same_v<T, glm::vec3>) {
              instance->SetVector3(paramName, arg);
            }
            else if constexpr (std::is_same_v<T, glm::vec4>) {
              instance->SetVector4(paramName, arg);
            }
            else if constexpr (std::is_same_v<T, glm::mat3>) {
              instance->SetMatrix3(paramName, arg);
            }
            else if constexpr (std::is_same_v<T, glm::mat4>) {
              instance->SetMatrix4(paramName, arg);
            }
            // 数组类型
            else if constexpr (std::is_same_v<T, std::vector<int>>) {
              instance->SetIntArray(paramName, arg.data(), arg.size());
            }
            else if constexpr (std::is_same_v<T, std::vector<float>>) {
              instance->SetFloatArray(paramName, arg.data(), arg.size());
            }
            else if constexpr (std::is_same_v<T, std::vector<glm::vec3>>) {
              instance->SetVector3Array(paramName, arg.data(), arg.size());
            }
            // 纹理路径特殊处理
            else if constexpr (std::is_same_v<T, std::string>) {
              auto texture = AssetManager::Get().LoadTexture(arg);
              if (texture) {
                instance->SetTexture(paramName, std::make_shared<Texture>(texture->handle));
              }
              else {
                LOG_WARN("Cannot load texture: {}", arg);
              }
            }
            else {
              static_assert(always_false_v<T>, "非支持的材质参数类型");
            }
          },
          variant.Get());
    }
    catch (const std::bad_variant_access &e) {
      LOG_ERROR(
          "材质参数类型不匹配: {} ({}), 错误: {}", paramName, variant.GetTypeName(), e.what());
    }
  }
  return instance;
}

void MaterialSystem::ReloadTemplate(const std::string &name, std::unique_ptr<Material> newMaterial)
{
  auto it = m_templates.find(name);
  if (it == m_templates.end()) {
    LOG_ERROR("Reload failed，material invalid: {}", name);
    return;
  }

  // 1. 触发事件（旧材质即将被替换）
  Material *oldMaterial = it->second.get();
  MaterialReloadedEvent event(name, oldMaterial, newMaterial.get());
  EventBus::Get().Post(event);

  // 2. 替换模板
  it->second = std::move(newMaterial);
  // 材质模板已重载
  LOG_INFO("Material template has been reloaded: {}", name);
}

void MaterialSystem::SetFallbackMaterial(std::unique_ptr<Material> material)
{
  if (!material) {
    // 回退材质不能为空
    LOG_ERROR("Fallback material cannot be nullptr.");
    return;
  }
  m_fallbackMaterial = std::move(material);
  // 设置回退材质
  LOG_DEBUG("Setting fallback material: {}", m_fallbackMaterial->GetName());
}
};  // namespace mite