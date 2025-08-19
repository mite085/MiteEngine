#ifndef MITE_SCENE
#define MITE_SCENE

#include "component_system_manager.h"
#include "basic_data/camera.h"

namespace mite {
/**
 * @brief 场景类 - 管理所有实体、组件和系统的主容器
 *
 * 封装EnTT的registry，提供场景管理的高级接口，包括：
 * - 实体创建/销毁
 * - 系统管理
 * - 场景状态维护
 * - 序列化支持
 */
class Scene{
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
   * @param keepSystems 是否保留已注册的系统（仅当Scene析构时不保留）
   */
  void Clear(bool keepSystems = true);

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
   * @brief 获取主相机
   */
  std::shared_ptr<Camera> GetMainCamera() const;
  void SetMainCamera(Entity entity);

  // ------------------------ 序列化 ------------------------
  /**
   * @brief 序列化场景到文件
   * @param filepath 文件路径
   */
  //void Serialize(const std::filesystem::path &filepath);

  /**
   * @brief 从文件反序列化场景
   * @param filepath 文件路径
   */
  //void Deserialize(const std::filesystem::path &filepath);

  // ------------------------ 模块访问 ------------------------
  /**
   * @brief 获取Registry
   */
  SceneRegistry &GetRegistry()
  {
    return m_Registry;
  }
  /**
   * @brief 获取ComponentSystemManager
   */
  ComponentSystemManager &GetComponentSystemManager()
  {
    return m_SystemManager;
  }

 private:
  // 初始化默认系统
  void InitSystems();
  /**
   * @brief 初始化组件系统
   */
  void InitComponentSystems();
  /**
   * @brief 关闭组件系统
   */
  void ShutDownComponentSystems();

 private:
  // 场景名称
  std::string m_Name;         

  // 实体组件注册表：
  // 直接值持有,与Scene共享生命周期，
  // 避免unique_ptr不必要的堆分配，
  // 并方便其他模块直接引用m_Registry(可能存在风险？)
  SceneRegistry m_Registry;  

  // 场景系统
  //std::unique_ptr<SceneGraph> m_SceneGraph;        // 场景图系统
  //std::unique_ptr<SceneSerializer> m_Serializer;   // 序列化系统


  // 系统管理
  ComponentSystemManager m_SystemManager;

  // 实体ID生成计数
  uint32_t m_EntityCounter = 0;
};

}  // namespace mite

#endif