#include "time.h"

namespace mite {

// 初始化静态成员变量
float Time::s_DeltaTime = 0.0f;
float Time::s_CurrentTime = 0.0f;
float Time::s_FixedDeltaTime = 1.0f / 60.0f;  // 默认60Hz物理更新

std::chrono::high_resolution_clock::time_point Time::s_LastFrameTime =
    std::chrono::high_resolution_clock::now();

void Time::Update()
{
  // 获取当前时间点
  auto currentTime = std::chrono::high_resolution_clock::now();

  // 计算时间差（秒）
  s_DeltaTime = std::chrono::duration<float>(currentTime - s_LastFrameTime).count();

  // 处理极端情况：如果帧时间过长（如程序暂停），限制最大DeltaTime
  const float MAX_DELTA_TIME = 0.1f;  // 最大100ms帧间隔
  if (s_DeltaTime > MAX_DELTA_TIME) {
    s_DeltaTime = MAX_DELTA_TIME;
  }

  // 更新当前时间（从程序开始到现在的秒数）
  s_CurrentTime = std::chrono::duration<float>(currentTime.time_since_epoch()).count();

  // 保存当前帧时间供下一帧使用
  s_LastFrameTime = currentTime;
}

};
