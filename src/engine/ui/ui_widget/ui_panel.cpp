#include "ui_panel.h"

namespace mite {

UIPanel::UIPanel(const std::string &name)
    : UIWidget(name), m_Title("Panel"), m_Draggable(true), m_Resizable(false)
{
}

UIPanel::~UIPanel()
{
  ClearWidgets();
}

void UIPanel::AddWidget(std::shared_ptr<UIWidget> widget)
{
  if (!widget) {
    LOG_WARN("Attempted to add null widget to panel: {}", m_Name);
    return;
  }

  UUID widgetId = widget->GetID();
  if (m_WidgetMap.find(widgetId) != m_WidgetMap.end()) {
    LOG_WARN("Widget with ID {} already exists in panel: {}", UUIDGenerator::UUIDToString(widgetId), m_Name);
    return;
  }

  m_Widgets.push_back(widget);
  m_WidgetMap[widgetId] = widget;

  LOG_DEBUG("Added widget {} to panel: {}", widget->GetName(), m_Name);
}

void UIPanel::RemoveWidget(UUID widgetId)
{
  auto it = m_WidgetMap.find(widgetId);
  if (it == m_WidgetMap.end()) {
    LOG_WARN("Widget with ID {} not found in panel: {}", UUIDGenerator::UUIDToString(widgetId), m_Name);
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

void UIPanel::ClearWidgets()
{
  m_Widgets.clear();
  m_WidgetMap.clear();
  LOG_DEBUG("Cleared all widgets from panel: {}", m_Name);
}

void UIPanel::Update(float deltaTime)
{
  // 更新所有子控件
  for (auto &widget : m_Widgets) {
    if (widget->IsVisible() && widget->IsEnabled()) {
      widget->Update(deltaTime);
    }
  }

  // 面板自身的更新逻辑
  UIWidget::Update(deltaTime);
}

void UIPanel::Render()
{
  if (!m_Visible) {
    return;
  }

  // TODO: 渲染面板背景和边框
  // 这里会调用具体的后端渲染实现

  // 渲染所有子控件
  for (auto &widget : m_Widgets) {
    if (widget->IsVisible()) {
      widget->Render();
    }
  }
}

void UIPanel::CalculateLayout()
{
  // 基础布局计算实现
  // 子类可以重写此方法实现特定的布局算法
  // TODO: 使用ui_layout.h实现布局

  glm::vec2 currentPos = m_Position;
  float maxWidth = 0.0f;
  float totalHeight = 0.0f;

  for (auto &widget : m_Widgets) {
    if (!widget->IsVisible()) {
      continue;
    }

    // 简单垂直布局
    widget->SetPosition(currentPos);
    currentPos.y += widget->GetSize().y + 5.0f;  // 5px间距

    maxWidth = std::max(maxWidth, widget->GetSize().x);
    totalHeight += widget->GetSize().y + 5.0f;
  }

  // 更新面板尺寸以适应内容
  if (!m_Widgets.empty()) {
    m_Size = glm::vec2(maxWidth, totalHeight - 5.0f);  // 减去最后一个间距
  }

  LOG_DEBUG("Calculated layout for panel: {}, size: {}x{}", m_Name, m_Size.x, m_Size.y);
}

}  // namespace mite
