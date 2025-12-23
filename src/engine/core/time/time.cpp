#include "time.h"

namespace mite {
// 初始化静态成员变量
float Time::s_DeltaTime = 0.0f;
size_t Time::s_DeltaTimeMS = 0;
float Time::s_CurrentTime = 0.0f;
size_t Time::s_CurrentTimeMS = 0;
Time::Clock::time_point Time::s_StartTime = Time::Clock::now();
Time::Clock::time_point Time::s_LastFrameTime = Time::s_StartTime;

void Time::Reset() {
  // 重置静态成员变量
  s_DeltaTime = 0.0f;
  s_DeltaTimeMS = 0;
  s_CurrentTime = 0.0f;
  s_CurrentTimeMS = 0;
  s_StartTime = Time::Clock::now();
  s_LastFrameTime = Time::s_StartTime;
}
void Time::Update() {
  // 获取当前时间点
  auto currentTime = Clock::now();

  // 使用微秒精度计算时间差（避免精度损失）
  auto deltaMicro =
      std::chrono::duration_cast<Microseconds>(currentTime - s_LastFrameTime);

  // 转换为毫秒和秒（保持高精度）
  size_t deltaMicroCount = deltaMicro.count();
  s_DeltaTimeMS = static_cast<size_t>(deltaMicroCount / 1000);  // 微秒转毫秒
  s_DeltaTime = static_cast<float>(deltaMicroCount) / 1000000.0f;  // 微秒转秒

  // 计算总时间（同样使用微秒精度）
  auto totalDuration = currentTime - s_StartTime;
  auto totalMicro = std::chrono::duration_cast<Microseconds>(totalDuration);
  int64_t totalMicroCount = totalMicro.count();

  s_CurrentTimeMS = static_cast<size_t>(totalMicroCount / 1000);
  s_CurrentTime = static_cast<float>(totalMicroCount) / 1000000.0f;

  s_LastFrameTime = currentTime;
}
float Time::DeltaTime() { return s_DeltaTime; }
size_t Time::DeltaTimeMS() { return s_DeltaTimeMS; }
float Time::CurrentTime() { return s_CurrentTime; }
size_t Time::CurrentTimeMS() { return s_CurrentTimeMS; }
};  // namespace mite