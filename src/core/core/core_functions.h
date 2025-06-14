#ifndef MITE_CORE_FUNCTIONS
#define MITE_CORE_FUNCTIONS

#include <string>
#include <vector>

namespace mite {
/**
 * @brief CoreFunctions核心函数模块
 *
 * 负责管理一些关键的通用型函数
 */
class CoreFunctions {
 public:
  /**
   * @brief 通配符字符串匹配函数
   *
   * 用于判断一个字符串是否与给定的通配符模式匹配，
   * * - 匹配任意数量的任意字符，包括零个字符
   *	 (如my_image.jpg可以被*.jpg匹配)
   * ? - 匹配任意单个字符
   *	 (如test可以被te??匹配)
   *
   * @param str		被匹配的字符串
   * @param pattern 包含通配符的字符串(支持 * 以及 ? 符号)
   * @return		匹配成功则返回ture，否则false
   */
  static bool StringMatchWildcard(const std::string &str, const std::string &pattern);
};

}  // namespace mite

#endif
