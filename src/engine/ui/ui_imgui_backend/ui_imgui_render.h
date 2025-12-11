#ifndef MITE_UI_IMGUI_RENDER_H
#define MITE_UI_IMGUI_RENDER_H

#include "ui_core/ui_localization.h"
#include "ui_core/ui_render.h"

namespace mite {
/**
 * @brief ImGui UI渲染实现
 * 负责将抽象的UI渲染调用转换为具体的ImGui调用
 */
class ImGuiUIRender : public UIRender {
 public:
  ImGuiUIRender() = default;
  ~ImGuiUIRender() override = default;
  // ==================== 菜单渲染接口实现 ====================
  bool BeginMenuBar(const MenuBarProps &props) override;
  void EndMenuBar() override;
  bool RenderMenuItem(MenuItemProps &props) override;
  // ==================== 面板管理接口实现 ====================
  bool BeginPanel(PanelProps &props) override;
  void EndPanel() override;
  bool BeginChild(ChildProps &props) override;
  void EndChild() override;
  glm::vec2 GetWindowPos() override;
  glm::vec2 GetPanelPos() override;
  glm::vec2 GetWindowSize() override;
  glm::vec2 GetPanelSize() override;
  bool IsPanelFocused() override;
  bool IsPanelHovered() override;
  // ==================== 基础控件渲染 ====================
  void RenderLabel(const LabelProps &props) override;
  void RenderLabelSprator(const LabelProps &props) override;
  bool RenderButton(const ButtonProps &props) override;
  bool RenderCheckbox(CheckboxProps &props) override;
  bool RenderTextInput(TextInputProps &props) override;
  bool RenderTextArea(TextAreaProps &props) override;

  // ==================== 选择器控件渲染 ====================
  bool RenderCombobox(ComboboxProps &props) override;
  bool RenderListBox(ListBoxProps &props) override;

  // ==================== 数值输入控件渲染 ====================
  void RenderReadOnlyInt(IntEditProps &props) override;
  void RenderReadOnlyFloat(FloatEditProps &props) override;
  void RenderReadOnlyFloat2(Float2EditProps &props) override;
  void RenderReadOnlyFloat3(Float3EditProps &props) override;
  void RenderReadOnlyFloat4(Float4EditProps &props) override;

  bool RenderSliderInt(IntEditProps &props) override;
  bool RenderSliderFloat(FloatEditProps &props) override;
  bool RenderSliderFloat2(Float2EditProps &props) override;
  bool RenderSliderFloat3(Float3EditProps &props) override;
  bool RenderSliderFloat4(Float4EditProps &props) override;
  
  bool RenderDragInt(IntEditProps &props) override;
  bool RenderDragFloat(FloatEditProps &props) override;
  bool RenderDragFloat2(Float2EditProps &props) override;
  bool RenderDragFloat3(Float3EditProps &props) override;
  bool RenderDragFloat4(Float4EditProps &props) override;
  

  // ==================== 特殊控件渲染 ====================
  void RenderProgressBar(const ProgressBarProps &props) override;
  bool RenderColorEdit(ColorEditProps &props) override;
  void RenderImage(const ImageProps &props) override;

  // ==================== 容器控件渲染 ====================
  void RenderGroup(const GroupProps &props, const std::function<void()> &renderContent) override;
  void RenderTreeNode(TreeNodeProps &props,
                      const std::function<void()> &itemSelectedContent,
                      const std::function<void(void *)> &dragDropTargetContent,
                      const std::function<void()> &subitemRenderContent) override;
  void RenderTreeVoid(const std::function<void(void *)> &dragDropTargetContent) override;
  bool RenderPopup(PopupProps &props, const std::function<void()> &renderContent) override;
  void RenderTable(TableProps &props, const std::function<void()> &renderContent) override;
  void TableNextRow() override;
  void TableNextColume()override;

  // ==================== 布局控件渲染 ====================
  void RenderSeparator() override;
  void RenderSpacer(const SpacerProps &props) override;
  void SetSameLine(float offset = 0.0f, float spacing = -1.0f) override;
  void SetNewLine() override;

  // ==================== 状态管理 ====================
  bool BeginDisabled(bool disabled = true) override;
  void EndDisabled() override;

  // ==================== 工具函数 ====================
  glm::vec2 GetCursorPos() override;
  void SetCursorPos(const glm::vec2 &pos) override;
  glm::vec2 CalcTextSize(const std::string &text) override;
  glm::vec2 GetMousePos() override;

 private:
  // ==================== 翻译辅助函数 ====================
  std::string GetTranslatedText(const BaseRenderProps &props);  // 获取翻译后的文本内容
  std::string GetTranslatedHint(const TextInputProps &props);  // 获取输入框提示文本的翻译
  std::string GetTranslatedOverlay(const ProgressBarProps &props);  // 获取进度条覆盖文本的翻译
  std::string GetTranslatedItem(const std::vector<std::string> &translationKeys,
                                int index);                                   // 选项翻译处理
  std::string GetTranslatedHeader(const TableProps &props, int columnIndex);  // 表头翻译处理

  // ==================== 私有辅助函数 ====================
  void SetItemTooltip(std::string tooltip);
};
}  // namespace mite

#endif  // MITE_UI_IMGUI_RENDER_H
