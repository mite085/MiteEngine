#include "ui_imgui_style_adapter.h"
#include "ui_event/ui_events_lifecycle.h"

namespace mite {
ImGuiStyleAdapter::ImGuiStyleAdapter()
{
  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite UI ImGui Style Adapter");
  m_Logger->info("Initializing ImGuiStyleAdapter");
}

void ImGuiStyleAdapter::Initialize()
{ 
  // 备份原始ImGui样式
  m_BackupStyle = ImGui::GetStyle();

  // 创建默认样式映射
  CreateDefaultStyleMappings();

  // 订阅样式改变事件
  m_EventSubscriptions.Subscribe<StyleChangedEvent>(BIND_DISPATCH_FN(OnStyleChanged));
}

void ImGuiStyleAdapter::Shutdown()
{
  m_Logger->info("Shutting down ImGuiStyleAdapter");
  // 恢复原始ImGui样式
  ImGui::GetStyle() = m_BackupStyle;
}

bool ImGuiStyleAdapter::ApplyUIStyle(const std::shared_ptr<UIStyle> uiStyle)
{
  if (!uiStyle) {
    m_Logger->error("Cannot apply null UIStyle");
    return false;
  }

  try {
    MapUIStyleToImGui(uiStyle);
    m_Logger->info("Applied UI style: {}", uiStyle->GetName());
    return true;
  }
  catch (const std::exception &e) {
    m_Logger->error("Failed to apply UI style {}: {}", uiStyle->GetName(), e.what());
    return false;
  }
}

void ImGuiStyleAdapter::MapUIStyleToImGui(const std::shared_ptr<UIStyle> &uiStyle)
{
  // 重置到默认样式
  ImGui::StyleColorsDark();

  // 映射各类属性
  MapColorProperties(uiStyle);
  MapEnumProperties(uiStyle);
  MapSizeProperties(uiStyle);
  MapBorderProperties(uiStyle);
  MapSpacingProperties(uiStyle);
}

void ImGuiStyleAdapter::MapColorProperties(const std::shared_ptr<UIStyle> &uiStyle)
{
  auto &style = ImGui::GetStyle();

  for (const auto &[propertyName, colorIndex] : m_ColorMappings) {
    if (uiStyle->HasProperty(propertyName)) {
      try {
        glm::vec4 color = uiStyle->GetProperty<glm::vec4>(propertyName);
        SetImGuiColor(colorIndex, color);
      }
      catch (const std::exception &e) {
        m_Logger->warn("Failed to map color property {}: {}", propertyName, e.what());
      }
    }
  }
}

void ImGuiStyleAdapter::MapSizeProperties(const std::shared_ptr<UIStyle> &uiStyle)
{
  auto &style = ImGui::GetStyle();

  for (const auto &[propertyName, setter] : m_SizeMappings) {
    if (uiStyle->HasProperty(propertyName)) {
      try {
        float value = uiStyle->GetProperty<float>(propertyName);
        setter(value);
      }
      catch (const std::exception &e) {
        m_Logger->warn("Failed to map size property {}: {}", propertyName, e.what());
      }
    }
  }
}

void ImGuiStyleAdapter::MapEnumProperties(const std::shared_ptr<UIStyle> &uiStyle)
{
  auto &style = ImGui::GetStyle();

  auto IntToImGuiDir = [](int value) {
    switch (value) {
      case 0:
        return ImGuiDir::ImGuiDir_Left;
      case 1:
        return ImGuiDir::ImGuiDir_Right;
      case 2:
        return ImGuiDir::ImGuiDir_Up;
      case 3:
        return ImGuiDir::ImGuiDir_Down;
      case -1:
        return ImGuiDir::ImGuiDir_None;
      default:
        LOG_WARN("Invalid ImGuiDir value: {}, defaulting to None", value);
        return ImGuiDir::ImGuiDir_None;
    }
  };

  for (const auto &[propertyName, setter] : m_EnumMappings) {
    if (uiStyle->HasProperty(propertyName)) {
      try {
        int enumValue = uiStyle->GetProperty<int>(propertyName);
        ImGuiDir dir = IntToImGuiDir(enumValue);
        setter(dir);
      }
      catch (const std::exception &e) {
        m_Logger->warn("Failed to map enum property {}: {}", propertyName, e.what());
      }
    }
  }
}

void ImGuiStyleAdapter::MapBorderProperties(const std::shared_ptr<UIStyle> &uiStyle)
{
  auto &style = ImGui::GetStyle();

  for (const auto &[propertyName, setter] : m_BorderMappings) {
    if (uiStyle->HasProperty(propertyName)) {
      try {
        float value = uiStyle->GetProperty<float>(propertyName);
        setter(value);
      }
      catch (const std::exception &e) {
        m_Logger->warn("Failed to map border property {}: {}", propertyName, e.what());
      }
    }
  }
}

void ImGuiStyleAdapter::MapSpacingProperties(const std::shared_ptr<UIStyle> &uiStyle)
{
  auto &style = ImGui::GetStyle();

  for (const auto &[propertyName, setter] : m_SpacingMappings) {
    if (uiStyle->HasProperty(propertyName)) {
      try {
        float value = uiStyle->GetProperty<float>(propertyName);
        setter(value);
      }
      catch (const std::exception &e) {
        m_Logger->warn("Failed to map spacing property {}: {}", propertyName, e.what());
      }
    }
  }
}

void ImGuiStyleAdapter::SetImGuiColor(ImGuiCol colorIndex, const glm::vec4 &color)
{
  ImGui::GetStyle().Colors[colorIndex] = ImVec4(color.r, color.g, color.b, color.a);
}

void ImGuiStyleAdapter::CreateDefaultStyleMappings()
{
  // 颜色映射
  m_ColorMappings = {
      {StyleProperties::COLOR_BACKGROUND, ImGuiCol_WindowBg},
      {StyleProperties::COLOR_TEXT, ImGuiCol_Text},
      {StyleProperties::COLOR_BORDER, ImGuiCol_Border},
      {StyleProperties::COLOR_HOVER, ImGuiCol_ButtonHovered},
      {StyleProperties::COLOR_ACTIVE, ImGuiCol_ButtonActive},
      {StyleProperties::COLOR_DISABLED, ImGuiCol_TextDisabled},
      {StyleProperties::COLOR_HEADER, ImGuiCol_Header},
      {StyleProperties::COLOR_HEADER_HOVER, ImGuiCol_HeaderHovered},
      {StyleProperties::COLOR_HEADER_ACTIVE, ImGuiCol_HeaderActive},
      {StyleProperties::COLOR_FRAME_BG, ImGuiCol_FrameBg},
      {StyleProperties::COLOR_FRAME_BG_HOVER, ImGuiCol_FrameBgHovered},
      {StyleProperties::COLOR_FRAME_BG_ACTIVE, ImGuiCol_FrameBgActive},
      {StyleProperties::COLOR_TITLE_BG, ImGuiCol_TitleBg},
      {StyleProperties::COLOR_TITLE_BG_ACTIVE, ImGuiCol_TitleBgActive},
      {StyleProperties::COLOR_TITLE_BG_COLLAPSED, ImGuiCol_TitleBgCollapsed},
      {StyleProperties::COLOR_MENU_BAR_BG, ImGuiCol_MenuBarBg},
      {StyleProperties::COLOR_SCROLLBAR_BG, ImGuiCol_ScrollbarBg},
      {StyleProperties::COLOR_SCROLLBAR_GRAB, ImGuiCol_ScrollbarGrab},
      {StyleProperties::COLOR_SCROLLBAR_GRAB_HOVER, ImGuiCol_ScrollbarGrabHovered},
      {StyleProperties::COLOR_SCROLLBAR_GRAB_ACTIVE, ImGuiCol_ScrollbarGrabActive},
      {StyleProperties::COLOR_CHECK_MARK, ImGuiCol_CheckMark},
      {StyleProperties::COLOR_SLIDER_GRAB, ImGuiCol_SliderGrab},
      {StyleProperties::COLOR_SLIDER_GRAB_ACTIVE, ImGuiCol_SliderGrabActive},
      {StyleProperties::COLOR_BUTTON, ImGuiCol_Button},
      {StyleProperties::COLOR_BUTTON_HOVER, ImGuiCol_ButtonHovered},
      {StyleProperties::COLOR_BUTTON_ACTIVE, ImGuiCol_ButtonActive},
      {StyleProperties::COLOR_SEPARATOR, ImGuiCol_Separator},
      {StyleProperties::COLOR_SEPARATOR_HOVER, ImGuiCol_SeparatorHovered},
      {StyleProperties::COLOR_SEPARATOR_ACTIVE, ImGuiCol_SeparatorActive},
      {StyleProperties::COLOR_RESIZE_GRIP, ImGuiCol_ResizeGrip},
      {StyleProperties::COLOR_RESIZE_GRIP_HOVER, ImGuiCol_ResizeGripHovered},
      {StyleProperties::COLOR_RESIZE_GRIP_ACTIVE, ImGuiCol_ResizeGripActive},
      {StyleProperties::COLOR_TAB, ImGuiCol_Tab},
      {StyleProperties::COLOR_TAB_HOVER, ImGuiCol_TabHovered},
      {StyleProperties::COLOR_TAB_ACTIVE, ImGuiCol_TabActive},
      {StyleProperties::COLOR_TAB_UNFOCUSED, ImGuiCol_TabUnfocused},
      {StyleProperties::COLOR_TAB_UNFOCUSED_ACTIVE, ImGuiCol_TabUnfocusedActive},
      {StyleProperties::COLOR_PLOT_LINES, ImGuiCol_PlotLines},
      {StyleProperties::COLOR_PLOT_LINES_HOVER, ImGuiCol_PlotLinesHovered},
      {StyleProperties::COLOR_PLOT_HISTOGRAM, ImGuiCol_PlotHistogram},
      {StyleProperties::COLOR_PLOT_HISTOGRAM_HOVER, ImGuiCol_PlotHistogramHovered},
      {StyleProperties::COLOR_TABLE_HEADER_BG, ImGuiCol_TableHeaderBg},
      {StyleProperties::COLOR_TABLE_BORDER_STRONG, ImGuiCol_TableBorderStrong},
      {StyleProperties::COLOR_TABLE_BORDER_LIGHT, ImGuiCol_TableBorderLight},
      {StyleProperties::COLOR_TABLE_ROW_BG, ImGuiCol_TableRowBg},
      {StyleProperties::COLOR_TABLE_ROW_BG_ALT, ImGuiCol_TableRowBgAlt},
      {StyleProperties::COLOR_TEXT_SELECTED_BG, ImGuiCol_TextSelectedBg},
      {StyleProperties::COLOR_DRAG_DROP_TARGET, ImGuiCol_DragDropTarget},
      {StyleProperties::COLOR_NAV_HIGHLIGHT, ImGuiCol_NavHighlight},
      {StyleProperties::COLOR_NAV_WINDOWING_HIGHLIGHT, ImGuiCol_NavWindowingHighlight},
      {StyleProperties::COLOR_NAV_WINDOWING_DIM_BG, ImGuiCol_NavWindowingDimBg},
      {StyleProperties::COLOR_MODAL_WINDOW_DIM_BG, ImGuiCol_ModalWindowDimBg}};

  // 尺寸映射
  ImGuiStyle &style = ImGui::GetStyle();
  m_SizeMappings = {
      {StyleProperties::SIZE_WINDOW_PADDING_X, [&](float v) { style.WindowPadding.x = v; }},
      {StyleProperties::SIZE_WINDOW_PADDING_Y, [&](float v) { style.WindowPadding.y = v; }},
      {StyleProperties::SIZE_WINDOW_ROUNDING, [&](float v) { style.WindowRounding = v; }},
      {StyleProperties::SIZE_WINDOW_MIN_SIZE_X, [&](float v) { style.WindowMinSize.x = v; }},
      {StyleProperties::SIZE_WINDOW_MIN_SIZE_Y, [&](float v) { style.WindowMinSize.y = v; }},
      {StyleProperties::SIZE_WINDOW_TITLE_ALIGN_X, [&](float v) { style.WindowTitleAlign.x = v; }},
      {StyleProperties::SIZE_WINDOW_TITLE_ALIGN_Y, [&](float v) { style.WindowTitleAlign.y = v; }},
      {StyleProperties::SIZE_CHILD_ROUNDING, [&](float v) { style.ChildRounding = v; }},
      {StyleProperties::SIZE_CHILD_BORDER_SIZE, [&](float v) { style.ChildBorderSize = v; }},
      {StyleProperties::SIZE_POPUP_ROUNDING, [&](float v) { style.PopupRounding = v; }},
      {StyleProperties::SIZE_POPUP_BORDER_SIZE, [&](float v) { style.PopupBorderSize = v; }},
      {StyleProperties::SIZE_FRAME_PADDING_X, [&](float v) { style.FramePadding.x = v; }},
      {StyleProperties::SIZE_FRAME_PADDING_Y, [&](float v) { style.FramePadding.y = v; }},
      {StyleProperties::SIZE_FRAME_ROUNDING, [&](float v) { style.FrameRounding = v; }},
      {StyleProperties::SIZE_FRAME_BORDER_SIZE, [&](float v) { style.FrameBorderSize = v; }},
      {StyleProperties::SIZE_ITEM_SPACING_X, [&](float v) { style.ItemSpacing.x = v; }},
      {StyleProperties::SIZE_ITEM_SPACING_Y, [&](float v) { style.ItemSpacing.y = v; }},
      {StyleProperties::SIZE_ITEM_INNER_SPACING_X, [&](float v) { style.ItemInnerSpacing.x = v; }},
      {StyleProperties::SIZE_ITEM_INNER_SPACING_Y, [&](float v) { style.ItemInnerSpacing.y = v; }},
      {StyleProperties::SIZE_CELL_PADDING_X, [&](float v) { style.CellPadding.x = v; }},
      {StyleProperties::SIZE_CELL_PADDING_Y, [&](float v) { style.CellPadding.y = v; }},
      {StyleProperties::SIZE_TOUCH_EXTRA_PADDING_X,
       [&](float v) { style.TouchExtraPadding.x = v; }},
      {StyleProperties::SIZE_TOUCH_EXTRA_PADDING_Y,
       [&](float v) { style.TouchExtraPadding.y = v; }},
      {StyleProperties::SIZE_INDENT_SPACING, [&](float v) { style.IndentSpacing = v; }},
      {StyleProperties::SIZE_COLUMNS_MIN_SPACING, [&](float v) { style.ColumnsMinSpacing = v; }},
      {StyleProperties::SIZE_SCROLLBAR_SIZE, [&](float v) { style.ScrollbarSize = v; }},
      {StyleProperties::SIZE_SCROLLBAR_ROUNDING, [&](float v) { style.ScrollbarRounding = v; }},
      {StyleProperties::SIZE_GRAB_MIN_SIZE, [&](float v) { style.GrabMinSize = v; }},
      {StyleProperties::SIZE_GRAB_ROUNDING, [&](float v) { style.GrabRounding = v; }},
      {StyleProperties::SIZE_LOG_SLIDER_DEADZONE, [&](float v) { style.LogSliderDeadzone = v; }},
      {StyleProperties::SIZE_TAB_ROUNDING, [&](float v) { style.TabRounding = v; }},
      {StyleProperties::SIZE_TAB_BORDER_SIZE, [&](float v) { style.TabBorderSize = v; }},
      {StyleProperties::SIZE_TAB_MIN_WIDTH_FOR_CLOSE_BUTTON,
       [&](float v) { style.TabCloseButtonMinWidthUnselected = v; }},
      {StyleProperties::SIZE_BUTTON_TEXT_ALIGN_X, [&](float v) { style.ButtonTextAlign.x = v; }},
      {StyleProperties::SIZE_BUTTON_TEXT_ALIGN_Y, [&](float v) { style.ButtonTextAlign.y = v; }},
      {StyleProperties::SIZE_SELECTABLE_TEXT_ALIGN_X,
       [&](float v) { style.SelectableTextAlign.x = v; }},
      {StyleProperties::SIZE_SELECTABLE_TEXT_ALIGN_Y,
       [&](float v) { style.SelectableTextAlign.y = v; }},
      {StyleProperties::SIZE_DISPLAY_WINDOW_PADDING_X,
       [&](float v) { style.DisplayWindowPadding.x = v; }},
      {StyleProperties::SIZE_DISPLAY_WINDOW_PADDING_Y,
       [&](float v) { style.DisplayWindowPadding.y = v; }},
      {StyleProperties::SIZE_DISPLAY_SAFE_AREA_PADDING_X,
       [&](float v) { style.DisplaySafeAreaPadding.x = v; }},
      {StyleProperties::SIZE_DISPLAY_SAFE_AREA_PADDING_Y,
       [&](float v) { style.DisplaySafeAreaPadding.y = v; }},
      {StyleProperties::SIZE_MOUSE_CURSOR_SCALE, [&](float v) { style.MouseCursorScale = v; }},
      {StyleProperties::SIZE_ANTI_ALIASED_LINES, [&](float v) { style.AntiAliasedLines = v; }},
      {StyleProperties::SIZE_ANTI_ALIASED_LINES_USE_TEX,
       [&](float v) { style.AntiAliasedLinesUseTex = v; }},
      {StyleProperties::SIZE_ANTI_ALIASED_FILL, [&](float v) { style.AntiAliasedFill = v; }},
      {StyleProperties::SIZE_CURVE_TESSELLATION_TOL,
       [&](float v) { style.CurveTessellationTol = v; }},
      {StyleProperties::SIZE_CIRCLE_TESSELLATION_MAX_ERROR,
       [&](float v) { style.CircleTessellationMaxError = v; }}};

  // 枚举映射
  m_EnumMappings = {{StyleProperties::LAYOUT_COLOR_BUTTON_POSITION,
                     [&](ImGuiDir dir) { style.ColorButtonPosition = dir; }},
                    {StyleProperties::LAYOUT_WINDOW_MENU_BUTTON_POSITION,
                     [&](ImGuiDir dir) { style.WindowMenuButtonPosition = dir; }}};

  // 边框映射
  m_BorderMappings = {
      {StyleProperties::BORDER_WINDOW, [&](float v) { style.WindowBorderSize = v; }},
      {StyleProperties::BORDER_CHILD, [&](float v) { style.ChildBorderSize = v; }},
      {StyleProperties::BORDER_POPUP, [&](float v) { style.PopupBorderSize = v; }},
      {StyleProperties::BORDER_FRAME, [&](float v) { style.FrameBorderSize = v; }},
      {StyleProperties::BORDER_TAB, [&](float v) { style.TabBorderSize = v; }}};

  // 间距映射
  m_SpacingMappings = {
      {StyleProperties::SPACING_WINDOW_PADDING_X, [&](float v) { style.WindowPadding.x = v; }},
      {StyleProperties::SPACING_WINDOW_PADDING_Y, [&](float v) { style.WindowPadding.y = v; }},
      {StyleProperties::SPACING_FRAME_PADDING_X, [&](float v) { style.FramePadding.x = v; }},
      {StyleProperties::SPACING_FRAME_PADDING_Y, [&](float v) { style.FramePadding.y = v; }},
      {StyleProperties::SPACING_ITEM_SPACING_X, [&](float v) { style.ItemSpacing.x = v; }},
      {StyleProperties::SPACING_ITEM_SPACING_Y, [&](float v) { style.ItemSpacing.y = v; }},
      {StyleProperties::SPACING_ITEM_INNER_SPACING_X,
       [&](float v) { style.ItemInnerSpacing.x = v; }},
      {StyleProperties::SPACING_ITEM_INNER_SPACING_Y,
       [&](float v) { style.ItemInnerSpacing.y = v; }},
      {StyleProperties::SPACING_CELL_PADDING_X, [&](float v) { style.CellPadding.x = v; }},
      {StyleProperties::SPACING_CELL_PADDING_Y, [&](float v) { style.CellPadding.y = v; }},
      {StyleProperties::SPACING_TOUCH_EXTRA_PADDING_X,
       [&](float v) { style.TouchExtraPadding.x = v; }},
      {StyleProperties::SPACING_TOUCH_EXTRA_PADDING_Y,
       [&](float v) { style.TouchExtraPadding.y = v; }},
      {StyleProperties::SPACING_INDENT_SPACING, [&](float v) { style.IndentSpacing = v; }},
      {StyleProperties::SPACING_COLUMNS_MIN_SPACING,
       [&](float v) { style.ColumnsMinSpacing = v; }}};
}

std::shared_ptr<UIStyle> ImGuiStyleAdapter::ExportToUIStyle(const std::string &styleName)
{
  auto style = std::make_shared<UIStyle>(styleName);

  const auto &imguiStyle = ImGui::GetStyle();

  // 导出颜色属性
  for (const auto &[propertyName, colorIndex] : m_ColorMappings) {
    const ImVec4 &color = imguiStyle.Colors[colorIndex];
    style->SetProperty(propertyName, glm::vec4(color.x, color.y, color.z, color.w));
  }

  // 导出尺寸属性
  style->SetProperty(StyleProperties::SIZE_WINDOW_PADDING_X, imguiStyle.WindowPadding.x);
  style->SetProperty(StyleProperties::SIZE_WINDOW_PADDING_Y, imguiStyle.WindowPadding.y);
  style->SetProperty(StyleProperties::SIZE_FRAME_PADDING_X, imguiStyle.FramePadding.x);
  style->SetProperty(StyleProperties::SIZE_FRAME_PADDING_Y, imguiStyle.FramePadding.y);
  style->SetProperty(StyleProperties::SIZE_ITEM_SPACING_X, imguiStyle.ItemSpacing.x);
  style->SetProperty(StyleProperties::SIZE_ITEM_SPACING_Y, imguiStyle.ItemSpacing.y);
  style->SetProperty(StyleProperties::SIZE_WINDOW_ROUNDING, imguiStyle.WindowRounding);
  style->SetProperty(StyleProperties::SIZE_FRAME_ROUNDING, imguiStyle.FrameRounding);
  // 导出边框属性
  style->SetProperty(StyleProperties::BORDER_WINDOW, imguiStyle.WindowBorderSize);
  style->SetProperty(StyleProperties::BORDER_FRAME, imguiStyle.FrameBorderSize);
  // 导出枚举属性
  style->SetProperty(StyleProperties::LAYOUT_COLOR_BUTTON_POSITION,
                     static_cast<int>(imguiStyle.ColorButtonPosition));
  style->SetProperty(StyleProperties::LAYOUT_WINDOW_MENU_BUTTON_POSITION,
                     static_cast<int>(imguiStyle.WindowMenuButtonPosition));
  m_Logger->info("Exported ImGui style to UIStyle: {}", styleName);
  return style;
}

ImGuiStyle &ImGuiStyleAdapter::GetImGuiStyle()
{
  return ImGui::GetStyle();
}

bool ImGuiStyleAdapter::OnStyleChanged(StyleChangedEvent& event)
{
  std::shared_ptr<UIStyle> style = event.GetUIStyle();
  ApplyUIStyle(style);

  // 标记事件已解决
  event.Handled();
  return event.handled;
}
}  // namespace mite