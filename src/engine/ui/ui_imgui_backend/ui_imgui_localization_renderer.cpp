#include "ui_imgui_backend.h"
#include "ui_imgui_localization_renderer.h"

namespace mite {
void ImGuiFontManager::LoadFonts()
{
  ImGuiIO &io = ImGui::GetIO();

  // 加载默认字体（英文）
  m_DefaultFont = io.Fonts->AddFontDefault();

  // 加载中文字体
  std::string fontPath = FileSystem::GetAssetPath("fonts/NotoSansSC-Regular.ttf").string();
  if (FileSystem::Exists(fontPath)) {
    m_ChineseFont = io.Fonts->AddFontFromFileTTF(
        fontPath.c_str(), 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
  }
  else {
    LOG_WARN("Chinese font not found: {}", fontPath);
    m_ChineseFont = m_DefaultFont;
  }

  // 构建字体映射
  m_LanguageFonts["en-US"] = m_DefaultFont;
  m_LanguageFonts["zh-CN"] = m_ChineseFont;
}

void ImGuiFontManager::SetLanguageFont(const std::string &languageCode)
{
  auto it = m_LanguageFonts.find(languageCode);
  if (it != m_LanguageFonts.end()) {
    ImGui::GetIO().FontDefault = it->second;
  }
  else {
    ImGui::GetIO().FontDefault = m_DefaultFont;
  }
}

void ImGuiLocalizationRenderer::Initialize()
{
  LOG_INFO("ImGuiLocalizationRenderer initialized");
}

void ImGuiLocalizationRenderer::Shutdown()
{
}

void ImGuiLocalizationRenderer::Text(const char *translationKey)
{
  const std::string &translatedText = UILocalization::Get().Translate(translationKey);
  ImGui::Text("%s", translatedText.c_str());
}

void ImGuiLocalizationRenderer::TextColored(const ImVec4 &col, const char *translationKey)
{
  const std::string &translatedText = UILocalization::Get().Translate(translationKey);
  ImGui::TextColored(col, "%s", translatedText.c_str());
}

void ImGuiLocalizationRenderer::TextDisabled(const char *translationKey)
{
  const std::string &translatedText = UILocalization::Get().Translate(translationKey);
  ImGui::TextDisabled("%s", translatedText.c_str());
}

void ImGuiLocalizationRenderer::TextWrapped(const char *translationKey)
{
  const std::string &translatedText = UILocalization::Get().Translate(translationKey);
  ImGui::TextWrapped("%s", translatedText.c_str());
}

void ImGuiLocalizationRenderer::TextFormatted(const char *translationKey,
                                              const std::vector<std::string> &args)
{
  const std::string &translatedText = UILocalization::Get().TranslateFormat(translationKey, args);
  ImGui::Text("%s", translatedText.c_str());
}

bool ImGuiLocalizationRenderer::Button(const char *translationKey, const ImVec2 &size)
{
  const std::string &translatedText = UILocalization::Get().Translate(translationKey);
  return ImGui::Button(translatedText.c_str(), size);
}

bool ImGuiLocalizationRenderer::Checkbox(const char *translationKey, bool *v)
{
  const std::string &translatedText = UILocalization::Get().Translate(translationKey);
  return ImGui::Checkbox(translatedText.c_str(), v);
}

bool ImGuiLocalizationRenderer::InputText(const char *translationKey,
                                          char *buf,
                                          size_t buf_size,
                                          ImGuiInputTextFlags flags)
{
  const std::string &translatedText = UILocalization::Get().Translate(translationKey);
  return ImGui::InputText(translatedText.c_str(), buf, buf_size, flags);
}

bool ImGuiLocalizationRenderer::MenuItem(const char *translationKey,
                                         const char *shortcut,
                                         bool selected,
                                         bool enabled)
{
  const std::string &translatedText = UILocalization::Get().Translate(translationKey);
  return ImGui::MenuItem(translatedText.c_str(), shortcut, selected, enabled);
}

void ImGuiLocalizationRenderer::SetTooltip(const char *translationKey)
{
  const std::string &translatedText = UILocalization::Get().Translate(translationKey);
  ImGui::SetTooltip("%s", translatedText.c_str());
}

void ImGuiLocalizationRenderer::DrawLanguageSelector()
{
  if (ImGui::BeginCombo("##LanguageSelector", UILocalization::Get().GetCurrentLanguage().c_str()))
  {
    for (const auto &language : UILocalization::Get().GetAvailableLanguages()) {
      bool isSelected = (language == UILocalization::Get().GetCurrentLanguage());
      if (ImGui::Selectable(language.c_str(), isSelected)) {
        UILocalization::Get().SetCurrentLanguage(language);
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
}

}  // namespace mite
