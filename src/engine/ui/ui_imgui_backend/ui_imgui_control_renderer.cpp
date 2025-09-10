#include "ui_imgui_control_renderer.h"
#include <glm/gtc/type_ptr.hpp>

namespace mite {

// ==================== 基础控件实现 ====================

bool ImGuiControlRenderer::Button(const glm::vec2 &size)
{
  return ImGui::Button("", ImVec2(size.x, size.y));
}

bool ImGuiControlRenderer::ButtonWithIcon(const char *icon, const glm::vec2 &size)
{
  return ImGui::Button(icon, ImVec2(size.x, size.y));
}

bool ImGuiControlRenderer::SmallButton(const char *label)
{
  return ImGui::SmallButton(label);
}

bool ImGuiControlRenderer::ArrowButton(const char *str_id, ImGuiDir dir)
{
  return ImGui::ArrowButton(str_id, dir);
}

// ==================== 布局控件实现 ====================

void ImGuiControlRenderer::Separator()
{
  ImGui::Separator();
}

void ImGuiControlRenderer::SameLine(float offset_from_start_x, float spacing)
{
  ImGui::SameLine(offset_from_start_x, spacing);
}

void ImGuiControlRenderer::NewLine()
{
  ImGui::NewLine();
}

void ImGuiControlRenderer::Spacing()
{
  ImGui::Spacing();
}

void ImGuiControlRenderer::Dummy(const glm::vec2 &size)
{
  ImGui::Dummy(ImVec2(size.x, size.y));
}

void ImGuiControlRenderer::Indent(float indent_w)
{
  ImGui::Indent(indent_w);
}

void ImGuiControlRenderer::Unindent(float indent_w)
{
  ImGui::Unindent(indent_w);
}

// ==================== 分组控件实现 ====================

void ImGuiControlRenderer::BeginGroup()
{
  ImGui::BeginGroup();
}

void ImGuiControlRenderer::EndGroup()
{
  ImGui::EndGroup();
}

bool ImGuiControlRenderer::BeginChild(const char *str_id,
                                      const glm::vec2 &size,
                                      bool border,
                                      ImGuiWindowFlags flags)
{
  return ImGui::BeginChild(str_id, ImVec2(size.x, size.y), border, flags);
}

void ImGuiControlRenderer::EndChild()
{
  ImGui::EndChild();
}

// ==================== 进度指示器实现 ====================

void ImGuiControlRenderer::ProgressBar(float fraction,
                                       const glm::vec2 &size_arg,
                                       const char *overlay)
{
  ImGui::ProgressBar(fraction, ImVec2(size_arg.x, size_arg.y), overlay);
}


// ==================== 颜色控件实现 ====================

bool ImGuiControlRenderer::ColorButton(const char *desc_id,
                                       const ImVec4 &col,
                                       ImGuiColorEditFlags flags,
                                       const glm::vec2 &size)
{
  return ImGui::ColorButton(desc_id, col, flags, ImVec2(size.x, size.y));
}

bool ImGuiControlRenderer::ColorEdit3(const char *label, float col[3], ImGuiColorEditFlags flags)
{
  return ImGui::ColorEdit3(label, col, flags);
}

bool ImGuiControlRenderer::ColorEdit4(const char *label, float col[4], ImGuiColorEditFlags flags)
{
  return ImGui::ColorEdit4(label, col, flags);
}

bool ImGuiControlRenderer::ColorPicker3(const char *label, float col[3], ImGuiColorEditFlags flags)
{
  return ImGui::ColorPicker3(label, col, flags);
}

bool ImGuiControlRenderer::ColorPicker4(const char *label,
                                        float col[4],
                                        ImGuiColorEditFlags flags,
                                        const float *ref_col)
{
  return ImGui::ColorPicker4(label, col, flags, ref_col);
}

// ==================== 图像控件实现 ====================

void ImGuiControlRenderer::Image(ImTextureID user_texture_id,
                                 const glm::vec2 &size,
                                 const glm::vec2 &uv0,
                                 const glm::vec2 &uv1,
                                 const ImVec4 &tint_col,
                                 const ImVec4 &border_col)
{
  ImGui::Image(user_texture_id,
               ImVec2(size.x, size.y),
               ImVec2(uv0.x, uv0.y),
               ImVec2(uv1.x, uv1.y),
               tint_col,
               border_col);
}

bool ImGuiControlRenderer::ImageButton(const char *str_id,
                                       ImTextureID user_texture_id,
                                       const glm::vec2 &size,
                                       const glm::vec2 &uv0,
                                       const glm::vec2 &uv1,
                                       const ImVec4 &bg_col,
                                       const ImVec4 &tint_col)
{
  return ImGui::ImageButton(str_id,
                            user_texture_id,
                            ImVec2(size.x, size.y),
                            ImVec2(uv0.x, uv0.y),
                            ImVec2(uv1.x, uv1.y),
                            bg_col,
                            tint_col);
}

// ==================== 自定义形状实现 ====================

void ImGuiControlRenderer::DrawRect(const glm::vec2 &min,
                                    const glm::vec2 &max,
                                    ImU32 col,
                                    float rounding,
                                    ImDrawFlags flags,
                                    float thickness)
{
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRect(ImVec2(min.x, min.y), ImVec2(max.x, max.y), col, rounding, flags, thickness);
}

void ImGuiControlRenderer::DrawRectFilled(
    const glm::vec2 &min, const glm::vec2 &max, ImU32 col, float rounding, ImDrawFlags flags)
{
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRectFilled(ImVec2(min.x, min.y), ImVec2(max.x, max.y), col, rounding, flags);
}

void ImGuiControlRenderer::DrawCircle(
    const glm::vec2 &center, float radius, ImU32 col, int num_segments, float thickness)
{
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  draw_list->AddCircle(ImVec2(center.x, center.y), radius, col, num_segments, thickness);
}

void ImGuiControlRenderer::DrawCircleFilled(const glm::vec2 &center,
                                            float radius,
                                            ImU32 col,
                                            int num_segments)
{
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  draw_list->AddCircleFilled(ImVec2(center.x, center.y), radius, col, num_segments);
}

// ==================== 树形控件实现 ====================

bool ImGuiControlRenderer::TreeNode(const char *label, ImGuiTreeNodeFlags flags)
{
  return ImGui::TreeNodeEx(label, flags);
}

bool ImGuiControlRenderer::TreeNodeEx(const char *str_id,
                                      ImGuiTreeNodeFlags flags,
                                      const char *fmt,
                                      ...)
{
  va_list args;
  va_start(args, fmt);
  bool result = ImGui::TreeNodeExV(str_id, flags, fmt, args);
  va_end(args);
  return result;
}

void ImGuiControlRenderer::TreePop()
{
  ImGui::TreePop();
}

void ImGuiControlRenderer::SetNextItemOpen(bool is_open, ImGuiCond cond)
{
  ImGui::SetNextItemOpen(is_open, cond);
}

// ==================== 表格控件实现 ====================

bool ImGuiControlRenderer::BeginTable(const char *str_id,
                                      int column,
                                      ImGuiTableFlags flags,
                                      const glm::vec2 &outer_size,
                                      float inner_width)
{
  return ImGui::BeginTable(str_id, column, flags, ImVec2(outer_size.x, outer_size.y), inner_width);
}

void ImGuiControlRenderer::EndTable()
{
  ImGui::EndTable();
}

void ImGuiControlRenderer::TableNextRow(ImGuiTableRowFlags flags, float min_row_height)
{
  ImGui::TableNextRow(flags, min_row_height);
}

void ImGuiControlRenderer::TableNextColumn()
{
  ImGui::TableNextColumn();
}

void ImGuiControlRenderer::TableSetColumnIndex(int column_n)
{
  ImGui::TableSetColumnIndex(column_n);
}

// ==================== 弹出窗口实现 ====================

void ImGuiControlRenderer::OpenPopup(const char *str_id, ImGuiPopupFlags popup_flags)
{
  ImGui::OpenPopup(str_id, popup_flags);
}

bool ImGuiControlRenderer::BeginPopup(const char *str_id, ImGuiWindowFlags flags)
{
  return ImGui::BeginPopup(str_id, flags);
}

bool ImGuiControlRenderer::BeginPopupContextItem(const char *str_id, ImGuiPopupFlags popup_flags)
{
  return ImGui::BeginPopupContextItem(str_id, popup_flags);
}

bool ImGuiControlRenderer::BeginPopupContextWindow(const char *str_id, ImGuiPopupFlags popup_flags)
{
  return ImGui::BeginPopupContextWindow(str_id, popup_flags);
}

bool ImGuiControlRenderer::BeginPopupContextVoid(const char *str_id, ImGuiPopupFlags popup_flags)
{
  return ImGui::BeginPopupContextVoid(str_id, popup_flags);
}

void ImGuiControlRenderer::EndPopup()
{
  ImGui::EndPopup();
}

void ImGuiControlRenderer::CloseCurrentPopup()
{
  ImGui::CloseCurrentPopup();
}

// ==================== 拖拽控件实现 ====================

bool ImGuiControlRenderer::DragFloat(const char *label,
                                     float *v,
                                     float v_speed,
                                     float v_min,
                                     float v_max,
                                     const char *format,
                                     ImGuiSliderFlags flags)
{
  return ImGui::DragFloat(label, v, v_speed, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::DragFloat2(const char *label,
                                      float v[2],
                                      float v_speed,
                                      float v_min,
                                      float v_max,
                                      const char *format,
                                      ImGuiSliderFlags flags)
{
  return ImGui::DragFloat2(label, v, v_speed, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::DragFloat3(const char *label,
                                      float v[3],
                                      float v_speed,
                                      float v_min,
                                      float v_max,
                                      const char *format,
                                      ImGuiSliderFlags flags)
{
  return ImGui::DragFloat3(label, v, v_speed, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::DragFloat4(const char *label,
                                      float v[4],
                                      float v_speed,
                                      float v_min,
                                      float v_max,
                                      const char *format,
                                      ImGuiSliderFlags flags)
{
  return ImGui::DragFloat4(label, v, v_speed, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::DragInt(const char *label,
                                   int *v,
                                   float v_speed,
                                   int v_min,
                                   int v_max,
                                   const char *format,
                                   ImGuiSliderFlags flags)
{
  return ImGui::DragInt(label, v, v_speed, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::DragInt2(const char *label,
                                    int v[2],
                                    float v_speed,
                                    int v_min,
                                    int v_max,
                                    const char *format,
                                    ImGuiSliderFlags flags)
{
  return ImGui::DragInt2(label, v, v_speed, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::DragInt3(const char *label,
                                    int v[3],
                                    float v_speed,
                                    int v_min,
                                    int v_max,
                                    const char *format,
                                    ImGuiSliderFlags flags)
{
  return ImGui::DragInt3(label, v, v_speed, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::DragInt4(const char *label,
                                    int v[4],
                                    float v_speed,
                                    int v_min,
                                    int v_max,
                                    const char *format,
                                    ImGuiSliderFlags flags)
{
  return ImGui::DragInt4(label, v, v_speed, v_min, v_max, format, flags);
}

// ==================== 滑块控件实现 ====================

bool ImGuiControlRenderer::SliderFloat(const char *label,
                                       float *v,
                                       float v_min,
                                       float v_max,
                                       const char *format,
                                       ImGuiSliderFlags flags)
{
  return ImGui::SliderFloat(label, v, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::SliderFloat2(const char *label,
                                        float v[2],
                                        float v_min,
                                        float v_max,
                                        const char *format,
                                        ImGuiSliderFlags flags)
{
  return ImGui::SliderFloat2(label, v, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::SliderFloat3(const char *label,
                                        float v[3],
                                        float v_min,
                                        float v_max,
                                        const char *format,
                                        ImGuiSliderFlags flags)
{
  return ImGui::SliderFloat3(label, v, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::SliderFloat4(const char *label,
                                        float v[4],
                                        float v_min,
                                        float v_max,
                                        const char *format,
                                        ImGuiSliderFlags flags)
{
  return ImGui::SliderFloat4(label, v, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::SliderInt(
    const char *label, int *v, int v_min, int v_max, const char *format, ImGuiSliderFlags flags)
{
  return ImGui::SliderInt(label, v, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::SliderInt2(
    const char *label, int v[2], int v_min, int v_max, const char *format, ImGuiSliderFlags flags)
{
  return ImGui::SliderInt2(label, v, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::SliderInt3(
    const char *label, int v[3], int v_min, int v_max, const char *format, ImGuiSliderFlags flags)
{
  return ImGui::SliderInt3(label, v, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::SliderInt4(
    const char *label, int v[4], int v_min, int v_max, const char *format, ImGuiSliderFlags flags)
{
  return ImGui::SliderInt4(label, v, v_min, v_max, format, flags);
}

bool ImGuiControlRenderer::SliderAngle(const char *label,
                                       float *v_rad,
                                       float v_degrees_min,
                                       float v_degrees_max,
                                       const char *format,
                                       ImGuiSliderFlags flags)
{
  return ImGui::SliderAngle(label, v_rad, v_degrees_min, v_degrees_max, format, flags);
}

// ==================== 矢量控件实现 ====================

bool ImGuiControlRenderer::DragVector2(
    const char *label, glm::vec2 &value, float speed, float min, float max)
{
  return ImGui::DragFloat2(label, glm::value_ptr(value), speed, min, max);
}

bool ImGuiControlRenderer::DragVector3(
    const char *label, glm::vec3 &value, float speed, float min, float max)
{
  return ImGui::DragFloat3(label, glm::value_ptr(value), speed, min, max);
}

bool ImGuiControlRenderer::DragVector4(
    const char *label, glm::vec4 &value, float speed, float min, float max)
{
  return ImGui::DragFloat4(label, glm::value_ptr(value), speed, min, max);
}

// ==================== 工具函数实现 ====================

glm::vec2 ImGuiControlRenderer::GetCursorPos()
{
  ImVec2 pos = ImGui::GetCursorPos();
  return glm::vec2(pos.x, pos.y);
}

void ImGuiControlRenderer::SetCursorPos(const glm::vec2 &pos)
{
  ImGui::SetCursorPos(ImVec2(pos.x, pos.y));
}

glm::vec2 ImGuiControlRenderer::GetCursorScreenPos()
{
  ImVec2 pos = ImGui::GetCursorScreenPos();
  return glm::vec2(pos.x, pos.y);
}

void ImGuiControlRenderer::SetCursorScreenPos(const glm::vec2 &pos)
{
  ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y));
}

glm::vec2 ImGuiControlRenderer::GetContentRegionAvail()
{
  ImVec2 size = ImGui::GetContentRegionAvail();
  return glm::vec2(size.x, size.y);
}

glm::vec2 ImGuiControlRenderer::GetWindowSize()
{
  ImVec2 size = ImGui::GetWindowSize();
  return glm::vec2(size.x, size.y);
}

glm::vec2 ImGuiControlRenderer::GetWindowPos()
{
  ImVec2 pos = ImGui::GetWindowPos();
  return glm::vec2(pos.x, pos.y);
}

glm::vec2 ImGuiControlRenderer::GetWindowContentRegionMin()
{
  ImVec2 min = ImGui::GetWindowContentRegionMin();
  return glm::vec2(min.x, min.y);
}

glm::vec2 ImGuiControlRenderer::GetWindowContentRegionMax()
{
  ImVec2 max = ImGui::GetWindowContentRegionMax();
  return glm::vec2(max.x, max.y);
}

glm::vec2 ImGuiControlRenderer::CalcTextSize(const char *text,
                                             const char *text_end,
                                             bool hide_text_after_double_hash,
                                             float wrap_width)
{
  ImVec2 size = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);
  return glm::vec2(size.x, size.y);
}

}  // namespace mite
