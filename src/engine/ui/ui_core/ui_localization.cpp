#include "ui_localization.h"
#include "ui_localization_json.h"

namespace mite {

UILocalization &UILocalization::Get()
{
  // 当前唯一后端为Json翻译系统
  static UILocalizationJson instance;

  return instance;
}

std::string UILocalization::FormatString(const std::string &format,
                                         const std::vector<std::string> &args) const
{
  try {
    return fmt::vformat(format, fmt::make_format_args(args));
  }
  catch (const std::exception &e) {
    LOG_ERROR("String formatting failed: {}", e.what());
    return format;  // 返回原始格式字符串
  }
}

}  // namespace mite
