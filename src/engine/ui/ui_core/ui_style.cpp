#include "ui_style.h"

namespace mite {

UIStyle::UIStyle(const std::string& name) : m_Name(name)
{
  // 构造函数保持简洁，不再包含默认样式配置
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

}  // namespace mite
