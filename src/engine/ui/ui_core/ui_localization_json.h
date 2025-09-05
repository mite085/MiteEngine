#ifndef MITE_IMGUI_LOCALIZATION_H
#define MITE_IMGUI_LOCALIZATION_H

#include "filesystem/fileSystem.h"
#include "ui_core/ui_localization.h"

#include <nlohmann/json.hpp>

namespace mite {

/**
 * @brief 基于Json配置文件的本地化（翻译）实现
 */
class UILocalizationJson : public UILocalization {
 public:
  UILocalizationJson();
  ~UILocalizationJson() override;

  // 语言管理
  bool LoadLanguagePack(const std::string &languageCode, const std::string &filePath) override;
  bool SetCurrentLanguage(const std::string &languageCode) override;
  std::string GetCurrentLanguage() const override;
  std::vector<std::string> GetAvailableLanguages() const override;

  // 文本翻译
  std::string Translate(const std::string &key) const override;
  std::string TranslateFormat(const std::string &key,
                              const std::vector<std::string> &args) const override;

  // 文本方向
  bool IsRTLLanguage(const std::string &languageCode) const override;
  TextDirection GetTextDirection() const override;

  // 初始化内置语言
  void InitializeBuiltinLanguages();

 private:
  struct LanguagePack {
    std::unordered_map<std::string, std::string> translations;
    std::string locale;
    TextDirection direction;
    std::string displayName;
  };

  std::string m_CurrentLanguage;
  std::unordered_map<std::string, LanguagePack> m_LanguagePacks;
  Logger m_Logger;

  // 从文件加载语言包
  bool LoadLanguagePackFromFile(const std::string &languageCode, const std::string &filePath);

  // 获取本地化文件路径
  std::string GetLocalizationFilePath(const std::string &languageCode) const;

  // 验证JSON格式
  bool ValidateLanguagePack(const nlohmann::json &jsonData) const;

  // 解析JSON到语言包
  bool ParseLanguagePack(const nlohmann::json &jsonData, LanguagePack &pack) const;

  // 合并翻译项（支持层级结构）
  void MergeTranslations(const nlohmann::json &source,
                         std::unordered_map<std::string, std::string> &target,
                         const std::string &prefix = "") const;
};

}  // namespace mite

#endif  // MITE_IMGUI_LOCALIZATION_H
