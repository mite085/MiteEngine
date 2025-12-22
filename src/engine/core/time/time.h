#ifndef MITE_TIME
#define MITE_TIME

#include <chrono>

namespace mite {
// Time记录每帧的刷新时间，
// 确保动画、物理模拟和输入响应等系统
// 能够以与帧率无关的速度运行
class Time {
 public:
  // 内部使用高精度计时
  using Clock = std::chrono::high_resolution_clock;
  using Milliseconds = std::chrono::milliseconds;
  using Microseconds = std::chrono::microseconds;

  static void Reset();  // 重置累积时间

  static void Update();  // 每帧调用，更新时间

  // 对外提供两种接口：秒和毫秒
  static float DeltaTime();
  static size_t DeltaTimeMS();
  static float CurrentTime();
  static size_t CurrentTimeMS();

 private:
  // 内部存储两种表示
  static float s_DeltaTime;
  static size_t s_DeltaTimeMS;

  static float s_CurrentTime;
  static size_t s_CurrentTimeMS;

  static Clock::time_point s_StartTime;
  static Clock::time_point s_LastFrameTime;
};
};  // namespace mite

#endif
