#include "ui_imgui_render.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

namespace mite {
// ==================== 面板管理接口实现 ====================
bool ImGuiUIRender::BeginPanel(PanelProps &props)
{
  if (!props.visible) {
    return false;
  }

  // 设定titie
  std::string titleText = GetTranslatedText(props);

  // 设置窗口尺寸约束
  ImGui::SetNextWindowSizeConstraints(ImVec2(props.minSize.x, props.minSize.y),
                                      ImVec2(props.maxSize.x, props.maxSize.y));

  // 构建ImGui窗口标志
  ImGuiWindowFlags flags = ImGuiWindowFlags_None;

  if (!props.movable)
    flags |= ImGuiWindowFlags_NoMove;
  if (!props.resizable)
    flags |= ImGuiWindowFlags_NoResize;
  if (!props.scrollable)
    flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
  if (props.collapsed)
    flags |= ImGuiWindowFlags_NoTitleBar;
  if (props.bringToFront)
    flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
  if (props.noBackground)
    flags |= ImGuiWindowFlags_NoBackground;
  if (props.hasMenuBar)
    flags |= ImGuiWindowFlags_MenuBar;
  if (!props.dockable)
    flags |= ImGuiWindowFlags_NoDocking;

  // 开始panel
  bool began = ImGui::Begin(titleText.c_str(), &props.visible, flags);

  if (began) {
    // 更新Focused信息和Hovered信息
    props.isFocused = IsPanelFocused();
    props.isHovered = IsPanelHovered();

    // 若窗口悬停且鼠标在可操作区域内，窗口不可移动（操作props，在下一帧实现）
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (ImGui::IsWindowHovered() &&
        ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max))
      props.movable = false;
    else
      // 否则释放窗口，可移动
      props.movable = true;
  }
  return began;
}
void ImGuiUIRender::EndPanel()
{
  ImGui::End();
}
bool ImGuiUIRender::BeginChild(ChildProps &props)
{
  if (!props.visible) {
    return false;
  }
  // 生成ID
  std::string childId = GetTranslatedText(props);
  if (childId.empty()) {
    childId = GenerateImGuiId(props.elementId);
  }
  // 计算实际尺寸
  ImVec2 childSize(props.size.x, props.size.y);
  if (props.autoResizeX)
    childSize.x = 0;
  if (props.autoResizeY)
    childSize.y = 0;

  // 构建子窗口标志
  ImGuiWindowFlags flags = ImGuiWindowFlags_None;
  if (!props.scrollable)
    flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
  if (props.noBackground)
    flags |= ImGuiWindowFlags_NoBackground;
  if (props.alwaysVerticalScrollbar)
    flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
  if (props.alwaysHorizontalScrollbar)
    flags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;

  // 开始子窗口
  bool began = ImGui::BeginChild(childId.c_str(), childSize, props.border, flags);

  if (began) {
    // 更新运行时状态
    props.isHovered = IsPanelHovered();
  }

  return began;
}
void ImGuiUIRender::EndChild()
{
  ImGui::EndChild();
}
glm::vec2 ImGuiUIRender::GetWindowPos()
{
  ImVec2 position = ImGui::GetWindowPos();
  return glm::vec2(position.x, position.y);
}
glm::vec2 ImGuiUIRender::GetPanelPos()
{
  ImVec2 position = ImGui::GetCursorScreenPos();
  return glm::vec2(position.x, position.y);
}
glm::vec2 ImGuiUIRender::GetWindowSize()
{
  ImVec2 size = ImGui::GetWindowSize();
  return glm::vec2(size.x, size.y);
}
glm::vec2 ImGuiUIRender::GetPanelSize()
{
  ImVec2 size = ImGui::GetContentRegionAvail();
  return glm::vec2(size.x, size.y);
}
bool ImGuiUIRender::IsPanelFocused()
{
  return ImGui::IsWindowFocused();
}
bool ImGuiUIRender::IsPanelHovered()
{
  return ImGui::IsWindowHovered();
}

// ==================== 基础控件渲染实现 ====================

void ImGuiUIRender::RenderLabel(const LabelProps &props)
{
  if (!props.visible)
    return;

  std::string displayText = GetTranslatedText(props);
  ImGui::Text("%s", displayText.c_str());
  SetItemTooltip(props.tooltip);
}

bool ImGuiUIRender::RenderButton(const ButtonProps &props)
{
  if (!props.visible)
    return false;

  std::string displayText = GetTranslatedText(props);
  bool changed = ImGui::Button(
      displayText.c_str(),
      ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y)));
  SetItemTooltip(props.tooltip);

  return changed;
}

bool ImGuiUIRender::RenderCheckbox(CheckboxProps &props)
{
  if (!props.visible)
    return false;

  std::string displayText = GetTranslatedText(props);
  bool changed = ImGui::Checkbox(GenerateImGuiId(props.elementId), &props.checked);

  if (!displayText.empty()) {
    ImGui::SameLine();
    ImGui::Text("%s", displayText.c_str());
  }
  SetItemTooltip(props.tooltip);
  return changed;
}

bool ImGuiUIRender::RenderTextInput(TextInputProps &props)
{
  if (!props.visible)
    return false;

  std::string hintText = GetTranslatedHint(props);
  std::string labelText = GetTranslatedText(props);

  bool changed = false;
  if (props.isPassword) {
    char buffer[256] = {0};
    std::copy_n(props.text.c_str(), std::min(props.text.size(), sizeof(buffer) - 1), buffer);
    // 确保以null结尾
    buffer[sizeof(buffer) - 1] = '\0';

    if (ImGui::InputTextWithHint(labelText.c_str(),
                                 hintText.c_str(),
                                 buffer,
                                 sizeof(buffer),
                                 ImGuiInputTextFlags_Password))
    {
      props.text = buffer;
      changed = true;
    }
  }
  else {
    changed = ImGui::InputTextWithHint(labelText.c_str(), hintText.c_str(), &props.text);
  }
  SetItemTooltip(props.tooltip);
  return changed;
}

bool ImGuiUIRender::RenderTextArea(TextAreaProps &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::InputTextMultiline(
      labelText.c_str(),
      &props.text,
      ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y)));

  SetItemTooltip(props.tooltip);

  return changed;
}

// ==================== 选择器控件渲染实现 ====================

bool ImGuiUIRender::RenderCombobox(ComboboxProps &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  bool changed = false;

  if (ImGui::BeginCombo(labelText.c_str(), props.previewText.c_str())) {
    for (int i = 0; i < props.items.size(); ++i) {
      std::string itemText = GetTranslatedItem(props.itemTranslationKeys, props.items, i);
      bool isSelected = (i == props.selectedIndex);
      if (ImGui::Selectable(itemText.c_str(), isSelected)) {
        props.selectedIndex = i;
        changed = true;
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  return changed;
}

bool ImGuiUIRender::RenderListBox(ListBoxProps &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  bool changed = false;

  if (ImGui::BeginListBox(
          labelText.c_str(),
          ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y))))
  {
    for (int i = 0; i < props.items.size(); ++i) {
      std::string itemText = GetTranslatedItem(props.itemTranslationKeys, props.items, i);
      bool isSelected = (i == props.selectedIndex);
      if (ImGui::Selectable(itemText.c_str(), isSelected)) {
        props.selectedIndex = i;
        changed = true;
      }
    }
    ImGui::EndListBox();
  }

  return changed;
}

// ==================== 数值输入控件渲染实现 ====================

bool ImGuiUIRender::RenderDragFloat(DragFloatProps &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::DragFloat(labelText.c_str(),
                          &props.value,
                          props.speed,
                          props.minValue,
                          props.maxValue,
                          props.format.c_str());
  SetItemTooltip(props.tooltip);
  return changed;
}

bool ImGuiUIRender::RenderDragFloat2(DragFloat2Props &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::DragFloat2(labelText.c_str(),
                           glm::value_ptr(props.value),
                           props.speed,
                           props.minValue,
                           props.maxValue,
                           props.format.c_str());
  SetItemTooltip(props.tooltip);
  return changed;
}

bool ImGuiUIRender::RenderDragFloat3(DragFloat3Props &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::DragFloat3(labelText.c_str(),
                           glm::value_ptr(props.value),
                           props.speed,
                           props.minValue,
                           props.maxValue,
                           props.format.c_str());
  SetItemTooltip(props.tooltip);
  return changed;
}

bool ImGuiUIRender::RenderDragFloat4(DragFloat4Props &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::DragFloat4(labelText.c_str(),
                           glm::value_ptr(props.value),
                           props.speed,
                           props.minValue,
                           props.maxValue,
                           props.format.c_str());
  SetItemTooltip(props.tooltip);
  return changed;
}

bool ImGuiUIRender::RenderDragInt(DragIntProps &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::DragInt(labelText.c_str(),
                        &props.value,
                        props.speed,
                        props.minValue,
                        props.maxValue,
                        props.format.c_str());
  SetItemTooltip(props.tooltip);
  return changed;
}

// ==================== 特殊控件渲染实现 ====================

void ImGuiUIRender::RenderProgressBar(const ProgressBarProps &props)
{
  if (!props.visible)
    return;

  std::string overlayText = GetTranslatedOverlay(props);
  ImGui::ProgressBar(props.progress,
                     ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y)),
                     overlayText.empty() ? nullptr : overlayText.c_str());
  SetItemTooltip(props.tooltip);
}

bool ImGuiUIRender::RenderColorEdit(ColorEditProps &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  ImGuiColorEditFlags flags = ImGuiColorEditFlags_None;

  // Alpha相关设置
  if (props.showAlpha) {
    flags |= ImGuiColorEditFlags_AlphaPreviewHalf;  // 半透明预览
    flags |= ImGuiColorEditFlags_AlphaBar;          // 显示alpha条
  }
  else {
    flags |= ImGuiColorEditFlags_NoAlpha;      // 禁用alpha编辑
    flags |= ImGuiColorEditFlags_AlphaOpaque;  // 预览中禁用alpha
  }

  // 输入框设置
  if (!props.showInputs) {
    flags |= ImGuiColorEditFlags_NoInputs;
  }

  // 选择器设置
  if (!props.showPicker) {
    flags |= ImGuiColorEditFlags_NoPicker;
  }

  // 预览设置
  if (!props.showPreview) {
    flags |= ImGuiColorEditFlags_NoSidePreview;
    flags |= ImGuiColorEditFlags_NoSmallPreview;
  }

  // 工具提示设置
  if (!props.showTooltip) {
    flags |= ImGuiColorEditFlags_NoTooltip;
  }

  bool changed = ImGui::ColorEdit4(labelText.c_str(), glm::value_ptr(props.color), flags);
  SetItemTooltip(props.tooltip);
  return changed;
}

void ImGuiUIRender::RenderImage(const ImageProps &props)
{
  if (!props.visible)
    return;

  ImGui::Image(props.textureId,
               ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y)),
               ImVec2(props.uv0.x, props.uv0.y),
               ImVec2(props.uv1.x, props.uv1.y));
}

// ==================== 容器控件渲染实现 ====================

void ImGuiUIRender::RenderGroup(const GroupProps &props,
                                const std::function<void()> &renderContent)
{
  if (!props.visible)
    return;

  std::string labelText = GetTranslatedText(props);

  if (!labelText.empty()) {
    ImGui::Text("%s", labelText.c_str());
    ImGui::Separator();
  }

  ImGui::BeginGroup();
  if (renderContent) {
    renderContent();
  }
  ImGui::EndGroup();
}

bool ImGuiUIRender::RenderTreeNode(TreeNodeProps &props,
                                   const std::function<void()> &renderContent)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
  if (props.isExpand)
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
  if (props.isLeaf)
    flags |= ImGuiTreeNodeFlags_Leaf;
  if (props.isSelect)
    flags |= ImGuiTreeNodeFlags_Selected;

  bool isOpen = ImGui::TreeNodeEx(
      GenerateImGuiId(props.elementId), flags, "%s", labelText.c_str());
  bool changed = (isOpen != props.isExpand);
  props.isExpand = isOpen;

  if (isOpen && renderContent) {
    renderContent();
    ImGui::TreePop();
  }
  SetItemTooltip(props.tooltip);
  return changed;
}

bool ImGuiUIRender::RenderPopup(PopupProps &props, const std::function<void()> &renderContent)
{
  if (!props.visible)
    return false;

  std::string titleText = GetTranslatedText(props);
  bool shouldKeepOpen = props.open;

  if (props.open) {
    if (props.modal) {
      ImGui::OpenPopup(titleText.c_str());
      if (ImGui::BeginPopupModal(titleText.c_str(), &props.open)) {
        if (renderContent)
          renderContent();
        ImGui::EndPopup();
      }
    }
    else {
      if (ImGui::BeginPopup(titleText.c_str())) {
        if (renderContent)
          renderContent();
        ImGui::EndPopup();
      }
    }
  }

  return shouldKeepOpen;
}

void ImGuiUIRender::RenderTable(TableProps &props, const std::function<void()> &renderContent)
{
  if (!props.visible)
    return;

  if (ImGui::BeginTable(GenerateImGuiId(props.elementId),
                        props.columns,
                        ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders))
  {
    // 渲染表头
    if (props.showHeaders) {
      for (int i = 0; i < props.columns; ++i) {
        std::string headerText = GetTranslatedHeader(props, i);
        ImGui::TableSetupColumn(headerText.c_str());
      }
      ImGui::TableHeadersRow();
    }

    // 渲染表格内容
    if (renderContent) {
      renderContent();
    }

    ImGui::EndTable();
  }
}

// ==================== 布局控件渲染实现 ====================

void ImGuiUIRender::RenderSeparator()
{
  ImGui::Separator();
}

void ImGuiUIRender::RenderSpacer(const SpacerProps &props)
{
  if (!props.visible)
    return;
  ImGui::Dummy(ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y)));
}

void ImGuiUIRender::SetSameLine(float offset, float spacing)
{
  ImGui::SameLine(offset, spacing);
}

void ImGuiUIRender::SetNewLine()
{
  ImGui::NewLine();
}

// ==================== 状态管理实现 ====================

void ImGuiUIRender::BeginDisabled(bool disabled)
{
  if (disabled) {
    ImGui::BeginDisabled();
  }
}

void ImGuiUIRender::EndDisabled()
{
  ImGui::EndDisabled();
}

// ==================== 工具函数实现 ====================

glm::vec2 ImGuiUIRender::GetCursorPos()
{
  ImVec2 pos = ImGui::GetCursorPos();
  return glm::vec2(pos.x, pos.y);
}

void ImGuiUIRender::SetCursorPos(const glm::vec2 &pos)
{
  ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
}

glm::vec2 ImGuiUIRender::CalcTextSize(const std::string &text)
{
  ImVec2 size = ImGui::CalcTextSize(text.c_str());
  return glm::vec2(size.x, size.y);
}

// ==================== 翻译辅助函数 ====================

std::string ImGuiUIRender::GetTranslatedText(const BaseRenderProps &props)
{
  if (!props.translationKey.empty()) {
    return UILocalization::Get().Translate(props.translationKey.c_str());
  }
  return props.fallbackText;
}

std::string ImGuiUIRender::GetTranslatedHint(const TextInputProps &props)
{
  if (!props.hintTranslationKey.empty()) {
    return UILocalization::Get().Translate(props.hintTranslationKey.c_str());
  }
  return props.hintFallbackText;
}

std::string ImGuiUIRender::GetTranslatedOverlay(const ProgressBarProps &props)
{
  if (!props.overlayTranslationKey.empty()) {
    return UILocalization::Get().Translate(props.overlayTranslationKey.c_str());
  }
  return props.overlayFallbackText;
}

std::string ImGuiUIRender::GetTranslatedItem(const std::vector<std::string> &translationKeys,
                                             const std::vector<std::string> &fallbackItems,
                                             int index)
{
  if (index < translationKeys.size() && !translationKeys[index].empty()) {
    return UILocalization::Get().Translate(translationKeys[index].c_str());
  }
  if (index < fallbackItems.size()) {
    return fallbackItems[index];
  }
  return "Unknown";
}

std::string ImGuiUIRender::GetTranslatedHeader(const TableProps &props, int columnIndex)
{
  if (columnIndex < props.headerTranslationKeys.size() &&
      !props.headerTranslationKeys[columnIndex].empty())
  {
    return UILocalization::Get().Translate(props.headerTranslationKeys[columnIndex].c_str());
  }
  if (columnIndex < props.headerFallbackTexts.size()) {
    return props.headerFallbackTexts[columnIndex];
  }
  return "Column";
}

// ==================== 私有辅助函数 ====================

void ImGuiUIRender::SetItemTooltip(std::string tooltip)
{
  ImGui::SetItemTooltip(tooltip.c_str());
}

const char *ImGuiUIRender::GenerateImGuiId(const UUID &elementId)
{
  thread_local static char buffer[64];
  snprintf(buffer, sizeof(buffer), "##%s", UUIDGenerator::UUIDToString(elementId).c_str());
  return buffer;
}
}  // namespace mite