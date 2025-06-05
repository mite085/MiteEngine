#ifndef MITE_WINDOW_INTERFACE
#define MITE_WINDOW_INTERFACE

#include "headers.h"

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
  void SetEventCallback(const EventCallbackFn &callback)
  {
    m_EventCallback = callback;
  }

  // �������� - �����ض����͵Ĵ���
  static std::unique_ptr<Window> Create(const WindowConfig &config = WindowConfig());

 protected:
  WindowConfig m_Config;
  EventCallbackFn m_EventCallback;

  // ����״̬��־
  bool m_Initialized = false;
  bool m_ShouldClose = false;

  // ��־ϵͳ
  Logger m_Logger;
};

// TODO: �����¼�����ʵ��
class WindowCloseEvent : public Event {
 public:
  WindowCloseEvent() = default;
  EVENT_CLASS_TYPE(WindowClose)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowResizeEvent : public Event {
 public:
  WindowResizeEvent(unsigned int width, unsigned int height) : m_Width(width), m_Height(height) {}
  unsigned int GetWidth() const;
  unsigned int GetHeight() const;
  EVENT_CLASS_TYPE(WindowResize)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)

 private:
  unsigned int m_Width, m_Height;
};

class WindowFocusEvent : public Event {
  WindowFocusEvent() = default;
  EVENT_CLASS_TYPE(WindowFocus)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowLostFocusEvent : public Event {
  WindowLostFocusEvent() = default;
  EVENT_CLASS_TYPE(WindowLostFocus)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowMovedEvent : public Event {
  WindowMovedEvent() = default;
  EVENT_CLASS_TYPE(WindowMoved)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

}  // namespace mite

#endif
