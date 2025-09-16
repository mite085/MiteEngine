#include "ui_imgui_render.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace mite {
// ==================== 基础控件渲染实现 ====================

void ImGuiUIRender::RenderLabel(const LabelProps &props)
{
  if (!props.visible)
    return;

  std::string displayText = GetTranslatedText(props);
  ImGui::Text("%s", displayText.c_str());
}

bool ImGuiUIRender::RenderButton(const ButtonProps &props)
{
  if (!props.visible)
    return false;

  std::string displayText = GetTranslatedText(props);
  return ImGui::Button(displayText.c_str(), ImVec2(props.size.x, props.size.y));
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

  return changed;
}

bool ImGuiUIRender::RenderToggle(ToggleProps &props)
{
  if (!props.visible)
    return false;

  // IMGUI没有原生的Toggle开关控件
  // 通过绘制按钮和滑块实现该功能
  std::string labelText = GetTranslatedText(props);
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  ImGuiStyle &style = ImGui::GetStyle();

  // 计算布局
  ImVec2 pos = ImGui::GetCursorScreenPos();
  float height = ImGui::GetFrameHeight();
  float width = height * 1.8f;
  float radius = height * 0.5f;

  // 创建不可见按钮用于交互
  ImGui::InvisibleButton(GenerateImGuiId(props.elementId), ImVec2(width, height));
  bool isHovered = ImGui::IsItemHovered();
  bool isClicked = ImGui::IsItemClicked();

  if (isClicked) {
    props.value = !props.value;
  }

  // 计算颜色
  ImU32 bg_color;
  ImU32 circle_color;

  if (!props.enabled) {
    bg_color = ImGui::GetColorU32(ImGuiCol_FrameBg);
    circle_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
  }
  else if (isHovered) {
    bg_color = props.value ? IM_COL32(0, 135, 255, 255) : IM_COL32(100, 100, 100, 255);
    circle_color = IM_COL32(255, 255, 255, 255);
  }
  else {
    bg_color = props.value ? IM_COL32(0, 120, 240, 255) : IM_COL32(70, 70, 70, 255);
    circle_color = IM_COL32(240, 240, 240, 255);
  }

  // 绘制背景
  draw_list->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), bg_color, height * 0.5f);

  // 绘制圆形滑块
  float circle_x = props.value ? (pos.x + width - radius) : (pos.x + radius);
  draw_list->AddCircleFilled(ImVec2(circle_x, pos.y + radius), radius - 1.5f, circle_color);

  // 绘制标签
  if (!labelText.empty()) {
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (height - ImGui::GetTextLineHeight()) * 0.5f);
    ImGui::Text("%s", labelText.c_str());
  }

  return isClicked;
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

  return changed;
}

bool ImGuiUIRender::RenderTextArea(TextAreaProps &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  return ImGui::InputTextMultiline(
      labelText.c_str(), &props.text, ImVec2(props.size.x, props.size.y));
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

  if (ImGui::BeginListBox(labelText.c_str(), ImVec2(props.size.x, props.size.y))) {
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
  return ImGui::DragFloat(labelText.c_str(),
                          &props.value,
                          props.speed,
                          props.minValue,
                          props.maxValue,
                          props.format.c_str());
}

bool ImGuiUIRender::RenderDragFloat2(DragFloat2Props &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  return ImGui::DragFloat2(labelText.c_str(),
                           glm::value_ptr(props.value),
                           props.speed,
                           props.minValue,
                           props.maxValue,
                           props.format.c_str());
}

bool ImGuiUIRender::RenderDragFloat3(DragFloat3Props &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  return ImGui::DragFloat3(labelText.c_str(),
                           glm::value_ptr(props.value),
                           props.speed,
                           props.minValue,
                           props.maxValue,
                           props.format.c_str());
}

bool ImGuiUIRender::RenderDragFloat4(DragFloat4Props &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  return ImGui::DragFloat4(labelText.c_str(),
                           glm::value_ptr(props.value),
                           props.speed,
                           props.minValue,
                           props.maxValue,
                           props.format.c_str());
}

bool ImGuiUIRender::RenderDragInt(DragIntProps &props)
{
  if (!props.visible)
    return false;

  std::string labelText = GetTranslatedText(props);
  return ImGui::DragInt(labelText.c_str(),
                        &props.value,
                        props.speed,
                        props.minValue,
                        props.maxValue,
                        props.format.c_str());
}

// ==================== 特殊控件渲染实现 ====================

void ImGuiUIRender::RenderProgressBar(const ProgressBarProps &props)
{
  if (!props.visible)
    return;

  std::string overlayText = GetTranslatedOverlay(props);
  ImGui::ProgressBar(props.progress,
                     ImVec2(props.size.x, props.size.y),
                     overlayText.empty() ? nullptr : overlayText.c_str());
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

  return ImGui::ColorEdit4(labelText.c_str(), glm::value_ptr(props.color), flags);
}

void ImGuiUIRender::RenderImage(const ImageProps &props)
{
  if (!props.visible)
    return;

  ImGui::Image(props.textureId,
               ImVec2(props.size.x, props.size.y),
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
  if (props.isOpen)
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
  if (!props.hasChildren)
    flags |= ImGuiTreeNodeFlags_Leaf;

  bool isOpen = ImGui::TreeNodeEx(
      GenerateImGuiId(props.elementId), flags, "%s", labelText.c_str());
  bool changed = (isOpen != props.isOpen);
  props.isOpen = isOpen;

  if (isOpen && renderContent) {
    renderContent();
    ImGui::TreePop();
  }

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
  ImGui::Dummy(ImVec2(props.size.x, props.size.y));
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

const char *ImGuiUIRender::GenerateImGuiId(const UUID &elementId)
{
  thread_local static char buffer[64];
  snprintf(buffer, sizeof(buffer), "##%s", UUIDGenerator::UUIDToString(elementId).c_str());
  return buffer;
}

}  // namespace mite
