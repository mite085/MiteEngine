#ifndef MITE_SCENE_ID_COMPONENT
#define MITE_SCENE_ID_COMPONENT

#include "headers/headers.h"

#define UUID_SYSTEM_GENERATOR
#include <uuid.h> 

namespace mite {
/**
 * @brief 实体唯一标识组件
 *
 * 每个实体必须包含此组件，用于：
 * 1. 实体的持久化标识
 * 2. 场景序列化/反序列化
 * 3. 跨场景引用
 * 4. 网络同步标识
 */
class IDComponent {
 public:
  /**
   * @brief 默认构造函数（生成新UUID）
   */
  IDComponent();

  /**
   * @brief 从现有UUID字符串构造
   * @param id 符合RFC4122标准的UUID字符串
   * @throws std::runtime_error 当UUID格式无效时抛出
   */
  explicit IDComponent(const std::string &id);

  // 禁止拷贝和赋值（保持ID唯一性）
  IDComponent(const IDComponent &) = delete;
  IDComponent &operator=(const IDComponent &) = delete;

  // 允许移动（转移所有权）
  IDComponent(IDComponent &&) = default;
  IDComponent &operator=(IDComponent &&) = default;

  /**
   * @brief 获取UUID字符串表示（RFC4122格式）
   * @return 示例："f81d4fae-7dec-11d0-a765-00a0c91e6bf6"
   */
  const std::string &String() const
  {
    return m_UUIDString;
  }

  /**
   * @brief 获取底层UUID对象
   */
  const uuids::uuid &GetUUID() const
  {
    return m_UUID;
  }

  /**
   * @brief 比较运算符
   */
  bool operator==(const IDComponent &other) const
  {
    return m_UUID == other.m_UUID;
  }
  bool operator!=(const IDComponent &other) const
  {
    return !(*this == other);
  }

  /**
   * @brief 生成新的随机UUID（静态方法）
   */
  static IDComponent Generate();

  /**
   * @brief 检查字符串是否为有效UUID
   * @param id 待检查字符串
   * @return 是否有效
   */
  static bool IsValid(const std::string &id);

 private:
  uuids::uuid m_UUID;        // 二进制格式UUID
  std::string m_UUIDString;  // 字符串缓存（优化频繁访问）
};
};

#endif
