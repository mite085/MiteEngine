#ifndef MITE_ENUM_COMBOBOX_H
#define MITE_ENUM_COMBOBOX_H
#include <array>
#include <string>
#include <utility>
#include <vector>

namespace mite {
/**
 * @brief 通用枚举组合框列表模板类
 * @tparam EnumType 枚举类型
 * @tparam N 枚举值数量
 *
 * 用于管理枚举类型与显示名称的映射关系，支持在UI中显示下拉选择框
 * 基础功能：
 * 1. 根据枚举值获取索引
 * 2. 根据索引获取枚举值
 * 3. 获取所有显示名称
 * 4. 根据枚举值获取显示名称
 *
 * 使用示例：PropertyBase<TransformComponent>的Transform::EulerOrder相关
 */
template<typename EnumType, size_t N> class EnumComboBoxList {
 public:
  /**
   * @brief 构造函数
   * @param items 枚举值和名称的初始化数组
   */
  constexpr EnumComboBoxList(std::array<std::pair<EnumType, const char *>, N> items)
      : m_Items(std::move(items))
  {
  }

  /**
   * @brief 根据枚举值获取在列表中的索引
   * @param type 要查找的枚举值
   * @return 对应的索引，如果未找到返回0（第一个元素）
   */
  int GetIndex(EnumType type) const
  {
    // 遍历数组查找匹配的枚举值
    for (size_t i = 0; i < N; ++i) {
      if (m_Items[i].first == type) {
        return static_cast<int>(i);
      }
    }
    // 未找到时返回第一个元素的索引
    return 0;
  }

  /**
   * @brief 根据索引获取对应的枚举值
   * @param index 要查找的索引
   * @return 对应的枚举值，如果索引越界返回第一个枚举值
   */
  EnumType GetEnumType(int index) const
  {
    // 检查索引是否在有效范围内
    if (index >= 0 && index < static_cast<int>(N)) {
      return m_Items[index].first;
    }
    // 索引越界时返回第一个枚举值
    return m_Items[0].first;
  }

  /**
   * @brief 获取所有显示名称的字符串向量
   * @return 包含所有显示名称的std::vector<std::string>
   */
  std::vector<std::string> GetTranslateKeys() const
  {
    std::vector<std::string> result;
    result.reserve(N);  // 预分配内存提高性能

    // 遍历所有项，提取显示名称
    for (const auto &item : m_Items) {
      result.emplace_back(item.second);
    }
    return result;
  }

  /**
   * @brief 根据枚举值获取对应的显示名称
   * @param type 要查找的枚举值
   * @return 对应的显示名称字符串，如果未找到返回第一个元素的名称
   */
  std::string GetName(EnumType type) const
  {
    // 遍历数组查找匹配的枚举值
    for (const auto &item : m_Items) {
      if (item.first == type) {
        return std::string(item.second);
      }
    }
    // 未找到时返回第一个元素的名称
    return std::string(m_Items[0].second);
  }

 private:
  // 存储枚举值和对应显示名称的数组
  // 格式：{枚举值, 显示名称}
  std::array<std::pair<EnumType, const char *>, N> m_Items;
};
}  // namespace mite

#endif  // MITE_ENUM_COMBOBOX_H