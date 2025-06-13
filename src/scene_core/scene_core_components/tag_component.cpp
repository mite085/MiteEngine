#include "tag_component.h"

namespace mite {
TagComponent::TagComponent(const std::string &tag) : m_Tag(tag)
{
  // 验证标签有效性
  if (m_Tag.empty()) {
    m_Tag = "Entity";
  }
}

TagComponent::TagComponent(const std::string &tag, const glm::vec4 &color)
    : m_Tag(tag), m_Color(color)
{
  if (m_Tag.empty()) {
    m_Tag = "Entity";
  }
}

bool TagComponent::MatchSearch(const std::string &searchStr) const
{
  // 空搜索字符串匹配所有
  if (searchStr.empty()) {
    return true;
  }

  // 简单通配符匹配
  return CoreFunctions::StringMatchWildcard(m_Tag, searchStr);
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

//// TODO: 基于cereal库的序列化实现
//template<typename Archive> void TagComponent::serialize(Archive &archive)
//{
//  archive(cereal::make_nvp("Tag", m_Tag), cereal::make_nvp("Color", m_Color));
//}
};
