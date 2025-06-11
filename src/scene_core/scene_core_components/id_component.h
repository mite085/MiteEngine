#ifndef MITE_SCENE_ID_COMPONENT
#define MITE_SCENE_ID_COMPONENT

#include "headers/headers.h"

#define UUID_SYSTEM_GENERATOR
#include <uuid.h> 

namespace mite {
/**
 * @brief ʵ��Ψһ��ʶ���
 *
 * ÿ��ʵ������������������ڣ�
 * 1. ʵ��ĳ־û���ʶ
 * 2. �������л�/�����л�
 * 3. �糡������
 * 4. ����ͬ����ʶ
 */
class IDComponent {
 public:
  /**
   * @brief Ĭ�Ϲ��캯����������UUID��
   */
  IDComponent();

  /**
   * @brief ������UUID�ַ�������
   * @param id ����RFC4122��׼��UUID�ַ���
   * @throws std::runtime_error ��UUID��ʽ��Чʱ�׳�
   */
  explicit IDComponent(const std::string &id);

  // ��ֹ�����͸�ֵ������IDΨһ�ԣ�
  IDComponent(const IDComponent &) = delete;
  IDComponent &operator=(const IDComponent &) = delete;

  // �����ƶ���ת������Ȩ��
  IDComponent(IDComponent &&) = default;
  IDComponent &operator=(IDComponent &&) = default;

  /**
   * @brief ��ȡUUID�ַ�����ʾ��RFC4122��ʽ��
   * @return ʾ����"f81d4fae-7dec-11d0-a765-00a0c91e6bf6"
   */
  const std::string &String() const
  {
    return m_UUIDString;
  }

  /**
   * @brief ��ȡ�ײ�UUID����
   */
  const uuids::uuid &GetUUID() const
  {
    return m_UUID;
  }

  /**
   * @brief �Ƚ������
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
   * @brief �����µ����UUID����̬������
   */
  static IDComponent Generate();

  /**
   * @brief ����ַ����Ƿ�Ϊ��ЧUUID
   * @param id ������ַ���
   * @return �Ƿ���Ч
   */
  static bool IsValid(const std::string &id);

 private:
  uuids::uuid m_UUID;        // �����Ƹ�ʽUUID
  std::string m_UUIDString;  // �ַ������棨�Ż�Ƶ�����ʣ�
};
};

#endif
