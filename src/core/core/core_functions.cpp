#include "core_functions.h"

namespace mite {
bool CoreFunctions::StringMatchWildcard(const std::string &str, const std::string &pattern)
{
  int m = str.size();
  int n = pattern.size();

  // dp[i][j] 表示 str[0..i-1] 是否匹配 pattern[0..j-1]
  std::vector<std::vector<bool>> dp(m + 1, std::vector<bool>(n + 1, false));

  // 空字符串匹配空模式
  dp[0][0] = true;

  // 处理模式开头有多个 '*' 的情况
  for (int j = 1; j <= n; ++j) {
    if (pattern[j - 1] == '*') {
      dp[0][j] = dp[0][j - 1];
    }
  }

  for (int i = 1; i <= m; ++i) {
    for (int j = 1; j <= n; ++j) {
      if (pattern[j - 1] == '*') {
        // '*' 可以匹配零个字符或一个或多个字符
        dp[i][j] = dp[i][j - 1] || dp[i - 1][j];
      }
      else if (pattern[j - 1] == '?' || str[i - 1] == pattern[j - 1]) {
        // '?' 或字符匹配
        dp[i][j] = dp[i - 1][j - 1];
      }
    }
  }

  return dp[m][n];
}
};  // namespace mite
