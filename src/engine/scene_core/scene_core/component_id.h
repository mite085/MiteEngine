#ifndef MITE_COMPONENT_ID
#define MITE_COMPONENT_ID

#include "headers/headers.h"
#include <uuid.h>

namespace mite {
/**
 * @class ComponentID
 * @brief 唯一标识组件类型的ID系统
 *
 * 提供编译期和运行时的组件类型标识能力，支持：
 * 1. 类型安全的组件操作
 * 2. 运行时类型查询
 * 3. 序列化支持
 */
class ComponentID {
 public:
  // 获取特定组件类型的ID (编译期确定)
  template<typename T> static ComponentID Get();

  // 获取未知类型的ID (运行时使用)
  static ComponentID FromString(const std::string &uuidStr);

  // 比较操作
  bool operator==(const ComponentID &other) const;
  bool operator!=(const ComponentID &other) const;
  bool operator<(const ComponentID &other) const;

  // 转换为字符串表示
  std::string ToString() const;

  // 获取哈希值
  size_t Hash() const;

  // 是否为有效ID
  bool IsValid() const;

 private:
  explicit ComponentID(uuids::uuid id);

  // 内部UUID存储
  uuids::uuid m_ID;
};

};  // namespace mite

// 哈希特化
namespace std {
template<> struct hash<mite::ComponentID> {
  size_t operator()(const mite::ComponentID &id) const
  {
    return id.Hash();
  }
};
}  // namespace std

#endif
