#ifndef MITE_SCENE_COMPONENT_TYPE
#define MITE_SCENE_COMPONENT_TYPE

#include "headers/headers.h"

namespace mite {
/**
 * @class ComponentType
 * @brief ����Ψһ��ʶ�͹���ECS�ܹ��е��������
 *
 * �ṩ��
 * 1. ������͵�Ψһ��ʶ
 * 2. ������Ϣ�洢
 * 3. ������ƹ���
 * 4. ���Ͱ�ȫ���������
 */
class ComponentType {
 public:
  using TypeId = uint32_t;

  /**
   * @brief ��ȡ������͵�ΨһID
   * @return �������ID
   */
  TypeId GetId() const
  {
    return m_Id;
  }

  /**
   * @brief ��ȡ������͵�C++������Ϣ
   * @return std::type_index����
   */
  const std::type_index &GetTypeIndex() const
  {
    return m_TypeIndex;
  }

  /**
   * @brief ��ȡ������͵�����
   * @return ������������ַ���
   */
  const std::string &GetName() const
  {
    return m_Name;
  }

  /**
   * @brief ��ȡ������͵Ĵ�С(�ֽ�)
   * @return ������ʹ�С
   */
  size_t GetSize() const
  {
    return m_Size;
  }

  /**
   * @brief �Ƚ�������������Ƿ���ͬ
   * @param other Ҫ�Ƚϵ��������
   * @return �����ͬ����true
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
   * @brief ���ڹ�ϣ�����������ıȽ�
   */
  bool operator<(const ComponentType &other) const
  {
    return m_Id < other.m_Id;
  }

  /**
   * @brief ��ȡȫ���������ID������
   * @note �����ڲ�ʵ�֣�����ΨһID
   */
  static TypeId GetNextTypeId();

  /**
   * @brief ����C++���ͻ�ȡ�������
   * @tparam T �������
   * @return ��Ӧ��ComponentType����
   */
  template<typename T> static ComponentType Get();

 private:
  // ˽�й��캯����ֻ��ͨ��Get��������ʵ��
  ComponentType(TypeId id, std::type_index typeIndex, const char *name, size_t size)
      : m_Id(id), m_TypeIndex(typeIndex), m_Name(name), m_Size(size)
  {
  }

  TypeId m_Id;                  // Ψһ����ID
  std::type_index m_TypeIndex;  // C++������Ϣ
  std::string m_Name;           // ��������(���ڵ��Ժ����л�)
  size_t m_Size;                // ���ʹ�С(�ֽ�)
};

}  // namespace mite

// ComponentType�Ĺ�ϣ�ػ�������unordered����
namespace std {
template<> struct hash<mite::ComponentType> {
  size_t operator()(const mite::ComponentType &type) const
  {
    return hash<mite::ComponentType::TypeId>()(type.GetId());
  }
};
}  // namespace std

#endif
