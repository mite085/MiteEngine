#ifndef MITE_CORE_EVENT_SUBCRIPTION_FLAGS
#define MITE_CORE_EVENT_SUBCRIPTION_FLAGS

#include "headers/headers.h"

namespace mite {

/**
 * @brief 订阅标志位
 */
enum class SubscriptionFlags : uint8_t {
  Sync = 0,            // 同步处理（默认）
  Async = 1 << 0,      // 异步处理
  Deferred = 1 << 1,   // 延迟到下一帧处理
  ThreadSafe = 1 << 2  // 线程安全处理
};

/**
 * @brief SubscriptionFlags辅助函数
 */
namespace SubscriptionFlagUtil {
// 检查是否包含特定标志
inline bool HasFlag(SubscriptionFlags flags, SubscriptionFlags checkFlag)
{
  return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(checkFlag)) != 0;
}

// 设置标志位
inline SubscriptionFlags SetFlag(SubscriptionFlags flags, SubscriptionFlags setFlag)
{
  return static_cast<SubscriptionFlags>(static_cast<uint8_t>(flags) |
                                        static_cast<uint8_t>(setFlag));
}

// 清除标志位
inline SubscriptionFlags ClearFlag(SubscriptionFlags flags, SubscriptionFlags clearFlag)
{
  return static_cast<SubscriptionFlags>(static_cast<uint8_t>(flags) &
                                        ~static_cast<uint8_t>(clearFlag));
}

// 切换标志位
inline SubscriptionFlags ToggleFlag(SubscriptionFlags flags, SubscriptionFlags toggleFlag)
{
  return static_cast<SubscriptionFlags>(static_cast<uint8_t>(flags) ^
                                        static_cast<uint8_t>(toggleFlag));
}

// 检查是否只有同步标志
inline bool IsSyncOnly(SubscriptionFlags flags)
{
  return flags == SubscriptionFlags::Sync;
}

// 检查是否包含异步标志
inline bool IsAsync(SubscriptionFlags flags)
{
  return HasFlag(flags, SubscriptionFlags::Async);
}

// 检查是否包含延迟标志
inline bool IsDeferred(SubscriptionFlags flags)
{
  return HasFlag(flags, SubscriptionFlags::Deferred);
}

// 检查是否包含线程安全标志
inline bool IsThreadSafe(SubscriptionFlags flags)
{
  return HasFlag(flags, SubscriptionFlags::ThreadSafe);
}

// 组合标志：异步且线程安全
inline SubscriptionFlags AsyncThreadSafe()
{
  return SetFlag(SubscriptionFlags::Async, SubscriptionFlags::ThreadSafe);
}

// 组合标志：延迟且线程安全
inline SubscriptionFlags DeferredThreadSafe()
{
  return SetFlag(SubscriptionFlags::Deferred, SubscriptionFlags::ThreadSafe);
}
}  // namespace SubscriptionFlagUtil
// SubscriptionFlags运算符重载
inline SubscriptionFlags operator|(SubscriptionFlags lhs, SubscriptionFlags rhs)
{
  return SubscriptionFlagUtil::SetFlag(lhs, rhs);
}
inline SubscriptionFlags operator&(SubscriptionFlags lhs, SubscriptionFlags rhs)
{
  return static_cast<SubscriptionFlags>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}
inline SubscriptionFlags operator~(SubscriptionFlags flags)
{
  return static_cast<SubscriptionFlags>(~static_cast<uint8_t>(flags));
}
inline SubscriptionFlags &operator|=(SubscriptionFlags &lhs, SubscriptionFlags rhs)
{
  lhs = lhs | rhs;
  return lhs;
}
inline SubscriptionFlags &operator&=(SubscriptionFlags &lhs, SubscriptionFlags rhs)
{
  lhs = lhs & rhs;
  return lhs;
}
}  // namespace mite

#endif
