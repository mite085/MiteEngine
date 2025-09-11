#include "ui_layout_horizontal.h"

namespace mite {

std::vector<glm::vec2> UILayoutHorizontal::CalculateLayout(
    const std::vector<std::shared_ptr<UIElement>> &elements,
    const glm::vec2 &containerSize,
    const glm::vec2 &containerPosition)
{
  std::vector<glm::vec2> positions;
  positions.reserve(elements.size());

  if (elements.empty()) {
    return positions;
  }

  // 计算可用空间（考虑边距）
  float availableWidth = containerSize.x - m_Padding.x - m_Padding.z;
  float availableHeight = containerSize.y - m_Padding.y - m_Padding.w;

  // 计算总宽度（包括间距）和最大高度
  float totalWidth = 0.0f;
  float maxElementHeight = 0.0f;

  for (const auto &element : elements) {
    if (!element->IsVisible()) {
      continue;
    }

    glm::vec2 elementSize = element->GetSize();
    totalWidth += elementSize.x;
    maxElementHeight = std::max(maxElementHeight, elementSize.y);

    // 最后一行不加间距
    if (&element != &elements.back()) {
      totalWidth += m_Spacing;
    }
  }

  // 计算起始位置
  float startX = containerPosition.x + m_Padding.x;
  float currentX = startX;

  // 水平对齐调整
  switch (m_Alignment) {
    case Alignment::TopCenter:
    case Alignment::Center:
    case Alignment::BottomCenter:
      currentX += (availableWidth - totalWidth) / 2.0f;
      break;
    case Alignment::TopRight:
    case Alignment::CenterRight:
    case Alignment::BottomRight:
      currentX += availableWidth - totalWidth;
      break;
    case Alignment::TopLeft:
    case Alignment::CenterLeft:
    case Alignment::BottomLeft:
    default:  // Left alignments
      currentX = startX;
      break;
  }

  // 根据对齐方式计算Y坐标
  float baseY = containerPosition.y + m_Padding.y;

  // 布局每个元素
  for (const auto &element : elements) {
    if (!element->IsVisible()) {
      positions.emplace_back(0, 0);  // 占位
      continue;
    }

    glm::vec2 elementSize = element->GetSize();
    float elementY = baseY;

    // 如果启用拉伸，自动调整元素高度
    if (m_StretchChildren && elementSize.y < availableHeight) {
      element->SetSize(glm::vec2(elementSize.x, availableHeight));
      elementSize = element->GetSize();

      // 计算垂直对齐（高度可能改变了）
      switch (m_Alignment) {
        case Alignment::CenterLeft:
        case Alignment::Center:
        case Alignment::CenterRight:
          elementY = containerPosition.y + m_Padding.y + (availableHeight - elementSize.y) / 2.0f;
          break;
        case Alignment::BottomLeft:
        case Alignment::BottomCenter:
        case Alignment::BottomRight:
          elementY = containerPosition.y + m_Padding.y + availableHeight - elementSize.y;
          break;
        case Alignment::TopLeft:
        case Alignment::TopCenter:
        case Alignment::TopRight:
        default:  // Top alignments
          elementY = containerPosition.y + m_Padding.y;
          break;
      }
    }
    else {
      // 根据对齐方式调整Y坐标
      switch (m_Alignment) {
        case Alignment::CenterLeft:
        case Alignment::Center:
        case Alignment::CenterRight:
          elementY = baseY + (availableHeight - elementSize.y) / 2.0f;
          break;
        case Alignment::BottomLeft:
        case Alignment::BottomCenter:
        case Alignment::BottomRight:
          elementY = baseY + availableHeight - elementSize.y;
          break;
        case Alignment::TopLeft:
        case Alignment::TopCenter:
        case Alignment::TopRight:
        default:  // Top alignments
          elementY = baseY;
          break;
      }
    }

    positions.emplace_back(currentX, elementY);
    currentX += elementSize.x + m_Spacing;
  }

  LOG_DEBUG("Horizontal layout calculated for {} elements", elements.size());
  return positions;
}

const char *UILayoutHorizontal::GetLayoutType() const
{
  return "Horizontal";
}

std::shared_ptr<UILayout> UILayoutHorizontal::Clone() const
{
  auto clone = std::make_shared<UILayoutHorizontal>(m_Alignment);
  clone->m_Spacing = m_Spacing;
  clone->m_Padding = m_Padding;
  clone->m_StretchChildren = m_StretchChildren;
  clone->m_Alignment = m_Alignment;
  return clone;
}

}  // namespace mite
