#include "ui_localization.h"

#include "ui_localization_json.h"

namespace mite {
UILocalization &UILocalization::Get() {
  // 当前唯一后端为Json翻译系统
  static UILocalizationJson instance;

  return instance;
}
}  // namespace mite