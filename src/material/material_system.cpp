#include "material_system.h"
#include "asset_manager.h"
#include "material_template.h"
#include "basic_data/shader_cache.h"

namespace mite {
// 静态单例初始化
MaterialSystem &MaterialSystem::Get()
{
  static MaterialSystem instance;
  return instance;
}

void MaterialSystem::Initialize()
{  
  // 注册基础材质
  // TODO: shader的创建放在此处是否合理？待后续调整
  auto basicShader = ShaderCache::Get().GetOpenGLShader("shader/basic.vert", "shader/basic.frag");
  auto basicTemplate = std::make_unique<BasicMaterialTemplate>(basicShader);
  MaterialSystem::Get().RegisterTemplate("BasicMaterial", std::move(basicTemplate));

  // 注册PBR材质
  auto pbrShader = ShaderCache::Get().GetOpenGLShader("shader/pbr.vert", "shader/pbr.frag");
  auto pbrTemplate = std::make_unique<PBRMaterialTemplate>(pbrShader);
  MaterialSystem::Get().RegisterTemplate("DefaultPBR", std::move(pbrTemplate));
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
    const std::unordered_map<std::string, UniformVariant> &overrides)
{
  // 1. 创建基础材质实例（复用已有逻辑）
  auto instance = CreateInstance(templateName);
  if (!instance) {
    // 无法创建材质实例
    LOG_ERROR("Cannot create material instance: {}", templateName);
    return nullptr;
  }

  // 2. 应用覆盖参数（类型安全处理）
  for (const auto &[name, value] : overrides) {
    switch (value.GetType()) {
      case UniformVariant::Type::Float:
        instance->SetFloat(name, value.Get<float>());
        break;
      case UniformVariant::Type::Int:
        instance->SetInt(name, value.Get<int>());
        break;
      case UniformVariant::Type::Vector2:
        instance->SetVector2(name, value.Get<glm::vec2>());
        break;
      case UniformVariant::Type::Vector3:
        instance->SetVector3(name, value.Get<glm::vec3>());
        break;
      case UniformVariant::Type::Vector4:
        instance->SetVector4(name, value.Get<glm::vec4>());
        break;
      case UniformVariant::Type::Matrix3:
        instance->SetMatrix3(name, value.Get<glm::mat3>());
        break;
      case UniformVariant::Type::Matrix4:
        instance->SetMatrix4(name, value.Get<glm::mat4>());
        break;
      case UniformVariant::Type::IntArray: {
        auto [ptr, count] = value.GetArray<int>();
        instance->SetIntArray(name, ptr, count);
        break;
      }
      case UniformVariant::Type::FloatArray: {
        auto [ptr, count] = value.GetArray<float>();
        instance->SetFloatArray(name, ptr, count);
        break;
      }
      case UniformVariant::Type::Vector3Array: {
        auto [ptr, count] = value.GetArray<glm::vec3>();
        instance->SetVector3Array(name, ptr, count);
        break;
      }
      case UniformVariant::Type::String: {
        // 纹理路径特殊处理
        auto texture = AssetManager::Get().GetTexture(AssetManager::Get().LoadTexture(name));
        if (texture) {
          instance->SetTexture(name, std::make_shared<Texture>(texture->handle));
        }
        else {
          LOG_WARN("Cannot load texture: {}", name);
        }
        break;
      }
      default:
        LOG_ERROR("Invalid OpenGL uniform item: {};", name);
        break;
    }
  }

  return instance;
}

void MaterialSystem::ReloadTemplate(const std::string &name, std::unique_ptr<Material> newMaterial)
{
  auto it = m_templates.find(name);
  if (it == m_templates.end()) {
    // 未能在注册列表中寻找到需要被reload的material
    LOG_ERROR("Reload failed，reloaded material name invalid: {}", name);
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