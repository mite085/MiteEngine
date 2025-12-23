#ifndef MITE_TIMER
#define MITE_TIMER

#include <chrono>
#include <string>
#include <unordered_map>

namespace mite {
/**
 * @brief Timer 计时器
 * @note 职责：负责精确计时
 * @note 用于局部计时器和性能分析
 */
class Timer {
 public:
  Timer();
  void Reset();

  // 获取自上次Reset()后的经过时间(秒)
  float ElapsedSeconds() const;

  // 获取自上次Reset()后的经过时间(毫秒)
  float ElapsedMillis() const;

  // 性能分析工具
  void StartProfile(const std::string &name);
  float EndProfile(const std::string &name);

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_LastFrameTime;
  std::unordered_map<std::string, decltype(m_StartTime)> m_ProfileStart;
};
};  // namespace mite

#endif
