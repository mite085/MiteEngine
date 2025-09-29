#ifndef MITE_RENDER_STAGE
#define MITE_RENDER_STAGE

#include "render_core/render_context.h"
#include "render_core/render_command.h"

namespace mite {

/**
 * @brief 渲染阶段基类（所有渲染阶段的统一接口）
 *
 * 职责：
 * 1. 定义渲染阶段的执行接口
 * 2. 管理阶段的生命周期
 * 3. 提供阶段间的数据共享机制
 */
class RenderStage {
 public:
  explicit RenderStage(const std::string &name);
  virtual ~RenderStage() = default;

  // ---- 生命周期管理 ----
  virtual void Initialize() = 0;
  virtual void Execute(RenderContext &context) = 0;
  virtual void Shutdown() = 0;

  // ---- 阶段属性 ----
  const std::string &GetName() const
  {
    return m_Name;
  }
  bool IsEnabled() const
  {
    return m_Enabled;
  }
  void SetEnabled(bool enabled)
  {
    m_Enabled = enabled;
  }

  // ---- 执行统计 ----
  uint64_t GetExecutionCount() const
  {
    return m_ExecutionCount;
  }
  double GetLastExecutionTime() const
  {
    return m_LastExecutionTime;
  }

 protected:
  // ---- 保护成员 ----
  std::string m_Name;
  bool m_Enabled = true;
  Logger m_Logger;

  // ---- 性能统计 ----
  uint64_t m_ExecutionCount = 0;
  double m_LastExecutionTime = 0.0;
  std::chrono::high_resolution_clock::time_point m_ExecutionStartTime;
};

}  // namespace mite

#endif
