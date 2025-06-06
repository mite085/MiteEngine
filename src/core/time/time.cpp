#include "time.h"

namespace mite {

// ��ʼ����̬��Ա����
float Time::s_DeltaTime = 0.0f;
float Time::s_CurrentTime = 0.0f;
float Time::s_FixedDeltaTime = 1.0f / 60.0f;  // Ĭ��60Hz�������

std::chrono::high_resolution_clock::time_point Time::s_LastFrameTime =
    std::chrono::high_resolution_clock::now();

void Time::Update()
{
  // ��ȡ��ǰʱ���
  auto currentTime = std::chrono::high_resolution_clock::now();

  // ����ʱ���룩
  s_DeltaTime = std::chrono::duration<float>(currentTime - s_LastFrameTime).count();

  // ��������������֡ʱ��������������ͣ�����������DeltaTime
  const float MAX_DELTA_TIME = 0.1f;  // ���100ms֡���
  if (s_DeltaTime > MAX_DELTA_TIME) {
    s_DeltaTime = MAX_DELTA_TIME;
  }

  // ���µ�ǰʱ�䣨�ӳ���ʼ�����ڵ�������
  s_CurrentTime = std::chrono::duration<float>(currentTime.time_since_epoch()).count();

  // ���浱ǰ֡ʱ�乩��һ֡ʹ��
  s_LastFrameTime = currentTime;
}

};
