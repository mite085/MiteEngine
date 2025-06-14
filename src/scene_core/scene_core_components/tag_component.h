#ifndef MITE_SCENE_TAG_COMPONENT
#define MITE_SCENE_TAG_COMPONENT

#include "headers/headers.h"

namespace mite {
/**
 * @brief 标签组件 - 为实体提供名称标识和分类标签
 *
 * 每个实体都应该有一个TagComponent，用于：
 * - 在编辑器中显示友好名称
 * - 实体搜索和筛选
 * - 运行时实体标识
 */
class TagComponent {
 public:
  // 默认构造
  TagComponent() = default;

  /**
   * @brief 使用指定名称构造
   * @param tag 实体名称/标签
   */
  explicit TagComponent(const std::string &tag);

  /**
   * @brief 使用指定名称和颜色构造
   * @param tag 实体名称/标签
   * @param color 编辑器显示颜色(RGBA)
   */
  TagComponent(const std::string &tag, const glm::vec4 &color);

  // 序列化支持
  template<typename Archive> void serialize(Archive &archive);

  // ------------------------ 属性访问 ------------------------
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

  // ------------------------ 辅助方法 ------------------------
  /**
   * @brief 检查标签是否匹配搜索字符串(支持简单通配符)
   * @param searchStr 搜索字符串，可以包含*通配符
   * @return 是否匹配
   */
  bool MatchSearch(const std::string &searchStr) const;

  /**
   * @brief 获取用于显示的短名称(去除层级前缀)
   */
  std::string GetDisplayName() const;

  /**
   * @brief 检查是否包含子标签
   * @param subTag 要检查的子标签
   * @return 是否包含
   */
  bool HasSubTag(const std::string &subTag) const;

 private:
  std::string m_Tag = "Entity";                  // 实体主标签/名称
  glm::vec4 m_Color = {1.0f, 1.0f, 1.0f, 1.0f};  // 编辑器显示颜色

  // 缓存的计算属性（std::optional表示可能不存在的值，类似可空类型）
  mutable std::optional<std::vector<std::string>> m_CachedSubTags;

  /**
   * @brief 更新子标签缓存
   */
  void UpdateSubTagsCache() const;
};

};  // namespace mite

#endif
