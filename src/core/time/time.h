#ifndef MITE_TIME
#define MITE_TIME

#include <chrono>

namespace mite {

// Time记录每帧的刷新时间，
// 确保动画、物理模拟和输入响应等系统
// 能够以与帧率无关的速度运行
class Time {
 public:
  static void Update();  // 每帧调用，更新时间
  static float DeltaTime()
  {
    return s_DeltaTime;
  }
  static float CurrentTime()
  {
    return s_CurrentTime;
  }
  static float FixedDeltaTime()
  {
    return s_FixedDeltaTime;
  }  // 用于固定步长物理更新

  // 设置固定的物理更新时间步长 (默认为1/60秒)
  static void SetFixedDeltaTime(float fixedDeltaTime)
  {
    s_FixedDeltaTime = fixedDeltaTime;
  }

 private:
  static float s_DeltaTime;       // 上一帧的时间差（秒）
  static float s_CurrentTime;     // 当前帧的时间（秒）
  static float s_FixedDeltaTime;  // 固定时间步长（物理更新用）
  static std::chrono::high_resolution_clock::time_point s_LastFrameTime;  // 上一帧的时间戳
};
};

#endif
