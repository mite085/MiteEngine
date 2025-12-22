#ifndef MITE_UI_LOCALIZATION_H
#define MITE_UI_LOCALIZATION_H

#include "ui_event/ui_events_lifecycle.h"
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
  virtual bool LoadLanguagePack(const std::string &languageCode,
                                const std::string &filePath) = 0;
  virtual bool SetCurrentLanguage(const std::string &languageCode) = 0;
  virtual std::string GetCurrentLanguage() const = 0;
  virtual std::vector<std::string> GetAvailableLanguages() const = 0;

  // 文本翻译
  virtual std::string Translate(const std::string &key) const = 0;

  // 文本方向
  virtual bool IsRTLLanguage(const std::string &languageCode) const = 0;
  virtual TextDirection GetTextDirection() const = 0;

  // 内置语言支持
  static constexpr const char *ENGLISH = "en-US";
  static constexpr const char *SIMPLIFIED_CHINESE = "zh-CN";

  // 支持完美转发的翻译行为
  //
  // 使用示例：
  // TranslateFormat("Price: ${:.2f} !", 19.99) ,可以翻译为“价格：19.99美元！”
  //
  // 对应Json示例：
  // "common": {
  //   "Price: ${:.2f} !": "价格：{:.2f}美元！"
  // }
  template <typename... Args>
  std::string TranslateFormat(const std::string &key,
                              const Args &&...args) const {
    std::string baseText = Translate(key);

    try {
      return fmt::format(baseText, std::forward<Args>(args)...);
    } catch (const std::exception &e) {
      LOG_ERROR("String formatting failed: {}", e.what());
      return baseText;  // 返回原始格式字符串
    }
  }

 protected:
  UILocalization() = default;

  virtual void OnLanguageChanged(LanguageChangedEvent &e) = 0;
};
}  // namespace mite

#endif  // MITE_UI_LOCALIZATION_H
