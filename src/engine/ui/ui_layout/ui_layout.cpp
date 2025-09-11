#include "ui_layout.h"
#include "ui_layout_horizontal.h"
#include "ui_layout_vertical.h"

namespace mite {
std::shared_ptr<UILayout> UILayout::CreateUILayout(LayoutType type, Alignment alignment)
{
  switch (type) {
    case mite::UILayout::LayoutType::Horizontal:
      return std::make_shared<UILayoutHorizontal>(alignment);
    case mite::UILayout::LayoutType::Vertical:
      return std::make_shared<UILayoutVertical>(alignment);
    default:
      return nullptr;
  }
  
}
void UILayout::SetSpacing(float spacing)
{
  if (m_Spacing != spacing) {
    m_Spacing = spacing;
    LOG_DEBUG("Layout spacing set to: {}", spacing);
  }
}

float UILayout::GetSpacing() const
{
  return m_Spacing;
}

void UILayout::SetPadding(const glm::vec4 &padding)
{
  if (m_Padding != padding) {
    m_Padding = padding;
    LOG_DEBUG(
        "Layout padding set to: ({}, {}, {}, {})", padding.x, padding.y, padding.z, padding.w);
  }
}

glm::vec4 UILayout::GetPadding() const
{
  return m_Padding;
}

void UILayout::SetStretchChildren(bool stretch)
{
  if (m_StretchChildren != stretch) {
    m_StretchChildren = stretch;
    LOG_DEBUG("Layout stretch children set to: {}", stretch);
  }
}

bool UILayout::GetStretchChildren() const
{
  return m_StretchChildren;
}

void UILayout::SetAlignment(UILayout::Alignment alignment)
{
  if (m_Alignment != alignment) {
    m_Alignment = alignment;
    LOG_DEBUG("UI layout alignment set to: {}", static_cast<int>(alignment));
  }
}

UILayout::Alignment UILayout::GetAlignment() const
{
  return m_Alignment;
}

}  // namespace mite
