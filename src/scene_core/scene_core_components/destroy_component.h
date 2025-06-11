#ifndef MITE_SCENE_DESTROY_COMPONENT
#define MITE_SCENE_DESTROY_COMPONENT

#include "headers/headers.h"

namespace mite {
/**
 * @brief ���ٱ�����
 *
 * ��������ڱ��ʵ������٣�ʵ�����ٲ������ڳ�������ʱͳһ����
 * �����ӳ����ٻ��ƿ��Ա����ڵ����������޸��������µĵ�����ʧЧ���⡣
 *
 * ע�⣺����һ�������������Ϊ���ʹ�ã��������κ����ݳ�Ա��
 */
struct DestroyComponent {
  // �����ݳ�Ա����������

  /**
   * @brief Ĭ�Ϲ��캯��
   */
  DestroyComponent() = default;

  /**
   * @brief ���ڵ��Ե��ַ�����ʾ
   */
  std::string ToString() const
  {
    return "DestroyComponent";
  }

  // ------------------------ ���л�֧�� ------------------------
  // ����������Ҫ���л����������ӿ��Ա���һ����

  /**
   * @brief ���л�����(��ʵ��)
   */
  template<typename Archive> void serialize(Archive &) {}  // ��������Ҫ���л�
};
};

#endif
