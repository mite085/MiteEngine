#ifndef MITE_SCENE_DESTROY_COMPONENT
#define MITE_SCENE_DESTROY_COMPONENT

#include "scene_core/component_system.h"

namespace mite {
// 前向声明
class SceneRegistry;
/**
 * @brief 销毁标记组件
 *
 * 此组件用于标记实体待销毁，实际销毁操作将在场景更新时统一处理。
 * 这种延迟销毁机制可以避免在迭代过程中修改容器导致的迭代器失效问题。
 *
 * 注意：这是一个空组件，仅作为标记使用，不包含任何数据成员。
 */
struct DestroyComponent : public ComponentTraits<DestroyComponent, Component::Family::Cleanup> {
  // 无数据成员，纯标记组件

  /**
   * @brief 默认构造函数
   */
  DestroyComponent();

  /**
   * @brief 用于调试的字符串表示
   */
  std::string ToString() const
  {
    return "DestroyComponent";
  }

  /**
   * @brief 序列化操作(空实现)
   */
  bool Serialize(std::ostream &output) const override
  {
    return true; // 无数据需要序列化
  }  
};

// Destroy组件系统 =====================================================
class DestroyComponentSystem : public ComponentSystem<DestroyComponent> {
  DECLARE_COMPONENT_SYSTEM(DestroyComponentSystem)

};
// Destroy组件事件：由EntityDestroyEvent代行 =====================================================

};

#endif
