#ifndef MITE_SCENE_TAG_COMPONENT
#define MITE_SCENE_TAG_COMPONENT

#include "headers/headers.h"

namespace mite {
/**
 * @brief ��ǩ��� - Ϊʵ���ṩ���Ʊ�ʶ�ͷ����ǩ
 *
 * ÿ��ʵ�嶼Ӧ����һ��TagComponent�����ڣ�
 * - �ڱ༭������ʾ�Ѻ�����
 * - ʵ��������ɸѡ
 * - ����ʱʵ���ʶ
 */
class TagComponent {
 public:
  // Ĭ�Ϲ���
  TagComponent() = default;

  /**
   * @brief ʹ��ָ�����ƹ���
   * @param tag ʵ������/��ǩ
   */
  explicit TagComponent(const std::string &tag);

  /**
   * @brief ʹ��ָ�����ƺ���ɫ����
   * @param tag ʵ������/��ǩ
   * @param color �༭����ʾ��ɫ(RGBA)
   */
  TagComponent(const std::string &tag, const glm::vec4 &color);

  // ���л�֧��
  template<typename Archive> void serialize(Archive &archive);

  // ------------------------ ���Է��� ------------------------
  const std::string &GetTag() const
  {
    return m_Tag;
  }
  void SetTag(const std::string &tag)
  {
    m_Tag = tag;
  }

  const glm::vec4 &GetColor() const
  {
    return m_Color;
  }
  void SetColor(const glm::vec4 &color)
  {
    m_Color = color;
  }

  // ------------------------ �������� ------------------------
  /**
   * @brief ����ǩ�Ƿ�ƥ�������ַ���(֧�ּ�ͨ���)
   * @param searchStr �����ַ��������԰���*ͨ���
   * @return �Ƿ�ƥ��
   */
  bool MatchSearch(const std::string &searchStr) const;

  /**
   * @brief ��ȡ������ʾ�Ķ�����(ȥ���㼶ǰ׺)
   */
  std::string GetDisplayName() const;

  /**
   * @brief ����Ƿ�����ӱ�ǩ
   * @param subTag Ҫ�����ӱ�ǩ
   * @return �Ƿ����
   */
  bool HasSubTag(const std::string &subTag) const;

 private:
  std::string m_Tag = "Entity";                  // ʵ������ǩ/����
  glm::vec4 m_Color = {1.0f, 1.0f, 1.0f, 1.0f};  // �༭����ʾ��ɫ

  // ����ļ������ԣ�std::optional��ʾ���ܲ����ڵ�ֵ�����ƿɿ����ͣ�
  mutable std::optional<std::vector<std::string>> m_CachedSubTags;

  /**
   * @brief �����ӱ�ǩ����
   */
  void UpdateSubTagsCache() const;
};

};  // namespace mite

#endif
