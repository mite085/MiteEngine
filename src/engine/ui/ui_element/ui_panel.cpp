#include "ui_panel.h"

namespace mite {
UIPanel::UIPanel(const std::string &name,
                 UILayout::LayoutType layoutType,
                 UILayout::Alignment layoutAlignment)
    : UIWidget(name),
      m_Draggable(true),
      m_Resizable(true),
      m_LayoutDirty(false),
      m_Layout(UILayout::CreateUILayout(layoutType, layoutAlignment))
{
}

UIPanel::~UIPanel()
{
  ClearWidgets();
}

void UIPanel::Update(float deltaTime)
{
  // 更新所有子控件
  for (auto &widget : m_Widgets) {
    if (widget->IsVisible() && widget->IsEnabled()) {
      widget->Update(deltaTime);
    }
  }

  // 检查并应用布局
  if (m_LayoutDirty) {
    ApplyLayout();
    m_LayoutDirty = false;
  }

  // 面板自身的更新逻辑
  UIWidget::Update(deltaTime);
}

void UIPanel::Render() {
  if (!IsVisible())
    return;

  // 渲染所有子控件
  for (const auto &widget : m_Widgets) {
    if (widget->IsVisible()) {
      widget->Render();
    }
  }
}

void UIPanel::AddWidget(std::shared_ptr<UIWidget> widget)
{
  if (!widget) {
    LOG_WARN("Attempted to add null widget to panel: {}", m_Name);
    return;
  }

  UUID widgetId = widget->GetID();
  if (m_WidgetMap.find(widgetId) != m_WidgetMap.end()) {
    LOG_WARN("Widget with ID {} already exists in panel: {}",
             UUIDGenerator::UUIDToString(widgetId),
             m_Name);
    return;
  }

  m_Widgets.push_back(widget);
  m_WidgetMap[widgetId] = widget;

  MarkLayoutDirty();

  LOG_DEBUG("Added widget {} to panel: {}", widget->GetName(), m_Name);
}

void UIPanel::RemoveWidget(UUID widgetId)
{
  auto it = m_WidgetMap.find(widgetId);
  if (it == m_WidgetMap.end()) {
    LOG_WARN(
        "Widget with ID {} not found in panel: {}", UUIDGenerator::UUIDToString(widgetId), m_Name);
    return;
  }

  // 从vector中移除
  auto widgetIt = std::remove_if(
      m_Widgets.begin(), m_Widgets.end(), [widgetId](const std::shared_ptr<UIWidget> &widget) {
        return widget->GetID() == widgetId;
      });

  if (widgetIt != m_Widgets.end()) {
    m_Widgets.erase(widgetIt, m_Widgets.end());
  }

  // 从map中移除
  m_WidgetMap.erase(it);

  LOG_DEBUG("Removed widget ID {} from panel: {}", UUIDGenerator::UUIDToString(widgetId), m_Name);
}

std::shared_ptr<UIWidget> UIPanel::GetWidget(UUID widgetId) const
{
  auto it = m_WidgetMap.find(widgetId);
  if (it != m_WidgetMap.end()) {
    return it->second;
  }
  return nullptr;
}

const std::vector<std::shared_ptr<UIWidget>> &UIPanel::GetWidgets() const
{
  return m_Widgets;
}

void UIPanel::ClearWidgets()
{
  m_Widgets.clear();
  m_WidgetMap.clear();

  MarkLayoutDirty();

  LOG_DEBUG("Cleared all widgets from panel: {}", m_Name);
}

void UIPanel::SetLayout(std::shared_ptr<UILayout> layout)
{
  if (m_Layout != layout) {
    m_Layout = layout;
    LOG_DEBUG("Set layout for panel {}: {}", m_Name, layout ? layout->GetLayoutType() : "None");
    MarkLayoutDirty();
  }
}
std::shared_ptr<UILayout> UIPanel::GetLayout() const
{
  return m_Layout;
}
void UIPanel::ApplyLayout()
{
  if (!m_Layout) {
    LOG_WARN("No layout set for panel: {}", m_Name);
    return;
  }
  if (m_Widgets.empty()) {
    LOG_DEBUG("No widgets to layout in panel: {}", m_Name);
    return;
  }
  try {
    // 收集需要布局的可见控件
    std::vector<std::shared_ptr<UIElement>> visibleElements;
    for (const auto &widget : m_Widgets) {
      if (widget->IsVisible()) {
        visibleElements.push_back(widget);
      }
    }
    if (visibleElements.empty()) {
      return;
    }

    // 计算布局位置
    auto positions = m_Layout->CalculateLayout(visibleElements, m_Size, m_Position);

    // 应用布局结果到控件
    size_t visibleIndex = 0;
    for (size_t i = 0; i < m_Widgets.size(); ++i) {
      if (!m_Widgets[i]->IsVisible()) {
        continue;
      }
      if (visibleIndex < positions.size()) {
        m_Widgets[i]->SetPosition(positions[visibleIndex]);
        visibleIndex++;
      }
    }
    LOG_DEBUG("Applied {} layout to panel: {} ({} widgets)",
              m_Layout->GetLayoutType(),
              m_Name,
              visibleElements.size());
  }
  catch (const std::exception &e) {
    LOG_ERROR("Failed to apply layout to panel {}: {}", m_Name, e.what());
  }
}

void UIPanel::MarkLayoutDirty()
{
  m_LayoutDirty = true;
}

bool UIPanel::IsDraggable() const
{
  return m_Draggable;
}

void UIPanel::SetDraggable(bool draggable)
{
  m_Draggable = draggable;
}

bool UIPanel::IsResizable() const
{
  return m_Resizable;
}

void UIPanel::SetResizable(bool resizable)
{
  m_Resizable = resizable;
}
}  // namespace mite