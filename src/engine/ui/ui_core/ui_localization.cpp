#include "ui_localization.h"

namespace mite {

UILocalization &UILocalization::Get()
{
  // TODO: 实际实现将在具体后端中提供
  static UILocalization *instance = nullptr;
  if (!instance) {
    LOG_ERROR("UILocalization instance not initialized");
    // 返回一个默认实现或抛出异常
  }
  return *instance;
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
