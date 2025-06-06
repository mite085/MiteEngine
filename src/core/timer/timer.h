#ifndef MITE_TIMER
#define MITE_TIMER

#include <chrono>
#include <unordered_map>

namespace mite {

// Timer������ȷ��ʱ��
// ���ھֲ���ʱ�������ܷ�����
// һ�������Timeģ���Ѿ��㹻ʹ�ã���ģ����ʱ����
class Timer {
 public:
  Timer();
  void Reset();

  // ��ȡ���ϴ�Reset()��ľ���ʱ��(��)
  float ElapsedSeconds() const;

  // ֡ʱ��ר�ã��Զ�������һ��ʱ���
  float GetDeltaTime();

  // ���ܷ�������
  void StartProfile(const std::string &name);
  float EndProfile(const std::string &name);

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_LastFrameTime;
  std::unordered_map<std::string, decltype(m_StartTime)> m_ProfileStart;
};
};

#endif
