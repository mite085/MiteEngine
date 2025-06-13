#ifndef MITE_SCENE
#define MITE_SCENE

#include "entity.h"
#include "components.h"

namespace mite {
// 前向声明
class SceneGraph;
class SceneObserver;
class SceneSerializer;

/**
 * @brief 场景类 - 管理所有实体、组件和系统的主容器
 *
 * 封装EnTT的registry，提供场景管理的高级接口，包括：
 * - 实体创建/销毁
 * - 系统管理
 * - 场景状态维护
 * - 序列化支持
 */
class Scene : public std::enable_shared_from_this<Scene> {
 public:
  Scene(const std::string &name = "Untitled Scene");
  ~Scene();

  // 禁止拷贝
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;

  // ------------------------ 场景管理 ------------------------
  /**
   * @brief 更新场景所有系统
   * @param timestep 帧时间(秒)
   */
  void OnUpdate(float timestep);

  /**
   * @brief 场景视图渲染前的准备
   */
  void OnRenderPrepare();

  /**
   * @brief 清空场景中的所有内容，重置为初始状态
   * @param keepSystems 是否保留已注册的系统（默认false，完全重置）
   */
  void Clear(bool keepSystems = false);

  // ------------------------ 实体管理 ------------------------
  /**
   * @brief 创建新实体
   * @param name 实体名称(可选)
   * @return 新创建的实体
   */
  Entity CreateEntity(const std::string &name = "");

  /**
   * @brief 销毁实体
   * @param entity 要销毁的实体
   */
  void DestroyEntity(Entity entity);

  /**
   * @brief 检查实体是否有效
   */
  bool IsValid(Entity entity) const;

  // ------------------------ 系统管理 ------------------------
  /**
   * @brief 注册组件系统
   * @tparam T 系统类型
   * @tparam Args 构造参数类型
   * @param args 构造参数
   * @return 注册的系统引用
   */
  template<typename T, typename... Args> T &RegisterSystem(Args &&...args);

  /**
   * @brief 获取已注册的系统
   */
  template<typename T> T &GetSystem();

  // ------------------------ 场景状态 ------------------------
  const std::string &GetName() const
  {
    return m_Name;
  }
  void SetName(const std::string &name)
  {
    m_Name = name;
  }

  /**
   * @brief 获取主相机实体
   */
  std::shared_ptr<Entity> GetMainCamera() const
  {
    return m_MainCamera;
  }
  void SetMainCamera(std::shared_ptr<Entity> entity)
  {
    m_MainCamera = entity;
  }

  // ------------------------ 序列化 ------------------------
  /**
   * @brief 序列化场景到文件
   * @param filepath 文件路径
   */
  void Serialize(const std::filesystem::path &filepath);

  /**
   * @brief 从文件反序列化场景
   * @param filepath 文件路径
   */
  void Deserialize(const std::filesystem::path &filepath);

  // ------------------------ EnTT访问 ------------------------
  /**
   * @brief 获取底层EnTT registry
   * 
   * 场景序列化需要修改registry内的entities顺序
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
  // 初始化默认系统
  void InitSystems();

 private:
  std::string m_Name;         // 场景名称
  entt::registry m_Registry;  // EnTT实体组件注册表

  // 场景系统
  std::unique_ptr<SceneGraph> m_SceneGraph;        // 场景图系统
  std::unique_ptr<SceneObserver> m_SceneObserver;  // 场景变更观察者
  std::unique_ptr<SceneSerializer> m_Serializer;   // 序列化系统

  // 场景状态
  std::shared_ptr<Entity> m_MainCamera;  // 主相机实体

  // 注册的系统
  std::unordered_map<size_t, std::unique_ptr<ComponentSystem>> m_Systems;

  // 实体ID生成
  uint32_t m_EntityCounter = 0;
};

// 模板方法实现
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