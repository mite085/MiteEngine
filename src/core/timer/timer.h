#ifndef MITE_TIMER
#define MITE_TIMER

#include "headers.h"

namespace mite {
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
