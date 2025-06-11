#ifndef MITE_SCENE_SYSTEM
#define MITE_SCENE_SYSTEM

#include "entity.h"

namespace mite {

class Scene;    // ǰ������

/**
 * @class SceneSystem
 * @brief ECS�ܹ��е�ϵͳ���࣬�����ض�������ϵ��߼�
 *
 * ϵͳ��ÿ֡����ʱ������з���������ʵ��ִ���ض�����
 * ͨ��ģ�����ָ����ϵͳ��Ҫ������������
 */
class SceneSystem {
 public:
  // ���캯������������
  SceneSystem() = default;
  virtual ~SceneSystem() = default;

  /**
   * @brief ��ʼ��ϵͳ
   * @param scene ��������������
   */
  virtual void OnInitialize(Scene &scene) = 0;

  /**
   * @brief ÿ֡����ϵͳ
   * @param scene ��������������
   * @param deltaTime ֡���ʱ��
   */
  virtual void OnUpdate(Scene &scene, float deltaTime) = 0;

  /**
   * @brief ϵͳ����ʱ����
   * @param scene ��������������
   */
  virtual void OnShutdown(Scene &scene) = 0;

  /**
   * @brief ��ȡϵͳ����(���ڵ��Ժ���־)
   * @return ϵͳ�����ַ���
   */
  virtual const char *GetName() const = 0;

  /**
   * @brief ���ʵ���Ƿ���ϸ�ϵͳ��������
   * @param entity Ҫ����ʵ��
   * @return ���ʵ�������ϵͳ�������������򷵻�true
   */
  virtual bool IsEntityRelevant(Entity entity) const = 0;

  /**
   * @brief ����ʵ�����ϵͳ����ʱ����
   * @param entity ����ӵ�ʵ��
   */
  virtual void OnEntityAdded(Entity entity) = 0;

  /**
   * @brief ��ʵ�岻�ٷ���ϵͳ����ʱ����
   * @param entity ���Ƴ���ʵ��
   */
  virtual void OnEntityRemoved(Entity entity) = 0;

 protected:
  // ���ÿ������ƶ�
  SceneSystem(const SceneSystem &) = delete;
  SceneSystem &operator=(const SceneSystem &) = delete;
  SceneSystem(SceneSystem &&) = delete;
  SceneSystem &operator=(SceneSystem &&) = delete;
};
};

#endif
