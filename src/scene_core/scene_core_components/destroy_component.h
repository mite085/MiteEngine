#ifndef MITE_SCENE_DESTROY_COMPONENT
#define MITE_SCENE_DESTROY_COMPONENT

#include "headers/headers.h"

namespace mite {
/**
 * @brief 销毁标记组件
 *
 * 此组件用于标记实体待销毁，实际销毁操作将在场景更新时统一处理。
 * 这种延迟销毁机制可以避免在迭代过程中修改容器导致的迭代器失效问题。
 *
 * 注意：这是一个空组件，仅作为标记使用，不包含任何数据成员。
 */
struct DestroyComponent {
  // 无数据成员，纯标记组件

  /**
   * @brief 默认构造函数
   */
  DestroyComponent() = default;

  /**
   * @brief 用于调试的字符串表示
   */
  std::string ToString() const
  {
    return "DestroyComponent";
  }

  // ------------------------ 序列化支持 ------------------------
  // 标记组件不需要序列化，但保留接口以保持一致性

  /**
   * @brief 序列化操作(空实现)
   */
  template<typename Archive> void serialize(Archive &) {}  // 无数据需要序列化
};
};

#endif
