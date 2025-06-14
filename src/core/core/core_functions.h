#ifndef MITE_CORE_FUNCTIONS
#define MITE_CORE_FUNCTIONS

#include <string>
#include <vector>

namespace mite {
/**
 * @brief CoreFunctions���ĺ���ģ��
 *
 * �������һЩ�ؼ���ͨ���ͺ���
 */
class CoreFunctions {
 public:
  /**
   * @brief ͨ����ַ���ƥ�亯��
   *
   * �����ж�һ���ַ����Ƿ��������ͨ���ģʽƥ�䣬
   * * - ƥ�����������������ַ�����������ַ�
   *	 (��my_image.jpg���Ա�*.jpgƥ��)
   * ? - ƥ�����ⵥ���ַ�
   *	 (��test���Ա�te??ƥ��)
   *
   * @param str		��ƥ����ַ���
   * @param pattern ����ͨ������ַ���(֧�� * �Լ� ? ����)
   * @return		ƥ��ɹ��򷵻�ture������false
   */
  static bool StringMatchWildcard(const std::string &str, const std::string &pattern);
};

}  // namespace mite

#endif
