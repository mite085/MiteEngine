#include "ui_layout_vertical.h"

namespace mite {

std::vector<glm::vec2> UILayoutVertical::CalculateLayout(
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

  // 计算总高度（包括间距）和最大宽度
  float totalHeight = 0.0f;
  float maxElementWidth = 0.0f;

  for (const auto &element : elements) {
    if (!element->IsVisible()) {
      continue;
    }

    glm::vec2 elementSize = element->GetSize();
    totalHeight += elementSize.y;
    maxElementWidth = std::max(maxElementWidth, elementSize.x);

    // 最后一行不加间距
    if (&element != &elements.back()) {
      totalHeight += m_Spacing;
    }
  }

  // 计算起始位置
  float startY = containerPosition.y + m_Padding.y;
  float currentY = startY;

  // 垂直对齐调整
  switch (m_Alignment) {
    case Alignment::CenterLeft:
    case Alignment::Center:
    case Alignment::CenterRight:
      currentY += (availableHeight - totalHeight) / 2.0f;
      break;
    case Alignment::BottomLeft:
    case Alignment::BottomCenter:
    case Alignment::BottomRight:
      currentY += availableHeight - totalHeight;
      break;
    case Alignment::TopLeft:
    case Alignment::TopCenter:
    case Alignment::TopRight:
    default:  // Top alignments
      currentY = startY;
      break;
  }

  // 根据对齐方式计算X坐标
  float baseX = containerPosition.x + m_Padding.x;

  // 布局每个元素
  for (const auto &element : elements) {
    if (!element->IsVisible()) {
      positions.emplace_back(0, 0);  // 占位
      continue;
    }

    glm::vec2 elementSize = element->GetSize();
    float elementX = baseX;

    // 如果启用拉伸（且elementSize水平方向无法占满整个availableWidth），调整元素宽度
    if (m_StretchChildren && elementSize.x < availableWidth) {
      element->SetSize(glm::vec2(availableWidth, elementSize.y));
      elementSize = element->GetSize();

      // 计算水平对齐（宽度可能改变了）
      switch (m_Alignment) {
        case Alignment::TopCenter:
        case Alignment::Center:
        case Alignment::BottomCenter:
          elementX = containerPosition.x + m_Padding.x + (availableWidth - elementSize.x) / 2.0f;
          break;
        case Alignment::TopRight:
        case Alignment::CenterRight:
        case Alignment::BottomRight:
          elementX = containerPosition.x + m_Padding.x + availableWidth - elementSize.x;
          break;
        case Alignment::TopLeft:
        case Alignment::CenterLeft:
        case Alignment::BottomLeft:
        default:  // Left alignments
          elementX = containerPosition.x + m_Padding.x;
          break;
      }
    }
    else {
      // 水平对齐调整
      switch (m_Alignment) {
        case Alignment::TopCenter:
        case Alignment::Center:
        case Alignment::BottomCenter:
          elementX += (availableWidth - elementSize.x) / 2.0f;
          break;
        case Alignment::TopRight:
        case Alignment::CenterRight:
        case Alignment::BottomRight:
          elementX += availableWidth - elementSize.x;
          break;
        case Alignment::TopLeft:
        case Alignment::CenterLeft:
        case Alignment::BottomLeft:
        default:  // Left alignments
          elementX = containerPosition.x + m_Padding.x;
          break;
      }
    }

    positions.emplace_back(elementX, currentY);
    currentY += elementSize.y + m_Spacing;
  }

  LOG_DEBUG("Vertical layout calculated for {} elements", elements.size());
  return positions;
}

const char *UILayoutVertical::GetLayoutType() const
{
  return "Vertical";
}

std::shared_ptr<UILayout> UILayoutVertical::Clone() const
{
  auto clone = std::make_shared<UILayoutVertical>(m_Alignment);
  clone->m_Spacing = m_Spacing;
  clone->m_Padding = m_Padding;
  clone->m_StretchChildren = m_StretchChildren;
  clone->m_Alignment = m_Alignment;
  return clone;
}

}  // namespace mite
