#ifndef MITE_SCENE_SERIALIZER
#define MITE_SCENE_SERIALIZER

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <entt/entt.hpp>
#include "headers/headers.h"

namespace mite {
// ǰ������
class Scene;

/**
 * @brief �������л��������𽫳����������л�Ϊ���ָ�ʽ�Լ������л�
 *
 * ֧�� JSON �Ͷ����Ƹ�ʽ������ Cereal ��ʵ�֡�������ĳ������ݡ�
 * ʵ�塢����Լ��ʲ����õȵ����л���
 */
class SceneSerializer {
 public:
  /**
   * @brief ���캯��
   * @param scene Ҫ���л��ĳ�������
   */
  explicit SceneSerializer(Scene &scene);

  /**
   * @brief ���л��������ļ���JSON��ʽ��
   * @param filepath �ļ�·��
   * @return �Ƿ�ɹ�
   */
  bool SerializeToJson(const std::string &filepath);

  /**
   * @brief ��JSON�ļ������л�����
   * @param filepath �ļ�·��
   * @return �Ƿ�ɹ�
   */
  bool DeserializeFromJson(const std::string &filepath);

  /**
   * @brief ���л��������ļ��������Ƹ�ʽ��
   * @param filepath �ļ�·��
   * @return �Ƿ�ɹ�
   */
  bool SerializeToBinary(const std::string &filepath);

  /**
   * @brief �Ӷ������ļ������л�����
   * @param filepath �ļ�·��
   * @return �Ƿ�ɹ�
   */
  bool DeserializeFromBinary(const std::string &filepath);

  /**
   * @brief ��ȡ���һ�����л�/�����л��Ĵ�����Ϣ
   * @return ������Ϣ�ַ���
   */
  const std::string &GetLastError() const
  {
    return m_lastError;
  }

 private:
  // ��������
  Scene &m_scene;

  // �洢���һ�δ�����Ϣ
  std::string m_lastError;

  /**
   * @brief ע�����п����л����������
   * @tparam Archive Cereal�浵����
   * @param archive Cereal�浵����
   */
  template<typename Archive> void RegisterComponentTypes(Archive &archive);

  /**
   * @brief ���л�ʵ�弰�����
   * @tparam Archive Cereal�浵����
   * @param archive Cereal�浵����
   * @param registry EnTTע���
   */
  template<typename Archive> void SerializeEntities(Archive &archive, entt::registry &registry);

  /**
   * @brief �����л�ʵ�弰�����
   * @tparam Archive Cereal�浵����
   * @param archive Cereal�浵����
   * @param registry EnTTע���
   */
  template<typename Archive> void DeserializeEntities(Archive &archive, entt::registry &registry);

  /**
   * @brief �������Ƹ�������
   * @return Component���������ƣ���"Transform"��"Tag"��
   */
  template<typename T> const char *component_type_name();
};
};

#endif
