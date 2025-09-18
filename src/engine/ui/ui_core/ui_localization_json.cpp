#include "ui_localization_json.h"
#include "ui_event/ui_events_lifecycle.h"
#include "ui_imgui_backend/ui_imgui_font_manager.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace mite {

UILocalizationJson::UILocalizationJson()
{
  // 初始化日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite UI Localization Json");
  m_Logger->info("Initializing UII Localization Json");


  InitializeBuiltinLanguages();
  SetCurrentLanguage(SIMPLIFIED_CHINESE);  // 默认中文

  // 订阅语言变更事件
  // Immediate同步模式：
  // 语言切换需要立即生效以确保UI界面实时更新，避免异步导致的界面显示不一致
  m_EventSubscriptions.SubscribeImmediate<LanguageChangedEvent>(
      BIND_DISPATCH_FN(OnLanguageChanged), EventPriority::Normal);
}

UILocalizationJson::~UILocalizationJson() = default;

void UILocalizationJson::InitializeBuiltinLanguages()
{
  // 尝试从文件加载
  std::string enPath = GetLocalizationFilePath("en-US");
  if (!enPath.empty() && LoadLanguagePackFromFile(ENGLISH, enPath)) {
    m_Logger->info("Loaded English language pack from file");
  }
  else {
    m_Logger->error("Failed in Loading English language pack from file");
  }

  std::string zhPath = GetLocalizationFilePath("zh-CN");
  if (!zhPath.empty() && LoadLanguagePackFromFile(SIMPLIFIED_CHINESE, zhPath)) {
    m_Logger->info("Loaded Simplified Chinese language pack from file");
  }
  else {
    m_Logger->error("Failed in Loading Simplified Chinese language pack from file");
  }
}

bool UILocalizationJson::LoadLanguagePack(const std::string &languageCode,
                                          const std::string &filePath)
{
  return LoadLanguagePackFromFile(languageCode, filePath);
}


void UILocalizationJson::OnLanguageChanged(LanguageChangedEvent &e)
{
  std::string languageCode = e.GetNewLanguage();
  if (SetCurrentLanguage(languageCode)) {
    // 语言切换成功，消费事件（其他本地化系统不需要重复处理）
    e.SetResult(EventResult::Consumed);
  }
  else {
    // 语言切换失败，标记处理失败但允许其他系统尝试
    e.SetResult(EventResult::Failed);
  }
}

bool UILocalizationJson::LoadLanguagePackFromFile(const std::string &languageCode,
                                                  const std::string &filePath)
{
  try {
    if (!FileSystem::Exists(filePath)) {
      m_Logger->error("Localization file not found: {}", filePath);
      return false;
    }

    // 读取文件内容
    std::string content = FileSystem::ReadFileToString(filePath);
    json jsonData = json::parse(content);

    // 验证JSON格式
    if (!ValidateLanguagePack(jsonData)) {
      m_Logger->error("Invalid localization file format: {}", filePath);
      return false;
    }

    LanguagePack pack;
    if (!ParseLanguagePack(jsonData, pack)) {
      return false;
    }

    m_LanguagePacks[languageCode] = pack;
    m_Logger->info("Loaded language pack: {} from {}", languageCode, filePath);
    return true;
  }
  catch (const json::exception &e) {
    m_Logger->error("JSON parsing error in {}: {}", filePath, e.what());
    return false;
  }
  catch (const std::exception &e) {
    m_Logger->error("Failed to load language pack from {}: {}", filePath, e.what());
    return false;
  }
}

std::string UILocalizationJson::GetLocalizationFilePath(const std::string &languageCode) const
{
  try {
    // 直接构建相对于assets的路径
    std::string relativePath = "localization/" + languageCode + ".json";
    fs::path fullPath = FileSystem::GetAssetPath(relativePath);

    if (FileSystem::Exists(fullPath)) {
      return fullPath.string();
    }

    return "";  // 文件不存在
  }
  catch (const std::exception &e) {
    m_Logger->debug("Localization file not found: {}", e.what());
    return "";
  }
}

bool UILocalizationJson::ValidateLanguagePack(const json &jsonData) const
{
  // 基本验证：检查必需字段
  if (!jsonData.is_object()) {
    m_Logger->error("Localization file must be a JSON object");
    return false;
  }

  if (!jsonData.contains("metadata") || !jsonData["metadata"].is_object()) {
    m_Logger->error("Localization file must contain metadata object");
    return false;
  }

  const auto &metadata = jsonData["metadata"];
  if (!metadata.contains("language") || !metadata["language"].is_string()) {
    m_Logger->error("Metadata must contain language field");
    return false;
  }

  if (!jsonData.contains("common") || !jsonData["common"].is_object()) {
    m_Logger->error("Localization file must contain common translations");
    return false;
  }

  return true;
}

bool UILocalizationJson::ParseLanguagePack(const json &jsonData, LanguagePack &pack) const
{
  try {
    const auto &metadata = jsonData["metadata"];
    pack.locale = metadata.value("locale", "");
    pack.displayName = metadata.value("display_name", "");

    std::string directionStr = metadata.value("direction", "LTR");
    pack.direction = (directionStr == "RTL") ? TextDirection::RTL : TextDirection::LTR;

    // 合并所有翻译项
    MergeTranslations(jsonData["common"], pack.translations, "common.");
    if (jsonData.contains("editor") && jsonData["editor"].is_object()) {
      MergeTranslations(jsonData["editor"], pack.translations, "editor.");
    }
    if (jsonData.contains("errors") && jsonData["errors"].is_object()) {
      MergeTranslations(jsonData["errors"], pack.translations, "errors.");
    }
    if (jsonData.contains("tooltips") && jsonData["tooltips"].is_object()) {
      MergeTranslations(jsonData["tooltips"], pack.translations, "tooltips.");
    }

    m_Logger->debug("Parsed language pack: {}, {} translations",
                    metadata.value("language", ""),
                    pack.translations.size());
    return true;
  }
  catch (const std::exception &e) {
    m_Logger->error("Failed to parse language pack: {}", e.what());
    return false;
  }
}

void UILocalizationJson::MergeTranslations(const json &source,
                                           std::unordered_map<std::string, std::string> &target,
                                           const std::string &prefix) const
{
  for (auto it = source.begin(); it != source.end(); ++it) {
    if (it.value().is_string()) {
      target[prefix + it.key()] = it.value();
    }
    else if (it.value().is_object()) {
      // 递归处理嵌套对象
      MergeTranslations(it.value(), target, prefix + it.key() + ".");
    }
  }
}

bool UILocalizationJson::SetCurrentLanguage(const std::string &languageCode)
{
  if (m_LanguagePacks.find(languageCode) == m_LanguagePacks.end()) {
    m_Logger->error("Language not available: {}", languageCode);
    return false;
  }

  std::string oldLanguage = m_CurrentLanguage;
  m_CurrentLanguage = languageCode;

  ImGuiFontManager::SetLanguageFont(languageCode);

  m_Logger->info("Language changed to: {}", languageCode);
  return true;
}

std::string UILocalizationJson::GetCurrentLanguage() const
{
  return m_CurrentLanguage;
}

std::vector<std::string> UILocalizationJson::GetAvailableLanguages() const
{
  std::vector<std::string> languages;
  for (const auto &pair : m_LanguagePacks) {
    languages.push_back(pair.first);
  }
  return languages;
}

std::string UILocalizationJson::Translate(const std::string &key) const
{
  if (m_CurrentLanguage.empty() || m_LanguagePacks.empty()) {
    return key;  // 返回键名作为默认值
  }

  const auto &languagePack = m_LanguagePacks.at(m_CurrentLanguage);
  auto it = languagePack.translations.find(key);
  if (it != languagePack.translations.end()) {
    return it->second;
  }

  // 如果当前语言找不到，尝试英文作为后备
  if (m_CurrentLanguage != ENGLISH && m_LanguagePacks.find(ENGLISH) != m_LanguagePacks.end()) {
    const auto &englishPack = m_LanguagePacks.at(ENGLISH);
    auto engIt = englishPack.translations.find(key);
    if (engIt != englishPack.translations.end()) {
      return engIt->second;
    }
  }

  //m_Logger->debug("Translation key not found: {}", key);
  return key;  // 无翻译，返回键名作为默认值
}



bool UILocalizationJson::IsRTLLanguage(const std::string &languageCode) const
{
  if (m_LanguagePacks.find(languageCode) != m_LanguagePacks.end()) {
    return m_LanguagePacks.at(languageCode).direction == TextDirection::RTL;
  }
  return false;
}

TextDirection UILocalizationJson::GetTextDirection() const
{
  if (m_CurrentLanguage.empty() ||
      m_LanguagePacks.find(m_CurrentLanguage) == m_LanguagePacks.end())
  {
    return TextDirection::LTR;
  }
  return m_LanguagePacks.at(m_CurrentLanguage).direction;
}

}  // namespace mite
