#ifndef MITE_IMGUI_FONT_MANAGER_H
#define MITE_IMGUI_FONT_MANAGER_H

#include "ui_core/ui_localization.h"

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
};

}  // namespace mite

#endif  // MITE_IMGUI_FONT_MANAGER_H
