#ifndef MITE_IMGUI_LOCALIZATION_RENDERER_H
#define MITE_IMGUI_LOCALIZATION_RENDERER_H

#include "ui_core/ui_localization.h"
#include <imgui.h>

namespace mite {
/**
 * @brief ImGui本地化样式管理器
 * 负责将本地化系统与ImGui渲染集成
 */
class ImGuiFontManager {
 public:
  // 加载字体样式文件（ttf）
  static void LoadFonts();
  // 设定Imgui全局字体（目前支持"en-US"和"zh-CN"）
  static bool SetLanguageFont(const std::string &languageCode);

 private:
  static ImFont *m_DefaultFont;
  static ImFont *m_ChineseFont;
  static std::unordered_map<std::string, ImFont *> m_LanguageFonts;
};

/**
 * @brief ImGui本地化渲染适配器
 * 负责将本地化系统与ImGui渲染集成
 * 
 * 使用示例：
 * 
 * // 文本类型
 * IMGUI_TEXT("editor.camera_position");
 * ImGui::Text("X: %.2f, Y: %.2f, Z: %.2f", m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z);
 * 
 * // 带参数的文本
 * std::vector<std::string> args = { "scene.mite" };
 * ImGuiLocalizationRenderer::TextFormatted("editor.scene_loaded", args);
 * 
 * // 按钮类型
 * IMGUI_BUTTON("common.button_save")
 * 
 * // 复选框类型(第二个参数为bool)
 * IMGUI_CHECKBOX("editor.show_grid", &m_ShowGrid);
 */
class ImGuiLocalizationRenderer {
 public:
  static void Initialize();
  static void Shutdown();

  // 文本渲染函数（替换ImGui原生函数）
  static void Text(const char *translationKey);
  static void TextColored(const ImVec4 &col, const char *translationKey);
  static void TextDisabled(const char *translationKey);
  static void TextWrapped(const char *translationKey);

  // 带参数的文本渲染
  static void TextFormatted(const char *translationKey, const std::vector<std::string> &args = {});

  // 控件标签本地化
  static bool Button(const char *translationKey, const ImVec2 &size = ImVec2(0, 0));
  static bool Checkbox(const char *translationKey, bool *v);
  static bool InputText(const char *translationKey,
                        char *buf,
                        size_t buf_size,
                        ImGuiInputTextFlags flags = 0);

  // 菜单项本地化
  static bool MenuItem(const char *translationKey,
                       const char *shortcut = NULL,
                       bool selected = false,
                       bool enabled = true);

  // 工具提示本地化
  static void SetTooltip(const char *translationKey);

  // 语言切换UI
  static void DrawLanguageSelector();
};

// 宏定义简化调用
#define IMGUI_TEXT(key) mite::ImGuiLocalizationRenderer::Text(key)
#define IMGUI_BUTTON(key) mite::ImGuiLocalizationRenderer::Button(key)
#define IMGUI_CHECKBOX(key, v) mite::ImGuiLocalizationRenderer::Checkbox(key, v)
#define IMGUI_MENU_ITEM(key) mite::ImGuiLocalizationRenderer::MenuItem(key)

}  // namespace mite

#endif  // MITE_IMGUI_LOCALIZATION_RENDERER_H
