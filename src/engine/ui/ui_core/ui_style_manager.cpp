#include "ui_style_manager.h"

namespace mite {

UIStyleManager &UIStyleManager::Get()
{
  static UIStyleManager instance;
  return instance;
}

UIStyleManager::UIStyleManager() : m_CurrentStyleName("default")
{
  // 构造函数保持简单，初始化在Initialize()中进行
}

void UIStyleManager::Initialize()
{
  LOG_INFO("Initializing UI Style Manager");

  // 创建并注册内置样式
  CreateBuiltinStyles();

  // 设置默认样式为当前样式
  if (HasStyle("default")) {
    SetCurrentStyle("default");
    LOG_INFO("Default style set as current style");
  }
  else {
    LOG_ERROR("Failed to find default style during initialization");
  }
}

bool UIStyleManager::RegisterStyle(const std::string &name, std::shared_ptr<UIStyle> style)
{
  if (name.empty()) {
    LOG_WARN("Cannot register style with empty name");
    return false;
  }

  if (!style) {
    LOG_WARN("Cannot register null style: {}", name);
    return false;
  }

  if (m_Styles.find(name) != m_Styles.end()) {
    LOG_WARN("Style already registered: {}", name);
    return false;
  }

  // 设置样式名称
  style->SetName(name);

  // 注册样式
  m_Styles[name] = style;
  LOG_INFO("Style registered: {}", name);

  return true;
}

std::shared_ptr<UIStyle> UIStyleManager::GetStyle(const std::string &name) const
{
  auto it = m_Styles.find(name);
  if (it != m_Styles.end()) {
    return it->second;
  }

  LOG_WARN("Style not found: {}", name);
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
    LOG_WARN("Cannot set current style: style not found - {}", name);
    return false;
  }

  std::string oldStyleName = m_CurrentStyleName;
  m_CurrentStyleName = name;

  // 发布样式变更事件
  StyleChangedEvent event;
  event.styleName = name;
  event.isGlobalChange = true;
  EventBus::Publish<StyleChangedEvent>(event);

  LOG_INFO("Current style changed from {} to {}", oldStyleName, name);
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
  // 注册默认样式
  auto defaultStyle = CreateDefaultStyle();
  if (defaultStyle) {
    RegisterStyle("default", defaultStyle);
  }

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

  LOG_INFO("Built-in styles created and registered");
}

std::shared_ptr<UIStyle> UIStyleManager::CreateDefaultStyle()
{
  auto style = std::make_shared<UIStyle>();
  style->SetName("default");
  style->ApplyDefaultStyle();
  return style;
}

std::shared_ptr<UIStyle> UIStyleManager::CreateDarkTheme()
{
  auto style = std::make_shared<UIStyle>();
  style->SetName("dark");

  // 暗色主题配置
  style->SetProperty(
      StyleProperties::COLOR_BACKGROUND, glm::vec4(0.15f, 0.15f, 0.15f, 1.0f), "暗色背景");
  style->SetProperty(
      StyleProperties::COLOR_TEXT, glm::vec4(0.9f, 0.9f, 0.9f, 1.0f), "暗色主题文字");
  style->SetProperty(StyleProperties::COLOR_BORDER, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), "暗色边框");
  style->SetProperty(
      StyleProperties::COLOR_HOVER, glm::vec4(0.25f, 0.25f, 0.25f, 1.0f), "暗色悬停");
  style->SetProperty(
      StyleProperties::COLOR_ACTIVE, glm::vec4(0.2f, 0.5f, 0.8f, 1.0f), "暗色激活状态");

  return style;
}

std::shared_ptr<UIStyle> UIStyleManager::CreateLightTheme()
{
  auto style = std::make_shared<UIStyle>();
  style->SetName("light");

  // 亮色主题配置
  style->SetProperty(
      StyleProperties::COLOR_BACKGROUND, glm::vec4(0.95f, 0.95f, 0.95f, 1.0f), "亮色背景");
  style->SetProperty(
      StyleProperties::COLOR_TEXT, glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), "亮色主题文字");
  style->SetProperty(StyleProperties::COLOR_BORDER, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f), "亮色边框");
  style->SetProperty(
      StyleProperties::COLOR_HOVER, glm::vec4(0.85f, 0.85f, 0.85f, 1.0f), "亮色悬停");
  style->SetProperty(
      StyleProperties::COLOR_ACTIVE, glm::vec4(0.6f, 0.8f, 1.0f, 1.0f), "亮色激活状态");

  return style;
}

}  // namespace mite
