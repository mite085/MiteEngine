#include "timer.h"

namespace mite {
Timer::Timer()
{
  Reset();
}

void Timer::Reset()
{
  m_StartTime = std::chrono::high_resolution_clock::now();
}

// 获取自上次Reset()后的经过时间(秒)
float Timer::ElapsedSeconds() const
{
  auto now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<float>(now - m_StartTime).count();
}

// 帧时间专用：自动更新上一次时间戳
float Timer::GetDeltaTime()
{
  auto now = std::chrono::high_resolution_clock::now();
  float delta = std::chrono::duration<float>(now - m_LastFrameTime).count();
  m_LastFrameTime = now;
  return delta;
}

// 性能分析工具
void Timer::StartProfile(const std::string &name)
{
  m_ProfileStart[name] = std::chrono::high_resolution_clock::now();
}

float Timer::EndProfile(const std::string &name)
{
  auto end = std::chrono::high_resolution_clock::now();
  auto start = m_ProfileStart[name];
  return std::chrono::duration<float, std::milli>(end - start).count();
}
};  // namespace mite