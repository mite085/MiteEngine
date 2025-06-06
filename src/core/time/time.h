#ifndef MITE_TIME
#define MITE_TIME

#include <chrono>

namespace mite {

// Time��¼ÿ֡��ˢ��ʱ�䣬
// ȷ������������ģ���������Ӧ��ϵͳ
// �ܹ�����֡���޹ص��ٶ�����
class Time {
 public:
  static void Update();  // ÿ֡���ã�����ʱ��
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
  }  // ���ڹ̶������������

  // ���ù̶����������ʱ�䲽�� (Ĭ��Ϊ1/60��)
  static void SetFixedDeltaTime(float fixedDeltaTime)
  {
    s_FixedDeltaTime = fixedDeltaTime;
  }

 private:
  static float s_DeltaTime;       // ��һ֡��ʱ���룩
  static float s_CurrentTime;     // ��ǰ֡��ʱ�䣨�룩
  static float s_FixedDeltaTime;  // �̶�ʱ�䲽������������ã�
  static std::chrono::high_resolution_clock::time_point s_LastFrameTime;  // ��һ֡��ʱ���
};
};

#endif
