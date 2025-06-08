#ifndef MITE_WINDOW
#define MITE_WINDOW

#include "headers.h"
#include "input_event.h"
#include "window_event.h"

namespace mite {
// Window����
enum WindowType {
  GLFWWINDOW,
};

// Window��������
// ע�⣬config���ڹ���Windowʱʹ��һ�Σ�
// ��������ʵʱ�洢Window����Ϣ��
struct WindowConfig {
  WindowType type = GLFWWINDOW;       // ����
  std::string title = "Mite Engine";  // ����
  uint32_t width = 1280;              // ���
  uint32_t height = 720;              // �߶�
  bool vsync = false;                 // ��ֱͬ��
  bool fullscreen = false;            // ȫ����ʾ
  bool resizable = true;              // ������
};

// Window������
class Window {
 public:
  // ���캯������������
  virtual ~Window() = default;

  // ������ѭ��,���Window�رձ�־
  virtual const bool WindowShouldClose() = 0;

  // Window�������ڹ���
  virtual void Initialize(const WindowConfig &config) = 0;
  virtual void Shutdown() = 0;

  // Window����
  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;
  virtual void *GetNativeWindow() const = 0;
  virtual bool IsVSync() const = 0;

  // Window����
  virtual void SetVSync(bool enabled) = 0;                   // ���ƴ�ֱͬ��
  virtual void SetTitle(const std::string &title) = 0;       // ���ô��ڱ���
  virtual void Resize(uint32_t width, uint32_t height) = 0;  // �������ڴ�С
  virtual void Maximize() = 0;                               // ��󻯴���
  virtual void Minimize() = 0;                               // ��С������
  virtual void Restore() = 0;                                // �ָ�����
  virtual void Close() = 0;                                  // �رմ���

  // Window�¼�����
  virtual void PollEvents() = 0;
  virtual void WaitEvents() = 0;

  // ��������������
  virtual bool IsKeyPressed(int keycode) const = 0;
  virtual bool IsMouseButtonPressed(int button) const = 0;
  virtual std::pair<float, float> GetMousePosition() const = 0;

  // ��Ⱦ������
  virtual void MakeContextCurrent() = 0;
  virtual void SwapBuffers() = 0;

  // ������������
  static const uint32_t WindowCount(WindowType &type);

  // �ص�����
  using EventCallbackFn = std::function<void(void *)>;
  virtual void SetEventCallback(const EventCallbackFn &callback) = 0;

  // �������� - �����ض����͵Ĵ���
  static std::unique_ptr<Window> Create(const WindowConfig &config = WindowConfig());

 protected:
  WindowConfig m_Config;

  // ����״̬��־
  bool m_Initialized = false;
  bool m_ShouldClose = false;

  // ��־ϵͳ
  Logger m_Logger;
};

}  // namespace mite

#endif
