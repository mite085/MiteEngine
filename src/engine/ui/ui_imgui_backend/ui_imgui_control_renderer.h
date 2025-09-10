#ifndef MITE_IMGUI_CONTROL_RENDERER_H
#define MITE_IMGUI_CONTROL_RENDERER_H

#include "headers/headers.h"
#include <imgui.h>


namespace mite {

/**
 * @brief ImGui控件渲染器 - 负责非本地化控件的渲染封装
 * 
 * 包含了
 * 1. 基础控件：按钮
 * 2. 布局控件：分割线
 * 3. 分组控件：Group
 * 4. 进度控件：进度条
 * 5. 颜色控件：带颜色的按钮，颜色选择器
 * 6. 图像控件：图像，带图像的按钮
 * 7. 形状控件：长方形，圆形（支持填充颜色）
 *
 * 提供所有不需要文本翻译的ImGui控件渲染接口
 * 与ImGuiLocalizationRenderer互补，构成完整的控件渲染体系
 */
class ImGuiControlRenderer {
 public:
  // ==================== 基础控件 ====================

  /**
   * @brief 渲染按钮（无文本）
   */
  static bool Button(const glm::vec2 &size = glm::vec2(0, 0));

  /**
   * @brief 渲染带有图标的按钮
   */
  static bool ButtonWithIcon(const char *icon, const glm::vec2 &size = glm::vec2(0, 0));

  /**
   * @brief 渲染小按钮
   */
  static bool SmallButton(const char *label);

  /**
   * @brief 渲染箭头按钮
   */
  static bool ArrowButton(const char *str_id, ImGuiDir dir);

  // ==================== 布局控件 ====================

  /**
   * @brief 渲染分隔线
   */
  static void Separator();

  /**
   * @brief 渲染相同的行（接着上一个控件继续）
   */
  static void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f);

  /**
   * @brief 渲染换行
   */
  static void NewLine();

  /**
   * @brief 渲染间隔
   */
  static void Spacing();

  /**
   * @brief 渲染可调整的间隔
   */
  static void Dummy(const glm::vec2 &size);

  /**
   * @brief 开始缩进
   */
  static void Indent(float indent_w = 0.0f);

  /**
   * @brief 结束缩进
   */
  static void Unindent(float indent_w = 0.0f);

  // ==================== 分组控件 ====================

  /**
   * @brief 开始分组
   */
  static void BeginGroup();

  /**
   * @brief 结束分组
   */
  static void EndGroup();

  /**
   * @brief 开始子窗口
   */
  static bool BeginChild(const char *str_id,
                         const glm::vec2 &size = glm::vec2(0, 0),
                         bool border = false,
                         ImGuiWindowFlags flags = 0);

  /**
   * @brief 结束子窗口
   */
  static void EndChild();

  // ==================== 进度指示器 ====================

  /**
   * @brief 渲染进度条
   */
  static void ProgressBar(float fraction,
                          const glm::vec2 &size_arg = glm::vec2(-1, 0),
                          const char *overlay = nullptr);

  // ==================== 颜色控件 ====================

  /**
   * @brief 渲染颜色按钮
   */
  static bool ColorButton(const char *desc_id,
                          const ImVec4 &col,
                          ImGuiColorEditFlags flags = 0,
                          const glm::vec2 &size = glm::vec2(0, 0));

  /**
   * @brief 渲染颜色编辑器
   */
  static bool ColorEdit3(const char *label, float col[3], ImGuiColorEditFlags flags = 0);

  static bool ColorEdit4(const char *label, float col[4], ImGuiColorEditFlags flags = 0);

  /**
   * @brief 渲染颜色选择器
   */
  static bool ColorPicker3(const char *label, float col[3], ImGuiColorEditFlags flags = 0);

  static bool ColorPicker4(const char *label,
                           float col[4],
                           ImGuiColorEditFlags flags = 0,
                           const float *ref_col = nullptr);

  // ==================== 图像控件 ====================

  /**
   * @brief 渲染图像
   * 
   * (注意OpenGL UV坐标翻转)
   */
  static void Image(ImTextureID user_texture_id,
                    const glm::vec2 &size,
                    const glm::vec2 &uv0 = glm::vec2(0, 1),
                    const glm::vec2 &uv1 = glm::vec2(1, 0),
                    const ImVec4 &tint_col = ImVec4(1, 1, 1, 1),
                    const ImVec4 &border_col = ImVec4(0, 0, 0, 0));

  /**
   * @brief 渲染可点击的图像按钮
   */
  static bool ImageButton(const char *str_id,
                          ImTextureID user_texture_id,
                          const glm::vec2 &size,
                          const glm::vec2 &uv0,
                          const glm::vec2 &uv1,
                          const ImVec4 &bg_col,
                          const ImVec4 &tint_col);

  // ==================== 自定义形状 ====================

  /**
   * @brief 渲染矩形
   */
  static void DrawRect(const glm::vec2 &min,
                       const glm::vec2 &max,
                       ImU32 col,
                       float rounding = 0.0f,
                       ImDrawFlags flags = 0,
                       float thickness = 1.0f);

  /**
   * @brief 渲染填充矩形
   */
  static void DrawRectFilled(const glm::vec2 &min,
                             const glm::vec2 &max,
                             ImU32 col,
                             float rounding = 0.0f,
                             ImDrawFlags flags = 0);

  /**
   * @brief 渲染圆形
   */
  static void DrawCircle(const glm::vec2 &center,
                         float radius,
                         ImU32 col,
                         int num_segments = 0,
                         float thickness = 1.0f);

  /**
   * @brief 渲染填充圆形
   */
  static void DrawCircleFilled(const glm::vec2 &center,
                               float radius,
                               ImU32 col,
                               int num_segments = 0);

  // ==================== 树形控件 ====================

  /**
   * @brief 开始树节点
   */
  static bool TreeNode(const char *label, ImGuiTreeNodeFlags flags = 0);

  /**
   * @brief 开始树节点（带ID）
   */
  static bool TreeNodeEx(const char *str_id, ImGuiTreeNodeFlags flags, const char *fmt, ...);

  /**
   * @brief 结束树节点
   */
  static void TreePop();

  /**
   * @brief 设置下一个树节点是否打开
   */
  static void SetNextItemOpen(bool is_open, ImGuiCond cond = 0);

  // ==================== 表格控件 ====================

  /**
   * @brief 开始表格
   */
  static bool BeginTable(const char *str_id,
                         int column,
                         ImGuiTableFlags flags = 0,
                         const glm::vec2 &outer_size = glm::vec2(0, 0),
                         float inner_width = 0.0f);

  /**
   * @brief 结束表格
   */
  static void EndTable();

  /**
   * @brief 下一行
   */
  static void TableNextRow(ImGuiTableRowFlags flags = 0, float min_row_height = 0.0f);

  /**
   * @brief 下一列
   */
  static void TableNextColumn();

  /**
   * @brief 设置列索引
   */
  static void TableSetColumnIndex(int column_n);

  // ==================== 弹出窗口 ====================

  /**
   * @brief 开始弹出窗口
   */
  static void OpenPopup(const char *str_id, ImGuiPopupFlags popup_flags = 0);

  /**
   * @brief 开始弹出窗口（带ID）
   */
  static bool BeginPopup(const char *str_id, ImGuiWindowFlags flags = 0);

  /**
   * @brief 开始弹出上下文菜单
   */
  static bool BeginPopupContextItem(const char *str_id = NULL, ImGuiPopupFlags popup_flags = 1);

  /**
   * @brief 开始弹出上下文窗口
   */
  static bool BeginPopupContextWindow(const char *str_id = NULL, ImGuiPopupFlags popup_flags = 1);

  /**
   * @brief 开始弹出上下文窗口（无ID）
   */
  static bool BeginPopupContextVoid(const char *str_id = NULL, ImGuiPopupFlags popup_flags = 1);

  /**
   * @brief 结束弹出窗口
   */
  static void EndPopup();

  /**
   * @brief 关闭当前弹出窗口
   */
  static void CloseCurrentPopup();

  // ==================== 拖拽控件 ====================

  /**
   * @brief 渲染拖拽浮点数
   */
  static bool DragFloat(const char *label,
                        float *v,
                        float v_speed = 1.0f,
                        float v_min = 0.0f,
                        float v_max = 0.0f,
                        const char *format = "%.3f",
                        ImGuiSliderFlags flags = 0);

  static bool DragFloat2(const char *label,
                         float v[2],
                         float v_speed = 1.0f,
                         float v_min = 0.0f,
                         float v_max = 0.0f,
                         const char *format = "%.3f",
                         ImGuiSliderFlags flags = 0);

  static bool DragFloat3(const char *label,
                         float v[3],
                         float v_speed = 1.0f,
                         float v_min = 0.0f,
                         float v_max = 0.0f,
                         const char *format = "%.3f",
                         ImGuiSliderFlags flags = 0);

  static bool DragFloat4(const char *label,
                         float v[4],
                         float v_speed = 1.0f,
                         float v_min = 0.0f,
                         float v_max = 0.0f,
                         const char *format = "%.3f",
                         ImGuiSliderFlags flags = 0);

  /**
   * @brief 渲染拖拽整数
   */
  static bool DragInt(const char *label,
                      int *v,
                      float v_speed = 1.0f,
                      int v_min = 0,
                      int v_max = 0,
                      const char *format = "%d",
                      ImGuiSliderFlags flags = 0);

  static bool DragInt2(const char *label,
                       int v[2],
                       float v_speed = 1.0f,
                       int v_min = 0,
                       int v_max = 0,
                       const char *format = "%d",
                       ImGuiSliderFlags flags = 0);

  static bool DragInt3(const char *label,
                       int v[3],
                       float v_speed = 1.0f,
                       int v_min = 0,
                       int v_max = 0,
                       const char *format = "%d",
                       ImGuiSliderFlags flags = 0);

  static bool DragInt4(const char *label,
                       int v[4],
                       float v_speed = 1.0f,
                       int v_min = 0,
                       int v_max = 0,
                       const char *format = "%d",
                       ImGuiSliderFlags flags = 0);

  // ==================== 滑块控件 ====================

  /**
   * @brief 渲染滑块浮点数
   */
  static bool SliderFloat(const char *label,
                          float *v,
                          float v_min,
                          float v_max,
                          const char *format = "%.3f",
                          ImGuiSliderFlags flags = 0);

  static bool SliderFloat2(const char *label,
                           float v[2],
                           float v_min,
                           float v_max,
                           const char *format = "%.3f",
                           ImGuiSliderFlags flags = 0);

  static bool SliderFloat3(const char *label,
                           float v[3],
                           float v_min,
                           float v_max,
                           const char *format = "%.3f",
                           ImGuiSliderFlags flags = 0);

  static bool SliderFloat4(const char *label,
                           float v[4],
                           float v_min,
                           float v_max,
                           const char *format = "%.3f",
                           ImGuiSliderFlags flags = 0);

  /**
   * @brief 渲染滑块整数
   */
  static bool SliderInt(const char *label,
                        int *v,
                        int v_min,
                        int v_max,
                        const char *format = "%d",
                        ImGuiSliderFlags flags = 0);

  static bool SliderInt2(const char *label,
                         int v[2],
                         int v_min,
                         int v_max,
                         const char *format = "%d",
                         ImGuiSliderFlags flags = 0);

  static bool SliderInt3(const char *label,
                         int v[3],
                         int v_min,
                         int v_max,
                         const char *format = "%d",
                         ImGuiSliderFlags flags = 0);

  static bool SliderInt4(const char *label,
                         int v[4],
                         int v_min,
                         int v_max,
                         const char *format = "%d",
                         ImGuiSliderFlags flags = 0);

  /**
   * @brief 渲染角度滑块
   */
  static bool SliderAngle(const char *label,
                          float *v_rad,
                          float v_degrees_min = -360.0f,
                          float v_degrees_max = +360.0f,
                          const char *format = "%.0f deg",
                          ImGuiSliderFlags flags = 0);

  // ==================== 矢量控件 ====================

  /**
   * @brief 渲染2D矢量编辑器
   */
  static bool DragVector2(
      const char *label, glm::vec2 &value, float speed = 1.0f, float min = 0.0f, float max = 0.0f);

  /**
   * @brief 渲染3D矢量编辑器
   */
  static bool DragVector3(
      const char *label, glm::vec3 &value, float speed = 1.0f, float min = 0.0f, float max = 0.0f);

  /**
   * @brief 渲染4D矢量编辑器
   */
  static bool DragVector4(
      const char *label, glm::vec4 &value, float speed = 1.0f, float min = 0.0f, float max = 0.0f);

  // ==================== 工具函数 ====================

  /**
   * @brief 获取光标位置
   */
  static glm::vec2 GetCursorPos();

  /**
   * @brief 设置光标位置
   */
  static void SetCursorPos(const glm::vec2 &pos);

  /**
   * @brief 获取光标屏幕位置
   */
  static glm::vec2 GetCursorScreenPos();

  /**
   * @brief 设置光标屏幕位置
   */
  static void SetCursorScreenPos(const glm::vec2 &pos);

  /**
   * @brief 获取可用内容区域
   */
  static glm::vec2 GetContentRegionAvail();

  /**
   * @brief 获取窗口大小
   */
  static glm::vec2 GetWindowSize();

  /**
   * @brief 获取窗口位置
   */
  static glm::vec2 GetWindowPos();

  /**
   * @brief 获取窗口内容区域最小位置
   */
  static glm::vec2 GetWindowContentRegionMin();

  /**
   * @brief 获取窗口内容区域最大位置
   */
  static glm::vec2 GetWindowContentRegionMax();

  /**
   * @brief 获取文本尺寸
   */
  static glm::vec2 CalcTextSize(const char *text,
                                const char *text_end = NULL,
                                bool hide_text_after_double_hash = false,
                                float wrap_width = -1.0f);
};

// 宏定义简化调用
#define IMGUI_BUTTON_ICON(icon) mite::ImGuiControlRenderer::ButtonWithIcon(icon)
#define IMGUI_SEPARATOR() mite::ImGuiControlRenderer::Separator()
#define IMGUI_SAME_LINE() mite::ImGuiControlRenderer::SameLine()
#define IMGUI_NEW_LINE() mite::ImGuiControlRenderer::NewLine()
#define IMGUI_SPACING() mite::ImGuiControlRenderer::Spacing()
#define IMGUI_DUMMY(size) mite::ImGuiControlRenderer::Dummy(size)
#define IMGUI_IMAGE(tex, size) mite::ImGuiControlRenderer::Image(tex, size)
#define IMGUI_PROGRESS_BAR(frac) mite::ImGuiControlRenderer::ProgressBar(frac)

}  // namespace mite

#endif  // MITE_IMGUI_CONTROL_RENDERER_H
