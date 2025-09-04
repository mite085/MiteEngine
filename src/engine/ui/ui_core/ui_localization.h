#ifndef MITE_UI_LOCALIZATION_H
#define MITE_UI_LOCALIZATION_H

#include "ui_event/ui_events_interaction.h"
#include "ui_style.h"

namespace mite {

enum class TextDirection {
  LTR,  // 从左到右
  RTL   // 从右到左
};

/**
 * @brief 本地化管理抽象类
 */
class UILocalization {
 public:
  virtual ~UILocalization() = default;

  // 单例访问
  static UILocalization &Get();

  // 语言管理
  virtual bool LoadLanguagePack(const std::string &languageCode, const std::string &filePath) = 0;
  virtual bool SetCurrentLanguage(const std::string &languageCode) = 0;
  virtual std::string GetCurrentLanguage() const = 0;
  virtual std::vector<std::string> GetAvailableLanguages() const = 0;

  // 文本翻译
  virtual std::string Translate(const std::string &key) const = 0;
  virtual std::string TranslateFormat(const std::string &key,
                                      const std::vector<std::string> &args) const = 0;

  // 区域设置
  virtual void SetLocale(const std::string &locale) = 0;
  virtual std::string GetCurrentLocale() const = 0;

  // 文本方向
  virtual bool IsRTLLanguage(const std::string &languageCode) const = 0;
  virtual TextDirection GetTextDirection() const = 0;

  // 内置语言支持
  static constexpr const char *ENGLISH = "en";
  static constexpr const char *SIMPLIFIED_CHINESE = "zh-CN";

 protected:
  UILocalization() = default;

  // 格式化工具函数
  std::string FormatString(const std::string &format, const std::vector<std::string> &args) const;
};

}  // namespace mite

#endif  // MITE_UI_LOCALIZATION_H
