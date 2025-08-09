#include "material_system.h"
#include "basic_data/shader_cache.h"

namespace mite {
MaterialSystem::MaterialSystem(AssetManager &assetManager) : m_AssetManager(assetManager)
{
  // 初始化LOGGER
  m_logger = mite::LoggerSystem::CreateModuleLogger("Mite Material System");
  m_logger->info("Create logger for material system");
}

void MaterialSystem::Initialize()
{
  // 注册材质
  m_logger->info("Registering material templates");

  // 注册基础材质
  // TODO: shader的创建放在此处是否合理？待后续调整
  auto pureColorShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/pure_color.vert").string(),
      FileSystem::GetAssetPath("shaders/pure_color.frag").string());
  auto pureColorMaterialTemplate = std::make_unique<PureColorMaterialTemplate>(pureColorShader);
  std::string basicType = pureColorMaterialTemplate->GetMaterialType();
  RegisterTemplate(basicType, std::move(pureColorMaterialTemplate));

  // 注册PBR材质
  auto pbrShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/pbr.vert").string(),
      FileSystem::GetAssetPath("shaders/pbr.frag").string());
  auto pbrMaterialTemplate = std::make_unique<PBRMaterialTemplate>(pbrShader);
  std::string pbrType = pbrMaterialTemplate->GetMaterialType();
  RegisterTemplate(pbrType, std::move(pbrMaterialTemplate));
}

void MaterialSystem::RegisterTemplate(const std::string &name, std::unique_ptr<Material> material)
{
  if (name.empty()) {
    // 材质模板名称不能为空
    m_logger->error("Material template name cannot be empty");
    throw std::invalid_argument("Material template name cannot be empty");
  }

  auto it = m_templates.find(name);
  if (it != m_templates.end()) {
    // 材质模板已存在，跳过注册步骤。
    m_logger->error("Existing material template: {}, registing failed", name);
    return;
  }
  // 注册材质模板
  m_logger->info("Register material template: {}", name);
  m_templates.emplace(name, std::move(material));
}

bool MaterialSystem::HasTemplate(const std::string &name) const
{
  return m_templates.find(name) != m_templates.end();
}

std::shared_ptr<MaterialInstance> MaterialSystem::CreateInstance(const std::string &templateName)
{
  m_logger->info("Creating material instance with material template: {}.", templateName);
  // 1. 查找模板
  auto it = m_templates.find(templateName);
  if (it == m_templates.end()) {
    // 材质模板不存在, 使用回退材质
    m_logger->warn("Invalid material template: {}, trying to use fallback material.",
                   templateName);
    if (!m_fallbackMaterial) {
      // 无可用回退材质
      m_logger->error("There has not any fallback material to use.");
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
    m_logger->error("Cannot create material instance: {}", templateName);
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
        auto texture = m_AssetManager.GetTexture(m_AssetManager.LoadTexture(name));
        if (texture) {
          instance->SetTexture(name, std::make_shared<Texture>(texture->handle));
        }
        else {
          m_logger->warn("Cannot load texture: {}", name);
        }
        break;
      }
      default:
        m_logger->error("Invalid OpenGL uniform item: {};", name);
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
    m_logger->error("Reload failed，reloaded material name invalid: {}", name);
    return;
  }

  // 1. 触发事件（旧材质即将被替换）
  Material *oldMaterial = it->second.get();
  MaterialReloadedEvent event(name, oldMaterial, newMaterial.get());
  EventBus::Get().Post(event);

  // 2. 替换模板
  it->second = std::move(newMaterial);
  // 材质模板已重载
  m_logger->info("Material template has been reloaded: {}", name);
}

void MaterialSystem::SetFallbackMaterial(std::unique_ptr<Material> material)
{
  if (!material) {
    // 回退材质不能为空
    m_logger->error("Fallback material cannot be nullptr.");
    return;
  }
  m_fallbackMaterial = std::move(material);
  // 设置回退材质
  m_logger->debug("Setting fallback material: {}", m_fallbackMaterial->GetName());
}
};  // namespace mite