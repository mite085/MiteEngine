#include "tag_component.h"

namespace mite {
TagComponent::TagComponent(const std::string &tag) : m_Tag(tag)
{
  // ��֤��ǩ��Ч��
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
  // �������ַ���ƥ������
  if (searchStr.empty()) {
    return true;
  }

  // ��ͨ���ƥ��
  return CoreFunctions::StringMatchWildcard(m_Tag, searchStr);
}

std::string TagComponent::GetDisplayName() const
{
  // �����ǩ�����㼶�ָ���(/)��ֻȡ���һ����
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

  // ���Ը����ӱ�ǩ����
  if (!m_CachedSubTags.has_value()) {
    UpdateSubTagsCache();
  }

  // ����ӱ�ǩ�Ƿ����
  return std::find(m_CachedSubTags->begin(), m_CachedSubTags->end(), subTag) !=
         m_CachedSubTags->end();
}

void TagComponent::UpdateSubTagsCache() const
{
  m_CachedSubTags.emplace();
  std::vector<std::string> &subTags = *m_CachedSubTags;

  // ���ָ�����ֱ�ǩ
  size_t start = 0;
  size_t end = m_Tag.find('.');

  while (end != std::string::npos) {
    subTags.push_back(m_Tag.substr(start, end - start));
    start = end + 1;
    end = m_Tag.find('.', start);
  }

  // ������һ����
  subTags.push_back(m_Tag.substr(start));
}

//// TODO: ����cereal������л�ʵ��
//template<typename Archive> void TagComponent::serialize(Archive &archive)
//{
//  archive(cereal::make_nvp("Tag", m_Tag), cereal::make_nvp("Color", m_Color));
//}
};
