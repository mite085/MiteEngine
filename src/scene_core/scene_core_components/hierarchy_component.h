#ifndef MITE_SCENE_HIERACHY_COMPONENT
#define MITE_SCENE_HIERACHY_COMPONENT

#include "headers/headers.h"
#include <entt/entt.hpp>

namespace mite {
/**
 * @brief ʵ���νṹ���
 *
 * ����ʵ���ĸ��ӹ�ϵ�����ɳ���ͼ�Ļ����ṹ
 * ע�⣺ʵ�ʸ��ӹ�ϵ�߼���Entity�����������洢����
 */
class HierarchyComponent {
 public:
  /**
   * @brief Ĭ�Ϲ��캯���������޸��ڵ�ĸ�ʵ�壩
   */
  HierarchyComponent() = default;

  // ��ֹ��������ι�ϵӦΨһ��
  HierarchyComponent(const HierarchyComponent &) = delete;
  HierarchyComponent &operator=(const HierarchyComponent &) = delete;

  // �����ƶ�
  HierarchyComponent(HierarchyComponent &&) noexcept = default;
  HierarchyComponent &operator=(HierarchyComponent &&) noexcept = default;

  /**
   * @brief ��ȡ��ʵ����
   * @return ��ʵ��EnTT�����entt::null��ʾ�޸��ڵ㣩
   */
  entt::entity GetParent() const
  {
    return m_Parent;
  }

  /**
   * @brief ��ȡ������ʵ����
   * @return ��ʵ�����б������˳��
   */
  const std::vector<entt::entity> &GetChildren() const
  {
    return m_Children;
  }

  /**
   * @brief ��ȡ��ʵ������
   */
  size_t GetChildCount() const
  {
    return m_Children.size();
  }

  /**
   * @brief ����Ƿ�ΪҶ�ڵ㣨���ӽڵ㣩
   */
  bool IsLeaf() const
  {
    return m_Children.empty();
  }

  /**
   * @brief ����Ƿ�Ϊ���ڵ㣨�޸��ڵ㣩
   */
  bool IsRoot() const
  {
    return m_Parent == entt::null;
  }

  /**
   * @brief ��ȡ��ȣ�������ڵ�Ĳ㼶����
   * @note ��Ҫ�ڳ����в�ѯ������������һ������
   */
  size_t GetDepth(const entt::registry &registry) const;

 private:
  friend class Entity;  // ����Entity��ֱ���޸Ĳ�ι�ϵ

  // �ڲ����� ==============================================

  /**
   * @brief ����ӽڵ㣨�ڲ�ʹ�ã�
   * @param child ��ʵ����
   */
  void AddChild(entt::entity child);

  /**
   * @brief �Ƴ��ӽڵ㣨�ڲ�ʹ�ã�
   * @param child ��ʵ����
   * @return �Ƿ�ɹ��Ƴ�
   */
  bool RemoveChild(entt::entity child);

  /**
   * @brief ��������ӽڵ㣨�ڲ�ʹ�ã�
   */
  void ClearChildren();

  /**
   * @brief ���ø��ڵ㣨�ڲ�ʹ�ã�
   * @param parent ��ʵ����
   */
  void SetParent(entt::entity parent);

 private:
  entt::entity m_Parent{entt::null};     // ��ʵ����
  std::vector<entt::entity> m_Children;  // ��ʵ���б�
  size_t m_DepthCache = 0;               // ��Ȼ��棨�ǳ־û���
};
};

#endif
