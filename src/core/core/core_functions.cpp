#include "core_functions.h"

namespace mite {
bool CoreFunctions::StringMatchWildcard(const std::string &str, const std::string &pattern)
{
  int m = str.size();
  int n = pattern.size();

  // dp[i][j] ��ʾ str[0..i-1] �Ƿ�ƥ�� pattern[0..j-1]
  std::vector<std::vector<bool>> dp(m + 1, std::vector<bool>(n + 1, false));

  // ���ַ���ƥ���ģʽ
  dp[0][0] = true;

  // ����ģʽ��ͷ�ж�� '*' �����
  for (int j = 1; j <= n; ++j) {
    if (pattern[j - 1] == '*') {
      dp[0][j] = dp[0][j - 1];
    }
  }

  for (int i = 1; i <= m; ++i) {
    for (int j = 1; j <= n; ++j) {
      if (pattern[j - 1] == '*') {
        // '*' ����ƥ������ַ���һ�������ַ�
        dp[i][j] = dp[i][j - 1] || dp[i - 1][j];
      }
      else if (pattern[j - 1] == '?' || str[i - 1] == pattern[j - 1]) {
        // '?' ���ַ�ƥ��
        dp[i][j] = dp[i - 1][j - 1];
      }
    }
  }

  return dp[m][n];
}
};  // namespace mite
