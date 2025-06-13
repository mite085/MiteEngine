#ifndef MITE_SCENE
#define MITE_SCENE

#include "entity.h"
#include "components.h"

namespace mite {
// ǰ������
class SceneGraph;
class SceneObserver;
class SceneSerializer;

/**
 * @brief ������ - ��������ʵ�塢�����ϵͳ��������
 *
 * ��װEnTT��registry���ṩ��������ĸ߼��ӿڣ�������
 * - ʵ�崴��/����
 * - ϵͳ����
 * - ����״̬ά��
 * - ���л�֧��
 */
class Scene : public std::enable_shared_from_this<Scene> {
 public:
  Scene(const std::string &name = "Untitled Scene");
  ~Scene();

  // ��ֹ����
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;

  // ------------------------ �������� ------------------------
  /**
   * @brief ���³�������ϵͳ
   * @param timestep ֡ʱ��(��)
   */
  void OnUpdate(float timestep);

  /**
   * @brief ������ͼ��Ⱦǰ��׼��
   */
  void OnRenderPrepare();

  /**
   * @brief ��ճ����е��������ݣ�����Ϊ��ʼ״̬
   * @param keepSystems �Ƿ�����ע���ϵͳ��Ĭ��false����ȫ���ã�
   */
  void Clear(bool keepSystems = false);

  // ------------------------ ʵ����� ------------------------
  /**
   * @brief ������ʵ��
   * @param name ʵ������(��ѡ)
   * @return �´�����ʵ��
   */
  Entity CreateEntity(const std::string &name = "");

  /**
   * @brief ����ʵ��
   * @param entity Ҫ���ٵ�ʵ��
   */
  void DestroyEntity(Entity entity);

  /**
   * @brief ���ʵ���Ƿ���Ч
   */
  bool IsValid(Entity entity) const;

  // ------------------------ ϵͳ���� ------------------------
  /**
   * @brief ע�����ϵͳ
   * @tparam T ϵͳ����
   * @tparam Args �����������
   * @param args �������
   * @return ע���ϵͳ����
   */
  template<typename T, typename... Args> T &RegisterSystem(Args &&...args);

  /**
   * @brief ��ȡ��ע���ϵͳ
   */
  template<typename T> T &GetSystem();

  // ------------------------ ����״̬ ------------------------
  const std::string &GetName() const
  {
    return m_Name;
  }
  void SetName(const std::string &name)
  {
    m_Name = name;
  }

  /**
   * @brief ��ȡ�����ʵ��
   */
  std::shared_ptr<Entity> GetMainCamera() const
  {
    return m_MainCamera;
  }
  void SetMainCamera(std::shared_ptr<Entity> entity)
  {
    m_MainCamera = entity;
  }

  // ------------------------ ���л� ------------------------
  /**
   * @brief ���л��������ļ�
   * @param filepath �ļ�·��
   */
  void Serialize(const std::filesystem::path &filepath);

  /**
   * @brief ���ļ������л�����
   * @param filepath �ļ�·��
   */
  void Deserialize(const std::filesystem::path &filepath);

  // ------------------------ EnTT���� ------------------------
  /**
   * @brief ��ȡ�ײ�EnTT registry
   * 
   * �������л���Ҫ�޸�registry�ڵ�entities˳��
   */
  entt::registry &GetRegistry()
  {
    return m_Registry;
  }
  const entt::registry &GetRegistry() const
  {
    return m_Registry;
  }

 private:
  // ��ʼ��Ĭ��ϵͳ
  void InitSystems();

 private:
  std::string m_Name;         // ��������
  entt::registry m_Registry;  // EnTTʵ�����ע���

  // ����ϵͳ
  std::unique_ptr<SceneGraph> m_SceneGraph;        // ����ͼϵͳ
  std::unique_ptr<SceneObserver> m_SceneObserver;  // ��������۲���
  std::unique_ptr<SceneSerializer> m_Serializer;   // ���л�ϵͳ

  // ����״̬
  std::shared_ptr<Entity> m_MainCamera;  // �����ʵ��

  // ע���ϵͳ
  std::unordered_map<size_t, std::unique_ptr<ComponentSystem>> m_Systems;

  // ʵ��ID����
  uint32_t m_EntityCounter = 0;
};

// ģ�巽��ʵ��
template<typename T, typename... Args> T &Scene::RegisterSystem(Args &&...args)
{
  auto typeHash = typeid(T).hash_code();
  if (m_Systems.find(typeHash) != m_Systems.end()) {
    return *static_cast<T *>(m_Systems[typeHash].get());
  }

  auto system = std::make_unique<T>(std::forward<Args>(args)...);
  system->m_Scene = this;
  m_Systems[typeHash] = system;
  return *system;
}

template<typename T> T &Scene::GetSystem()
{
  auto typeHash = typeid(T).hash_code();
  auto it = m_Systems.find(typeHash);
  if (it == m_Systems.end()) {
    throw std::runtime_error("System not registered");
  }
  return *static_cast<T *>(it->second.get());
}

}  // namespace mite

#endif