#include "ui_style.h"

namespace mite {

UIStyle::UIStyle() : m_Name("UnnamedStyle")
{
  ApplyDefaultStyle();
}

UIStyle::~UIStyle()
{
  Clear();
}

void UIStyle::SetProperty(const std::string &propertyName,
                          const StyleValue &value,
                          const std::string &description)
{
  m_Properties[propertyName] = {value, description};
}

bool UIStyle::HasProperty(const std::string &propertyName) const
{
  if (m_Properties.find(propertyName) != m_Properties.end()) {
    return true;
  }

  // 检查父样式
  if (m_Parent) {
    return m_Parent->HasProperty(propertyName);
  }

  return false;
}

void UIStyle::RemoveProperty(const std::string &propertyName)
{
  m_Properties.erase(propertyName);
}

void UIStyle::Clear()
{
  m_Properties.clear();
}

void UIStyle::Merge(const UIStyle &other, bool overwrite)
{
  for (const auto &[name, property] : other.m_Properties) {
    if (overwrite || m_Properties.find(name) == m_Properties.end()) {
      m_Properties[name] = property;
    }
  }
}

std::vector<std::string> UIStyle::GetPropertyNames() const
{
  std::vector<std::string> names;
  names.reserve(m_Properties.size());

  for (const auto &[name, _] : m_Properties) {
    names.push_back(name);
  }

  return names;
}

size_t UIStyle::GetPropertyCount() const
{
  return m_Properties.size();
}

void UIStyle::ApplyDefaultStyle()
{
  // 设置默认颜色
  SetProperty(
      StyleProperties::COLOR_BACKGROUND, glm::vec4(0.15f, 0.15f, 0.15f, 1.0f), "Background color");
  SetProperty(StyleProperties::COLOR_TEXT, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "Text color");
  SetProperty(StyleProperties::COLOR_BORDER, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), "Border color");
  SetProperty(StyleProperties::COLOR_HOVER, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), "Hover color");
  SetProperty(StyleProperties::COLOR_ACTIVE, glm::vec4(0.2f, 0.5f, 0.8f, 1.0f), "Active color");

  // 设置默认尺寸
  SetProperty(StyleProperties::SIZE_WIDTH, 100.0f, "Default width");
  SetProperty(StyleProperties::SIZE_HEIGHT, 30.0f, "Default height");

  // 设置默认边框
  SetProperty(StyleProperties::BORDER_WIDTH, 1.0f, "Border width");
  SetProperty(StyleProperties::BORDER_RADIUS, 4.0f, "Border radius");

  // 设置默认间距
  SetProperty(StyleProperties::PADDING_LEFT, 8.0f, "Left padding");
  SetProperty(StyleProperties::PADDING_RIGHT, 8.0f, "Right padding");
  SetProperty(StyleProperties::PADDING_TOP, 4.0f, "Top padding");
  SetProperty(StyleProperties::PADDING_BOTTOM, 4.0f, "Bottom padding");

  // 设置默认字体
  SetProperty(StyleProperties::FONT_SIZE, 14.0f, "Font size");
  SetProperty(StyleProperties::FONT_FAMILY, std::string("Arial"), "Font family");
}

// 模板特化实现
template<>
int UIStyle::GetProperty<int>(const std::string &propertyName, const int &defaultValue) const
{
  auto it = m_Properties.find(propertyName);
  if (it != m_Properties.end()) {
    try {
      return std::get<int>(it->second.value);
    }
    catch (const std::bad_variant_access &) {
      LOG_WARN("Style property {} is not of type int", propertyName);
    }
  }

  // 检查父样式
  if (m_Parent) {
    return m_Parent->GetProperty<int>(propertyName, defaultValue);
  }

  return defaultValue;
}

template<>
float UIStyle::GetProperty<float>(const std::string &propertyName, const float &defaultValue) const
{
  auto it = m_Properties.find(propertyName);
  if (it != m_Properties.end()) {
    try {
      return std::get<float>(it->second.value);
    }
    catch (const std::bad_variant_access &) {
      LOG_WARN("Style property {} is not of type float", propertyName);
    }
  }

  if (m_Parent) {
    return m_Parent->GetProperty<float>(propertyName, defaultValue);
  }

  return defaultValue;
}

template<>
bool UIStyle::GetProperty<bool>(const std::string &propertyName, const bool &defaultValue) const
{
  auto it = m_Properties.find(propertyName);
  if (it != m_Properties.end()) {
    try {
      return std::get<bool>(it->second.value);
    }
    catch (const std::bad_variant_access &) {
      LOG_WARN("Style property {} is not of type bool", propertyName);
    }
  }

  if (m_Parent) {
    return m_Parent->GetProperty<bool>(propertyName, defaultValue);
  }

  return defaultValue;
}

template<>
glm::vec2 UIStyle::GetProperty<glm::vec2>(const std::string &propertyName,
                                          const glm::vec2 &defaultValue) const
{
  auto it = m_Properties.find(propertyName);
  if (it != m_Properties.end()) {
    try {
      return std::get<glm::vec2>(it->second.value);
    }
    catch (const std::bad_variant_access &) {
      LOG_WARN("Style property {} is not of type glm::vec2", propertyName);
    }
  }

  if (m_Parent) {
    return m_Parent->GetProperty<glm::vec2>(propertyName, defaultValue);
  }

  return defaultValue;
}

template<>
glm::vec3 UIStyle::GetProperty<glm::vec3>(const std::string &propertyName,
                                          const glm::vec3 &defaultValue) const
{
  auto it = m_Properties.find(propertyName);
  if (it != m_Properties.end()) {
    try {
      return std::get<glm::vec3>(it->second.value);
    }
    catch (const std::bad_variant_access &) {
      LOG_WARN("Style property {} is not of type glm::vec3", propertyName);
    }
  }

  if (m_Parent) {
    return m_Parent->GetProperty<glm::vec3>(propertyName, defaultValue);
  }

  return defaultValue;
}

template<>
glm::vec4 UIStyle::GetProperty<glm::vec4>(const std::string &propertyName,
                                          const glm::vec4 &defaultValue) const
{
  auto it = m_Properties.find(propertyName);
  if (it != m_Properties.end()) {
    try {
      return std::get<glm::vec4>(it->second.value);
    }
    catch (const std::bad_variant_access &) {
      LOG_WARN("Style property {} is not of type glm::vec4", propertyName);
    }
  }

  if (m_Parent) {
    return m_Parent->GetProperty<glm::vec4>(propertyName, defaultValue);
  }

  return defaultValue;
}

template<>
std::string UIStyle::GetProperty<std::string>(const std::string &propertyName,
                                              const std::string &defaultValue) const
{
  auto it = m_Properties.find(propertyName);
  if (it != m_Properties.end()) {
    try {
      return std::get<std::string>(it->second.value);
    }
    catch (const std::bad_variant_access &) {
      LOG_WARN("Style property {} is not of type string", propertyName);
    }
  }

  if (m_Parent) {
    return m_Parent->GetProperty<std::string>(propertyName, defaultValue);
  }

  return defaultValue;
}

}  // namespace mite
