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
  ImGui::PushID(&props);

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
  ImGui::PopID();
}
bool ImGuiUIRender::BeginChild(ChildProps &props)
{
  if (!props.visible) {
    return false;
  }
  // 使用 PushID 自动生成唯一标识符
  ImGui::PushID(&props);

  std::string childId = GetTranslatedText(props);
  if (childId.empty()) {
    childId = "##Child";
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
  ImGui::PopID();  // 与 BeginChild 中的 PushID 配对
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
  ImGui::PushID(&props);

  std::string displayText = GetTranslatedText(props);
  ImGui::Text("%s", displayText.c_str());

  ImGui::PopID();
}

void ImGuiUIRender::RenderLabelSprator(const LabelProps &props)
{
  if (!props.visible)
    return;
  ImGui::PushID(&props);

  std::string displayText = GetTranslatedText(props);
  ImGui::SeparatorText(displayText.c_str());

  ImGui::PopID();
}

bool ImGuiUIRender::RenderButton(const ButtonProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string displayText = GetTranslatedText(props);
  bool changed = ImGui::Button(
      displayText.c_str(),
      ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y)));

  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderCheckbox(CheckboxProps &props)
{
  if (!props.visible)
    return false;

  // 使用 PushID 自动生成唯一标识符
  ImGui::PushID(&props);

  std::string displayText = GetTranslatedText(props);
  bool changed = ImGui::Checkbox("##Checkbox", &props.checked);
  ImGui::PopID();

  if (!displayText.empty()) {
    ImGui::SameLine();
    ImGui::Text("%s", displayText.c_str());
  }
  return changed;
}

bool ImGuiUIRender::RenderTextInput(TextInputProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string hintText = GetTranslatedHint(props);
  std::string labelText = GetTranslatedText(props);

  bool changed = ImGui::InputTextWithHint(labelText.c_str(), hintText.c_str(), &props.text);

  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderTextArea(TextAreaProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::InputTextMultiline(
      labelText.c_str(),
      &props.text,
      ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y)));

  return changed;
}

// ==================== 选择器控件渲染实现 ====================

bool ImGuiUIRender::RenderCombobox(ComboboxProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = false;

  // 获取当前显示文本（确保selectedIndex合法）
  std::string previewText;
  if (props.selectedIndex >= 0 && props.selectedIndex < props.itemTranslationKeys.size()) {
    previewText = GetTranslatedItem(props.itemTranslationKeys, props.selectedIndex);
  }
  else {
    previewText = "";
  }

  if (ImGui::BeginCombo(labelText.c_str(), previewText.c_str())) {
    for (int i = 0; i < props.itemTranslationKeys.size(); ++i) {
      std::string itemText = GetTranslatedItem(props.itemTranslationKeys, i);
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

  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderListBox(ListBoxProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = false;

  if (ImGui::BeginListBox(
          labelText.c_str(),
          ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y))))
  {
    for (int i = 0; i < props.itemTranslationKeys.size(); ++i) {
      std::string itemText = GetTranslatedItem(props.itemTranslationKeys, i);
      bool isSelected = (i == props.selectedIndex);
      if (ImGui::Selectable(itemText.c_str(), isSelected)) {
        props.selectedIndex = i;
        changed = true;
      }
    }
    ImGui::EndListBox();
  }

  ImGui::PopID();
  return changed;
}

void ImGuiUIRender::RenderReadOnlyInt(IntEditProps &props)
{
  if (!props.visible)
    return;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);

  ImGui::LabelText(labelText.c_str(), props.format.c_str(), props.value);

  ImGui::PopID();
  return;
}

void ImGuiUIRender::RenderReadOnlyFloat(FloatEditProps &props)
{
  if (!props.visible)
    return;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  ImGui::LabelText(labelText.c_str(), props.format.c_str(), props.value);
  ImGui::PopID();
  return;
}

void ImGuiUIRender::RenderReadOnlyFloat2(Float2EditProps &props)
{
  if (!props.visible)
    return;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  ImGui::LabelText(labelText.c_str(),
                   (props.format + ",   " + props.format).c_str(),
                   props.value.x,
                   props.value.y);
  ImGui::PopID();
  return;
}

void ImGuiUIRender::RenderReadOnlyFloat3(Float3EditProps &props)
{
  if (!props.visible)
    return;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  ImGui::LabelText(labelText.c_str(),
                   (props.format + ",   " + props.format + ",   " + props.format).c_str(),
                   props.value.x,
                   props.value.y,
                   props.value.z);
  ImGui::PopID();
  return;
}

void ImGuiUIRender::RenderReadOnlyFloat4(Float4EditProps &props)
{
  if (!props.visible)
    return;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  ImGui::LabelText(
      labelText.c_str(),
      (props.format + ",   " + props.format + ",   " + props.format + ",   " + props.format)
          .c_str(),
      props.value.x,
      props.value.y,
      props.value.z,
      props.value.w);
  ImGui::PopID();
  return;
}

// ==================== 数值输入控件渲染实现 ====================
bool ImGuiUIRender::RenderSliderInt(IntEditProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::SliderInt(
      labelText.c_str(), &props.value, props.minValue, props.maxValue, props.format.c_str());
  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderSliderFloat(FloatEditProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::SliderFloat(
      labelText.c_str(), &props.value, props.minValue, props.maxValue, props.format.c_str());
  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderSliderFloat2(Float2EditProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::SliderFloat2(labelText.c_str(),
                                     glm::value_ptr(props.value),
                                     props.minValue,
                                     props.maxValue,
                                     props.format.c_str());
  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderSliderFloat3(Float3EditProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::SliderFloat3(labelText.c_str(),
                                     glm::value_ptr(props.value),
                                     props.minValue,
                                     props.maxValue,
                                     props.format.c_str());
  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderSliderFloat4(Float4EditProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::SliderFloat4(labelText.c_str(),
                                     glm::value_ptr(props.value),
                                     props.minValue,
                                     props.maxValue,
                                     props.format.c_str());
  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderDragInt(IntEditProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::DragInt(labelText.c_str(),
                                &props.value,
                                props.dragSpeed,
                                props.minValue,
                                props.maxValue,
                                props.format.c_str());
  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderDragFloat(FloatEditProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::DragFloat(labelText.c_str(),
                                  &props.value,
                                  props.dragSpeed,
                                  props.minValue,
                                  props.maxValue,
                                  props.format.c_str());
  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderDragFloat2(Float2EditProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::DragFloat2(labelText.c_str(),
                                   glm::value_ptr(props.value),
                                   props.dragSpeed,
                                   props.minValue,
                                   props.maxValue,
                                   props.format.c_str());
  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderDragFloat3(Float3EditProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::DragFloat3(labelText.c_str(),
                                   glm::value_ptr(props.value),
                                   props.dragSpeed,
                                   props.minValue,
                                   props.maxValue,
                                   props.format.c_str());
  ImGui::PopID();
  return changed;
}

bool ImGuiUIRender::RenderDragFloat4(Float4EditProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  bool changed = ImGui::DragFloat4(labelText.c_str(),
                                   glm::value_ptr(props.value),
                                   props.dragSpeed,
                                   props.minValue,
                                   props.maxValue,
                                   props.format.c_str());
  ImGui::PopID();
  return changed;
}

// ==================== 特殊控件渲染实现 ====================

void ImGuiUIRender::RenderProgressBar(const ProgressBarProps &props)
{
  if (!props.visible)
    return;
  ImGui::PushID(&props);

  std::string overlayText = GetTranslatedOverlay(props);
  ImGui::ProgressBar(props.progress,
                     ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y)),
                     overlayText.empty() ? nullptr : overlayText.c_str());

  ImGui::PopID();
}

bool ImGuiUIRender::RenderColorEdit(ColorEditProps &props)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

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

  ImGui::PopID();
  return changed;
}

void ImGuiUIRender::RenderImage(const ImageProps &props)
{
  if (!props.visible)
    return;
  ImGui::PushID(&props);

  ImGui::Image(props.textureId,
               ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y)),
               ImVec2(props.uv0.x, props.uv0.y),
               ImVec2(props.uv1.x, props.uv1.y));

  ImGui::PopID();
}

// ==================== 容器控件渲染实现 ====================

void ImGuiUIRender::RenderGroup(const GroupProps &props,
                                const std::function<void()> &renderContent)
{
  if (!props.visible)
    return;
  ImGui::PushID(&props);

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
  ImGui::PopID();
}

void ImGuiUIRender::RenderTreeNode(TreeNodeProps &props,
                                   const std::function<void()> &itemSelectedContent,
                                   const std::function<void(void *)> &dragDropTargetContent,
                                   const std::function<void()> &subitemRenderContent)
{
  if (!props.visible)
    return;
  ImGui::PushID(&props);

  std::string labelText = GetTranslatedText(props);
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
  flags |= ImGuiTreeNodeFlags_OpenOnArrow;
  flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick;     // imgui标准展开模式
  flags |= ImGuiTreeNodeFlags_NavLeftJumpsToParent;  // 左侧箭头（展开用）
  flags |= ImGuiTreeNodeFlags_SpanFullWidth;         // 占满宽度，以便更容易用鼠标操作
  flags |= ImGuiTreeNodeFlags_DrawLinesToNodes;      // 绘制层次辅助线
  flags |= ImGuiTreeNodeFlags_DefaultOpen;
  if (props.isLeaf)
    flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
  if (props.isSelect)
    flags |= ImGuiTreeNodeFlags_Selected;

  bool nodeOpen = ImGui::TreeNodeEx(labelText.c_str(), flags);

  // 处理选择逻辑
  if (ImGui::IsItemFocused()) {
    props.isSelect = true;
    itemSelectedContent();
  }

  // 拖拽源 - 允许拖动此节点
  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
    ImGui::SetDragDropPayload("TreeNode", &props, sizeof(TreeNodeProps));
    ImGui::Text("Reset Parent of %s", labelText.c_str());
    ImGui::EndDragDropSource();
  }

  // 拖拽目标 -
  // 允许成为其他节点的父级（必须在子节点渲染完成之后，否则处理drop会导致子节点多次渲染）
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("TreeNode")) {
      dragDropTargetContent(payload->Data);
    }
    ImGui::EndDragDropTarget();
  }

  // 执行子节点渲染
  if (nodeOpen && subitemRenderContent) {
    subitemRenderContent();
    ImGui::TreePop();
  }

  ImGui::PopID();
}

void ImGuiUIRender::RenderTreeVoid(const std::function<void(void *)> &dragDropTargetContent)
{
  ImGui::PushID(&dragDropTargetContent);

  // 在面板剩余空白区域添加拖拽目标
  ImVec2 avail_size = ImGui::GetContentRegionAvail();
  if (avail_size.y > 0) {
    // 不可见的按钮，用于接收拖拽目标
    ImGui::InvisibleButton("TreeNode", avail_size);

    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("TreeNode")) {
        dragDropTargetContent(payload->Data);
      }
      ImGui::EndDragDropTarget();
    }
  }

  ImGui::PopID();
}

bool ImGuiUIRender::RenderPopup(PopupProps &props, const std::function<void()> &renderContent)
{
  if (!props.visible)
    return false;
  ImGui::PushID(&props);

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

  ImGui::PopID();
  return shouldKeepOpen;
}

void ImGuiUIRender::RenderTable(TableProps &props, const std::function<void()> &renderContent)
{
  if (!props.visible)
    return;
  ImGui::PushID(&props);
  std::string labelText = GetTranslatedText(props);
  ImGuiTreeNodeFlags flags = ImGuiTableFlags_None;

  // 基础特性设置
  if (props.resizable)
    flags |= ImGuiTableFlags_Resizable;
  if (props.reorderable)
    flags |= ImGuiTableFlags_Reorderable;
  if (props.hideable)
    flags |= ImGuiTableFlags_Hideable;
  if (props.sortable)
    flags |= ImGuiTableFlags_Sortable;
  // 装饰选项设置
  if (props.rowBg)
    flags |= ImGuiTableFlags_RowBg;
  if (props.borders)
    flags |= ImGuiTableFlags_Borders;
  else {
    if (props.bordersInnerH)
      flags |= ImGuiTableFlags_BordersInnerH;
    if (props.bordersInnerV)
      flags |= ImGuiTableFlags_BordersInnerV;
    if (props.bordersOuterH)
      flags |= ImGuiTableFlags_BordersOuterH;
    if (props.bordersOuterV)
      flags |= ImGuiTableFlags_BordersOuterV;
  }
  // 尺寸策略设置
  switch (props.sizingPolicy) {
    case TableProps::SizingFixedFit:
      flags |= ImGuiTableFlags_SizingFixedFit;
      break;
    case TableProps::SizingFixedSame:
      flags |= ImGuiTableFlags_SizingFixedSame;
      break;
    case TableProps::SizingStretchProp:
      flags |= ImGuiTableFlags_SizingStretchProp;
      break;
    case TableProps::SizingStretchSame:
      flags |= ImGuiTableFlags_SizingStretchSame;
      break;
  }
  // 尺寸额外选项设置
  if (props.noHostExtendX)
    flags |= ImGuiTableFlags_NoHostExtendX;
  if (props.noHostExtendY)
    flags |= ImGuiTableFlags_NoHostExtendY;
  if (props.preciseWidths)
    flags |= ImGuiTableFlags_PreciseWidths;
  // 滚动选项设置
  if (props.scrollX)
    flags |= ImGuiTableFlags_ScrollX;
  if (props.scrollY)
    flags |= ImGuiTableFlags_ScrollY;
  // 填充选项设置
  if (props.padOuterX)
    flags |= ImGuiTableFlags_PadOuterX;
  if (props.noPadInnerX)
    flags |= ImGuiTableFlags_NoPadInnerX;

  if (ImGui::BeginTable(labelText.c_str(), props.columns, flags)) {
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

  ImGui::PopID();
}

void ImGuiUIRender::TableNextRow()
{
  ImGui::TableNextRow();
  ImGui::TableNextColumn();  // 开启新的一行时直接更新新的一列
}

void ImGuiUIRender::TableNextColume()
{
  ImGui::TableNextColumn();
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
  ImGui::PushID(&props);

  ImGui::Dummy(ImVec2(static_cast<float>(props.size.x), static_cast<float>(props.size.y)));

  ImGui::PopID();
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

bool ImGuiUIRender::BeginDisabled(bool disabled)
{
  if (disabled) {
    ImGui::BeginDisabled();
    return true;
  }
  return false;
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
glm::vec2 ImGuiUIRender::GetMousePos()
{
  ImVec2 pos = ImGui::GetMousePos();
  return glm::vec2(pos.x, pos.y);
}
// ==================== 翻译辅助函数 ====================

std::string ImGuiUIRender::GetTranslatedText(const BaseRenderProps &props)
{
  return UILocalization::Get().Translate(props.translationKey.c_str());
}

std::string ImGuiUIRender::GetTranslatedHint(const TextInputProps &props)
{
  return UILocalization::Get().Translate(props.hintTranslationKey.c_str());
}

std::string ImGuiUIRender::GetTranslatedOverlay(const ProgressBarProps &props)
{
  return UILocalization::Get().Translate(props.overlayTranslationKey.c_str());
}

std::string ImGuiUIRender::GetTranslatedItem(const std::vector<std::string> &translationKeys,
                                             int index)
{
  if (index < translationKeys.size() && !translationKeys[index].empty()) {
    return UILocalization::Get().Translate(translationKeys[index].c_str());
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
  return "";
}

// ==================== 私有辅助函数 ====================

void ImGuiUIRender::SetItemTooltip(std::string tooltip)
{
  ImGui::SetItemTooltip(tooltip.c_str());
}
}  // namespace mite