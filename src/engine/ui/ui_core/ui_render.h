#ifndef MITE_UI_RENDER_H
#define MITE_UI_RENDER_H

#include "ui_core/ui_render_props.h"

namespace mite {
/**
 * @brief UI渲染抽象接口（单例访问）
 * 定义与后端无关的UI渲染操作，所有具体实现由后端负责
 */
class UIRender {
 public:
  virtual ~UIRender() = default;

  // 单例访问
  static UIRender &Get();
  // ==================== 面板管理接口 ====================
  /**
   * @brief 开始面板渲染
   * @param props 面板属性
   * @return 是否成功开始渲染（面板可见且未关闭）
   */
  virtual bool BeginPanel(PanelProps &props) = 0;
  /**
   * @brief 结束面板渲染
   */
  virtual void EndPanel() = 0;
  /**
   * @brief 开始子窗口渲染
   * @param props 子窗口属性
   * @return 是否成功开始渲染
   */
  virtual bool BeginChild(ChildProps &props) = 0;
  /**
   * @brief 结束子窗口渲染
   */
  virtual void EndChild() = 0;
  // ==================== 面板状态查询 ====================
  /**
   * @brief 获取面板内容区域可用尺寸
   */
  virtual glm::vec2 GetContentRegionAvail() = 0;
  /**
   * @brief 获取面板是否聚焦
   */
  virtual bool IsPanelFocused() = 0;
  /**
   * @brief 获取面板是否悬停
   */
  virtual bool IsPanelHovered() = 0;

  // ==================== 基础控件渲染 ====================
  virtual void RenderLabel(const LabelProps &props) = 0;    // 文本显示
  virtual bool RenderButton(const ButtonProps &props) = 0;  // 按键
  virtual bool RenderCheckbox(CheckboxProps &props) = 0;    // 复选框（是否选中）
  virtual bool RenderTextInput(TextInputProps &props) = 0;  // 文本输入
  virtual bool RenderTextArea(TextAreaProps &props) = 0;    // 多行文本输入

  // ==================== 选择器控件渲染 ====================
  virtual bool RenderCombobox(ComboboxProps &props) = 0;  // 下拉选择框
  virtual bool RenderListBox(ListBoxProps &props) = 0;    // 列表框

  // ==================== 数值输入控件渲染 ====================
  virtual bool RenderDragFloat(DragFloatProps &props) = 0;    // 浮点数（支持拖动编辑）
  virtual bool RenderDragFloat2(DragFloat2Props &props) = 0;  // 二维向量（支持拖动编辑）
  virtual bool RenderDragFloat3(DragFloat3Props &props) = 0;  // 三维向量（支持拖动编辑）
  virtual bool RenderDragFloat4(DragFloat4Props &props) = 0;  // 四维向量（支持拖动编辑）
  virtual bool RenderDragInt(DragIntProps &props) = 0;        // 整数（支持拖动编辑）

  // ==================== 特殊控件渲染 ====================
  virtual void RenderProgressBar(const ProgressBarProps &props) = 0;  // 进度条
  virtual bool RenderColorEdit(ColorEditProps &props) = 0;            // 颜色选择器
  virtual void RenderImage(const ImageProps &props) = 0;              // 图像显示

  // ==================== 容器控件渲染 ====================
  virtual void RenderGroup(const GroupProps &props,
                           const std::function<void()> &renderContent) = 0;  // 分组控件
  virtual bool RenderTreeNode(TreeNodeProps &props,
                              const std::function<void()> &renderContent) = 0;  // 树节点
  virtual bool RenderPopup(
      PopupProps &props, const std::function<void()> &renderContent) = 0;  // 模态或非模态弹出窗口
  virtual void RenderTable(TableProps &props,
                           const std::function<void()> &renderContent) = 0;  // 表格

  // ==================== 布局控件渲染 ====================
  virtual void RenderSeparator() = 0;
  virtual void RenderSpacer(const SpacerProps &props) = 0;  // 布局spacing的空白间隔
  virtual void SetSameLine(float offset = 0.0f,
                           float spacing = -1.0f) = 0;  // 让下一个控件在同一行显示，而不是换行显示
  virtual void SetNewLine() = 0;  // 显式换行，确保下一个控件在新的一行显示

  // ==================== 状态管理 ====================
  // Imgui的立即模式与上下文系统支持Begin和End的便利性接口
  virtual void BeginDisabled(bool disabled = true) = 0;  // 启用Disable区域，区域内控件均不允许编辑
  virtual void EndDisabled() = 0;  // 终止Disable区域，后续的控件允许编辑

  // ==================== 工具函数 ====================
  virtual glm::vec2 GetCursorPos() = 0;                 // 获取当前光标位置
  virtual void SetCursorPos(const glm::vec2 &pos) = 0;  // 设置光标位置
  virtual glm::vec2 CalcTextSize(const std::string &text) = 0;  // 计算文本在当前字体下的渲染尺寸
};
}  // namespace mite

#endif  // MITE_UI_RENDER_H
