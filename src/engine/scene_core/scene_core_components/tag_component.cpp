#include "tag_component.h"

namespace mite {
TagComponent::TagComponent() : ComponentTraits() {}
TagComponent::TagComponent(const std::string &tag) : ComponentTraits(), m_Tag(tag)
{
  // 验证标签有效性
  if (m_Tag.empty()) {
    m_Tag = "Entity";
  }
}

TagComponent::TagComponent(const std::string &tag, const glm::vec4 &color)
    : ComponentTraits(), m_Tag(tag), m_Color(color)
{
  if (m_Tag.empty()) {
    m_Tag = "Entity";
  }
}

bool StringMatchWildcard(const std::string &str, const std::string &pattern)
{
  size_t m = str.size();
  size_t n = pattern.size();

  // 创建动态规划表，dp[i][j] 表示 str 的前 i 个字符与 pattern 的前 j 个字符是否匹配
  std::vector<std::vector<bool>> dp(m + 1, std::vector<bool>(n + 1, false));

  // 初始化：空字符串与空模式匹配
  dp[0][0] = true;

  // 处理模式以 '*' 开头的情况：空字符串可以与连续的 '*' 匹配
  for (size_t j = 1; j <= n; ++j) {
    if (pattern[j - 1] == '*') {
      // 当前 '*' 可以匹配空字符串，继承前一个模式字符的匹配状态
      dp[0][j] = dp[0][j - 1];
    }
  }
  // 填充动态规划表
  for (size_t i = 1; i <= m; ++i) {  // 遍历字符串的每个字符
    for (size_t j = 1; j <= n; ++j) {  // 遍历模式的每个字符

      if (pattern[j - 1] == '*') {
        // 情况1：遇到 '*' 通配符
        // 可以选择：1) 忽略 '*'（匹配0个字符） 2) 使用 '*' 匹配当前字符（并继续使用该 '*'）
        dp[i][j] = dp[i][j - 1] || dp[i - 1][j];
      }
      else if (pattern[j - 1] == '?' || str[i - 1] == pattern[j - 1]) {
        // 情况2：字符精确匹配或使用 '?' 通配符
        // 继承前一个字符的匹配状态
        dp[i][j] = dp[i - 1][j - 1];
      }
      // 其他情况（字符不匹配）保持 dp[i][j] = false 的初始值
    }
  }
  // 返回整个字符串与整个模式的匹配结果
  return dp[m][n];
}

bool TagComponent::MatchSearch(const std::string &searchStr) const
{
  // 空搜索字符串匹配所有
  if (searchStr.empty()) {
    return true;
  }

  // 简单通配符匹配
  return StringMatchWildcard(m_Tag, searchStr);
}



std::string TagComponent::GetDisplayName() const
{
  // 如果标签包含层级分隔符(/)，只取最后一部分
  size_t pos = m_Tag.find_last_of('/');
  if (pos != std::string::npos) {
    return m_Tag.substr(pos + 1);
  }
  return m_Tag;
}

bool TagComponent::HasSubTag(const std::string &subTag) const
{
  if (subTag.empty()) {
    return false;
  }

  // 惰性更新子标签缓存
  if (!m_CachedSubTags.has_value()) {
    UpdateSubTagsCache();
  }

  // 检查子标签是否存在
  return std::find(m_CachedSubTags->begin(), m_CachedSubTags->end(), subTag) !=
         m_CachedSubTags->end();
}

void TagComponent::UpdateSubTagsCache() const
{
  m_CachedSubTags.emplace();
  std::vector<std::string> &subTags = *m_CachedSubTags;

  // 按分隔符拆分标签
  size_t start = 0;
  size_t end = m_Tag.find('.');

  while (end != std::string::npos) {
    subTags.push_back(m_Tag.substr(start, end - start));
    start = end + 1;
    end = m_Tag.find('.', start);
  }

  // 添加最后一部分
  subTags.push_back(m_Tag.substr(start));
}

// 基于cereal库的序列化实现
template<typename Archive> void TagComponent::serialize(Archive &archive)
{
  archive(cereal::make_nvp("Tag", m_Tag), cereal::make_nvp("Color", m_Color));
}
};  // namespace mite