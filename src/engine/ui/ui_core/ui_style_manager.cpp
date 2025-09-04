#include "ui_style_manager.h"
#include "ui_event/ui_events_interaction.h"

namespace mite {

UIStyleManager &UIStyleManager::Get()
{
  static UIStyleManager instance;
  return instance;
}

UIStyleManager::UIStyleManager() : m_CurrentStyleName("light")
{
  // 构造函数保持简单，初始化在Initialize()中进行
}

void UIStyleManager::Initialize()
{
  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite UI Style Manager");
  m_Logger->info("Initializing UI Style Manager");

  // 创建并注册内置样式
  CreateBuiltinStyles();

  // 设置默认样式为当前样式
  if (HasStyle("light")) {
    SetCurrentStyle("light");
    m_Logger->info("Light style set as current style");
  }
  else {
    m_Logger->error("Failed to find light style during initialization");
  }
}

bool UIStyleManager::RegisterStyle(const std::string &name, std::shared_ptr<UIStyle> style)
{
  if (name.empty()) {
    m_Logger->warn("Cannot register style with empty name");
    return false;
  }

  if (!style) {
    m_Logger->warn("Cannot register null style: {}", name);
    return false;
  }

  if (m_Styles.find(name) != m_Styles.end()) {
    m_Logger->warn("Style already registered: {}", name);
    return false;
  }

  // 设置样式名称
  style->SetName(name);

  // 注册样式
  m_Styles[name] = style;
  m_Logger->info("Style registered: {}", name);

  return true;
}

std::shared_ptr<UIStyle> UIStyleManager::GetStyle(const std::string &name) const
{
  auto it = m_Styles.find(name);
  if (it != m_Styles.end()) {
    return it->second;
  }

  m_Logger->warn("Style not found: {}", name);
  return nullptr;
}

std::shared_ptr<UIStyle> UIStyleManager::GetCurrentStyle() const
{
  return GetStyle(m_CurrentStyleName);
}

std::string UIStyleManager::GetCurrentStyleName() const
{
  return m_CurrentStyleName;
}

bool UIStyleManager::SetCurrentStyle(const std::string &name)
{
  if (!HasStyle(name)) {
    m_Logger->warn("Cannot set current style: style not found - {}", name);
    return false;
  }

  std::string oldStyleName = m_CurrentStyleName;
  m_CurrentStyleName = name;

  // 发布样式变更事件
  StyleChangedEvent event(name, true);
  EventBus::Publish<StyleChangedEvent>(event);

  m_Logger->info("Current style changed from {} to {}", oldStyleName, name);
  return true;
}

bool UIStyleManager::HasStyle(const std::string &name) const
{
  return m_Styles.find(name) != m_Styles.end();
}

std::vector<std::string> UIStyleManager::GetAllStyleNames() const
{
  std::vector<std::string> names;
  names.reserve(m_Styles.size());

  for (const auto &pair : m_Styles) {
    names.push_back(pair.first);
  }

  return names;
}

size_t UIStyleManager::GetStyleCount() const
{
  return m_Styles.size();
}

void UIStyleManager::CreateBuiltinStyles()
{
  // 注册暗色主题
  auto darkTheme = CreateDarkTheme();
  if (darkTheme) {
    RegisterStyle("dark", darkTheme);
  }

  // 注册亮色主题
  auto lightTheme = CreateLightTheme();
  if (lightTheme) {
    RegisterStyle("light", lightTheme);
  }

  m_Logger->info("Built-in styles created and registered");
}

std::shared_ptr<UIStyle> UIStyleManager::CreateDarkTheme()
{
  auto style = std::make_shared<UIStyle>();
  style->SetName("dark");

  // 暗色主题配置
  style->SetProperty(
      StyleProperties::COLOR_BACKGROUND, glm::vec4(0.15f, 0.15f, 0.15f, 1.0f), "dark background");
  style->SetProperty(
      StyleProperties::COLOR_TEXT, glm::vec4(0.9f, 0.9f, 0.9f, 1.0f), "dark text");
  style->SetProperty(StyleProperties::COLOR_BORDER, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), "dark border");
  style->SetProperty(
      StyleProperties::COLOR_HOVER, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f), "dark hover");
  style->SetProperty(
      StyleProperties::COLOR_ACTIVE, glm::vec4(0.2f, 0.5f, 0.8f, 1.0f), "dark active");

  return style;
}

std::shared_ptr<UIStyle> UIStyleManager::CreateLightTheme()
{
  auto style = std::make_shared<UIStyle>();
  style->SetName("light");

  // 亮色主题配置
  style->SetProperty(
      StyleProperties::COLOR_BACKGROUND, glm::vec4(0.95f, 0.95f, 0.95f, 1.0f), "light background");
  style->SetProperty(
      StyleProperties::COLOR_TEXT, glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), "light text");
  style->SetProperty(StyleProperties::COLOR_BORDER, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f), "light border");
  style->SetProperty(
      StyleProperties::COLOR_HOVER, glm::vec4(0.85f, 0.85f, 0.85f, 1.0f), "light hover");
  style->SetProperty(
      StyleProperties::COLOR_ACTIVE, glm::vec4(0.6f, 0.8f, 1.0f, 1.0f), "light active");

  return style;
}

}  // namespace mite
