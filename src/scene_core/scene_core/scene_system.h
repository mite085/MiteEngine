#ifndef MITE_SCENE_SYSTEM
#define MITE_SCENE_SYSTEM

#include "entity.h"

namespace mite {

class Scene;    // 前向声明

/**
 * @class SceneSystem
 * @brief ECS架构中的系统基类，处理特定组件集合的逻辑
 *
 * 系统在每帧更新时会对所有符合条件的实体执行特定操作
 * 通过模板参数指定该系统需要处理的组件类型
 */
class SceneSystem {
 public:
  // 构造函数与析构函数
  SceneSystem() = default;
  virtual ~SceneSystem() = default;

  /**
   * @brief 初始化系统
   * @param scene 所属场景的引用
   */
  virtual void OnInitialize(Scene &scene) = 0;

  /**
   * @brief 每帧更新系统
   * @param scene 所属场景的引用
   * @param deltaTime 帧间隔时间
   */
  virtual void OnUpdate(Scene &scene, float deltaTime) = 0;

  /**
   * @brief 系统销毁时调用
   * @param scene 所属场景的引用
   */
  virtual void OnShutdown(Scene &scene) = 0;

  /**
   * @brief 获取系统名称(用于调试和日志)
   * @return 系统名称字符串
   */
  virtual const char *GetName() const = 0;

  /**
   * @brief 检查实体是否符合该系统处理条件
   * @param entity 要检查的实体
   * @return 如果实体包含该系统所需的所有组件则返回true
   */
  virtual bool IsEntityRelevant(Entity entity) const = 0;

  /**
   * @brief 当新实体符合系统条件时调用
   * @param entity 新添加的实体
   */
  virtual void OnEntityAdded(Entity entity) = 0;

  /**
   * @brief 当实体不再符合系统条件时调用
   * @param entity 被移除的实体
   */
  virtual void OnEntityRemoved(Entity entity) = 0;

 protected:
  // 禁用拷贝和移动
  SceneSystem(const SceneSystem &) = delete;
  SceneSystem &operator=(const SceneSystem &) = delete;
  SceneSystem(SceneSystem &&) = delete;
  SceneSystem &operator=(SceneSystem &&) = delete;
};
};

#endif
