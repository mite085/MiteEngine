#ifndef MITE_TIMER
#define MITE_TIMER

#include <chrono>
#include <unordered_map>

namespace mite {

// Timer仅负责精确计时，
// 用于局部计时器和性能分析。
// 一般情况下Time模块已经足够使用，该模块暂时保留
class Timer {
 public:
  Timer();
  void Reset();

  // 获取自上次Reset()后的经过时间(秒)
  float ElapsedSeconds() const;

  // 帧时间专用：自动更新上一次时间戳
  float GetDeltaTime();

  // 性能分析工具
  void StartProfile(const std::string &name);
  float EndProfile(const std::string &name);

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_LastFrameTime;
  std::unordered_map<std::string, decltype(m_StartTime)> m_ProfileStart;
};
};

#endif
