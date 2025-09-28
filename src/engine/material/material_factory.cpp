#include "material_factory.h"
#include "basic_data/shader_cache.h"
#include "material_templates/material_template_pure_color.h"
#include "material_templates/material_template_gltf_pbr.h"

namespace mite {
void MaterialFactory::Initialize()
{
  // 初始化LOGGER
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite Material System");
  m_Logger->info("Create logger for material system");

  // 订阅MaterialLoad事件，创建材质
  m_EventSubscription.SubscribeAsync<MaterialLoadedEvent>(BIND_DISPATCH_FN(OnMaterialLoaded));

  // 注册材质
  m_Logger->info("Registering material templates");

  // 注册基础材质
  // TODO: shader的创建放在此处是否合理？待后续调整
  auto pureColorShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/pure_color.vert").string(),
      FileSystem::GetAssetPath("shaders/pure_color.frag").string());
  auto pureColorMaterialTemplate = std::make_unique<PureColorMaterialTemplate>(pureColorShader);
  RegisterTemplate(std::move(pureColorMaterialTemplate));

  // 注册PBR材质
  auto pbrShader = ShaderCache::Get().GetOpenGLShader(
      FileSystem::GetAssetPath("shaders/pbr.vert").string(),
      FileSystem::GetAssetPath("shaders/pbr.frag").string());
  auto pbrMaterialTemplate = std::make_unique<GLTFPBRMaterialTemplate>(pbrShader);
  RegisterTemplate(std::move(pbrMaterialTemplate));
}

void MaterialFactory::RegisterTemplate(std::unique_ptr<MaterialTemplate> material)
{
  // 使用MaterialType作为Key，不允许重复，
  std::string name = material->GetMaterialType();

  if (name.empty()) {
    // 材质模板名称不能为空
    m_Logger->error("Material template name cannot be empty");
    throw std::invalid_argument("Material template name cannot be empty");
  }

  auto it = m_Templates.find(name);
  if (it != m_Templates.end()) {
    // 材质模板已存在，跳过注册步骤。
    m_Logger->error("Existing material template: {}, registing failed", name);
    return;
  }
  // 注册材质模板
  m_Logger->info("Register material template: {}", name);
  m_Templates.emplace(name, std::move(material));
}

bool MaterialFactory::HasTemplate(const std::string &materialType) const
{
  return m_Templates.find(materialType) != m_Templates.end();
}

std::shared_ptr<MaterialInstance> MaterialFactory::CreateInstance(const std::string &templateName, const std::string &instanceName)
{
  m_Logger->info("Creating material instance with material template: {}.", templateName);
  // 1. 查找模板
  auto it = m_Templates.find(templateName);
  if (it == m_Templates.end()) {
    // 材质模板不存在, 使用回退材质
    m_Logger->warn("Invalid material template: {}, trying to use fallback material.",
                   templateName);
    if (!m_FallbackMaterial) {
      // 无可用回退材质
      m_Logger->error("There has not any fallback material to use.");
      throw std::out_of_range("There has not any fallback material to use.");
    }
    std::shared_ptr<MaterialInstance> fallbackInstance = m_FallbackMaterial->CreateInstance();
    return fallbackInstance;
  }

  // 2. 创建实例（通过模板工厂方法）
  std::shared_ptr<MaterialInstance> createInstance = it->second->CreateInstance();
  return createInstance;
}

std::shared_ptr<MaterialInstance> MaterialFactory::CreateInstanceWithOverrides(
    const std::string &templateName,
    const std::unordered_map<std::string, UniformVariant> &overrides,
    const std::string &instanceName)
{
  // 1. 创建基础材质实例
  std::shared_ptr<MaterialInstance> instance = CreateInstance(templateName, instanceName);

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
      case UniformVariant::Type::Texture: {
        instance->SetTexture(name, value.Get<TextureGPUSlot>());
        break;
      }
      default:
        m_Logger->error("Invalid OpenGL uniform item: {};", name);
        break;
    }
  }
  
  return instance;
}

std::shared_ptr<MaterialInstance> MaterialFactory::CreateInstanceFromMaterialSourceData(
    const MaterialSourceData &sourceData)
{
  std::string templateName = sourceData.templateName;
  std::string instanceName = sourceData.name;
  m_Logger->info("Creating material instance with material template: {}.", templateName);

  // 1. 查找模板
  auto it = m_Templates.find(templateName);
  if (it == m_Templates.end()) {
    // 材质模板不存在, 使用回退材质
    m_Logger->warn("Invalid material template: {}, trying to use fallback material.",
                   templateName);
    if (!m_FallbackMaterial) {
      // 无可用回退材质
      m_Logger->error("There has not any fallback material to use.");
      throw std::out_of_range("There has not any fallback material to use.");
    }
    std::shared_ptr<MaterialInstance> fallbackInstance = m_FallbackMaterial->CreateInstance(
        sourceData);
    return fallbackInstance;
  }

  // 2. 创建实例（通过模板工厂方法）
  std::shared_ptr<MaterialInstance> createInstance = it->second->CreateInstance(sourceData);
  return createInstance;
}


//void MaterialSystem::ReloadTemplate(const std::string &name, std::unique_ptr<MaterialTemplate> newMaterial)
//{
//  auto it = m_Templates.find(name);
//  if (it == m_Templates.end()) {
//    // 未能在注册列表中寻找到需要被reload的material
//    m_Logger->error("Reload failed，reloaded material name invalid: {}", name);
//    return;
//  }
//
//  // 1. 触发事件（旧材质即将被替换）
//  MaterialTemplate *oldMaterial = it->second.get();
//  MaterialReloadedEvent event(name, oldMaterial, newMaterial.get());
//  EventBus::Publish<MaterialReloadedEvent>(event);
//
//  // 2. 替换模板
//  it->second = std::move(newMaterial);
//  // 材质模板已重载
//  m_Logger->info("Material template has been reloaded: {}", name);
//}

void MaterialFactory::SetFallbackMaterial(std::unique_ptr<MaterialTemplate> material)
{
  if (!material) {
    // 回退材质不能为空
    m_Logger->error("Fallback material cannot be nullptr.");
    return;
  }
  m_FallbackMaterial = std::move(material);
  // 设置回退材质
  m_Logger->debug("Setting fallback material: {}", m_FallbackMaterial->GetName());
}

void MaterialFactory::OnMaterialLoaded(MaterialLoadedEvent &event) 
{
    // 获取sourcedata
  MaterialSourceData sourceData = event.GetSourceData();

  // 使用sourcedata创建材质实例
  std::shared_ptr<MaterialInstance> materialInstance = CreateInstanceFromMaterialSourceData(sourceData);

  // 委托Asset模块管理实例
  event.GetMaterialAsset()->instance = materialInstance;

  // 阻断事件传播
  event.SetResult(EventResult::HandledAndStop);
}

};  // namespace mite