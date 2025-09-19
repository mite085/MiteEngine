#ifndef MITE_SNAPSHOT
#define MITE_SNAPSHOT

#include "headers/headers.h"

namespace mite {

/**
 * @brief 快照基类
 *
 * 所有快照类型的基类，提供统一的接口和基础功能
 */
class ISnapshot {
 public:
  virtual ~ISnapshot() = default;

  // ================== 核心接口 ======================
  /**
   * @brief 应用快照（重做）
   */
  virtual void Apply() = 0;
  /**
   * @brief 撤销快照（撤销）
   */
  virtual void Revert() = 0;

  // ================== 工具相关 ======================
  /**
   * @brief 获取快照描述（用于调试）
   */
  virtual const char *GetDescription() const = 0;
  /**
   * @brief 检查快照是否可压缩
   */
  virtual bool IsCompressible() const
  {
    return false;
  }
  /**
   * @brief 压缩快照（如果支持）
   */
  virtual bool Compress()
  {
    return false;
  }
  /**
   * @brief 获取快照内存使用量
   */
  virtual size_t GetMemoryUsage() const = 0;

  // ================== 时间戳相关（无需重写） ======================
  /**
   * @brief 获取快照时间戳（毫秒）
   */
  uint64_t GetTimestamp() const
  {
    return m_timestamp;
  }
  /**
   * @brief 获取快照时间戳（秒）
   */
  float GetTimestampSeconds() const
  {
    return static_cast<float>(m_timestamp) / 1000.0f;
  }

 protected:
  uint64_t m_timestamp;  // 快照创建时间戳（毫秒）
};

// 快照智能指针
using SnapshotPtr = std::unique_ptr<ISnapshot>;

}  // namespace mite::scene

#endif  // MITE_SCENE_CORE_SNAPSHOT
