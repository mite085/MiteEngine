#ifndef MITE_SCENE_COMPONENT_TYPE
#define MITE_SCENE_COMPONENT_TYPE

#include "headers/headers.h"

namespace mite {
/**
 * @class ComponentType
 * @brief 用于唯一标识和管理ECS架构中的组件类型
 *
 * 提供：
 * 1. 组件类型的唯一标识
 * 2. 类型信息存储
 * 3. 组件名称管理
 * 4. 类型安全的组件操作
 */
class ComponentType {
 public:
  using TypeId = uint32_t;

  /**
   * @brief 获取组件类型的唯一ID
   * @return 组件类型ID
   */
  TypeId GetId() const
  {
    return m_Id;
  }

  /**
   * @brief 获取组件类型的C++类型信息
   * @return std::type_index对象
   */
  const std::type_index &GetTypeIndex() const
  {
    return m_TypeIndex;
  }

  /**
   * @brief 获取组件类型的名称
   * @return 组件类型名称字符串
   */
  const std::string &GetName() const
  {
    return m_Name;
  }

  /**
   * @brief 获取组件类型的大小(字节)
   * @return 组件类型大小
   */
  size_t GetSize() const
  {
    return m_Size;
  }

  /**
   * @brief 比较两个组件类型是否相同
   * @param other 要比较的组件类型
   * @return 如果相同返回true
   */
  bool operator==(const ComponentType &other) const
  {
    return m_Id == other.m_Id;
  }
  bool operator!=(const ComponentType &other) const
  {
    return !(*this == other);
  }

  /**
   * @brief 用于哈希和有序容器的比较
   */
  bool operator<(const ComponentType &other) const
  {
    return m_Id < other.m_Id;
  }

  /**
   * @brief 获取全局组件类型ID计数器
   * @note 用于内部实现，生成唯一ID
   */
  static TypeId GetNextTypeId();

  /**
   * @brief 根据C++类型获取组件类型
   * @tparam T 组件类型
   * @return 对应的ComponentType对象
   */
  template<typename T> static ComponentType Get();

 private:
  // 私有构造函数，只能通过Get方法创建实例
  ComponentType(TypeId id, std::type_index typeIndex, const char *name, size_t size)
      : m_Id(id), m_TypeIndex(typeIndex), m_Name(name), m_Size(size)
  {
  }

  TypeId m_Id;                  // 唯一类型ID
  std::type_index m_TypeIndex;  // C++类型信息
  std::string m_Name;           // 类型名称(用于调试和序列化)
  size_t m_Size;                // 类型大小(字节)
};

}  // namespace mite

// ComponentType的哈希特化，用于unordered容器
namespace std {
template<> struct hash<mite::ComponentType> {
  size_t operator()(const mite::ComponentType &type) const
  {
    return hash<mite::ComponentType::TypeId>()(type.GetId());
  }
};
}  // namespace std

#endif
