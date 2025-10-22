#include "ui_imgui_backend.h"
#include "ui_imgui_font_manager.h"
#include "filesystem/filesystem.h"
#include <imgui.h>

namespace mite {
ImFont *s_EnglistFont = nullptr;
ImFont *s_ChineseFont = nullptr;
std::unordered_map<std::string, ImFont *> s_LanguageFonts = {};

void ImGuiFontManager::LoadFonts()
{
  ImGuiIO &io = ImGui::GetIO();

  // 加载默认字体（英文）
  s_EnglistFont = io.Fonts->AddFontDefault();

  // 加载中文字体
  std::string fontPath = FileSystem::GetAssetPath("localization/NotoSansSC-Regular.ttf").string();
  if (FileSystem::Exists(fontPath)) {
    s_ChineseFont = io.Fonts->AddFontFromFileTTF(
        fontPath.c_str(), 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
  }
  else {
    LOG_WARN("Chinese font not found: {}", fontPath);
    s_ChineseFont = s_EnglistFont;
  }

  // 构建字体映射
  s_LanguageFonts["en-US"] = s_EnglistFont;
  s_LanguageFonts["zh-CN"] = s_ChineseFont;
}

bool ImGuiFontManager::SetLanguageFont(const std::string &languageCode)
{
  auto it = s_LanguageFonts.find(languageCode);
  if (it != s_LanguageFonts.end()) {
    ImGui::GetIO().FontDefault = it->second;
    return true;
  }
  else {
    LOG_WARN("Language code not found: {}, set language font failed!", languageCode);
    return false;
  }
}


}  // namespace mite
