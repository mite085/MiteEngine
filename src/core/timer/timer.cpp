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

// ��ȡ���ϴ�Reset()��ľ���ʱ��(��)
float Timer::ElapsedSeconds() const
{
  auto now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<float>(now - m_StartTime).count();
}

// ֡ʱ��ר�ã��Զ�������һ��ʱ���
float Timer::GetDeltaTime()
{
  auto now = std::chrono::high_resolution_clock::now();
  float delta = std::chrono::duration<float>(now - m_LastFrameTime).count();
  m_LastFrameTime = now;
  return delta;
}

// ���ܷ�������
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