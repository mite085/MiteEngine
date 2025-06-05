#ifndef MITE_WINDOW_INTERFACE
#define MITE_WINDOW_INTERFACE

#include "headers.h"

namespace mite {
// Window类型
enum WindowType {
  GLFWWINDOW,
};

// Window基本配置
// 注意，config仅在构造Window时使用一次，
// 并不用于实时存储Window的信息。
struct WindowConfig {
  WindowType type = GLFWWINDOW;       // 类型
  std::string title = "Mite Engine";  // 标题
  uint32_t width = 1280;              // 宽度
  uint32_t height = 720;              // 高度
  bool vsync = false;                 // 垂直同步
  bool fullscreen = false;            // 全屏显示
  bool resizable = true;              // 可拉伸
};

// Window抽象类
class Window {
 public:
  // 构造函数与析构函数
  virtual ~Window() = default;

  // 用于主循环,检测Window关闭标志
  virtual const bool WindowShouldClose() = 0;

  // Window生命周期管理
  virtual void Initialize(const WindowConfig &config) = 0;
  virtual void Shutdown() = 0;

  // Window属性
  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;
  virtual void *GetNativeWindow() const = 0;
  virtual bool IsVSync() const = 0;

  // Window操作
  virtual void SetVSync(bool enabled) = 0;                   // 控制垂直同步
  virtual void SetTitle(const std::string &title) = 0;       // 设置窗口标题
  virtual void Resize(uint32_t width, uint32_t height) = 0;  // 调整窗口大小
  virtual void Maximize() = 0;                               // 最大化窗口
  virtual void Minimize() = 0;                               // 最小化窗口
  virtual void Restore() = 0;                                // 恢复窗口
  virtual void Close() = 0;                                  // 关闭窗口

  // Window事件处理
  virtual void PollEvents() = 0;
  virtual void WaitEvents() = 0;

  // 键盘鼠标输入相关
  virtual bool IsKeyPressed(int keycode) const = 0;
  virtual bool IsMouseButtonPressed(int button) const = 0;
  virtual std::pair<float, float> GetMousePosition() const = 0;

  // 渲染上下文
  virtual void MakeContextCurrent() = 0;
  virtual void SwapBuffers() = 0;

  // 窗口数量计数
  static const uint32_t WindowCount(WindowType &type);

  // 回调设置
  using EventCallbackFn = std::function<void(void *)>;
  virtual void SetEventCallback(const EventCallbackFn &callback) = 0;

  // 工厂方法 - 创建特定类型的窗口
  static std::unique_ptr<Window> Create(const WindowConfig &config = WindowConfig());

 protected:
  WindowConfig m_Config;

  // 窗口状态标志
  bool m_Initialized = false;
  bool m_ShouldClose = false;

  // 日志系统
  Logger m_Logger;
};

// TODO: 窗口事件具体实现
class WindowCloseEvent : public Event {
 public:
  WindowCloseEvent() = default;
  EVENT_CLASS_TYPE(WindowClose)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowResizeEvent : public Event {
 public:
  WindowResizeEvent(uint32_t width, uint32_t height) : m_Width(width), m_Height(height) {}
  uint32_t GetWidth() const;
  uint32_t GetHeight() const;
  EVENT_CLASS_TYPE(WindowResize)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)

 private:
  uint32_t m_Width, m_Height;
};

class WindowFocusEvent : public Event {
 public:
  WindowFocusEvent() = default;
  EVENT_CLASS_TYPE(WindowFocus)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowLostFocusEvent : public Event {
 public:
  WindowLostFocusEvent() = default;
  EVENT_CLASS_TYPE(WindowLostFocus)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowMovedEvent : public Event {
 public:
  WindowMovedEvent() = default;
  EVENT_CLASS_TYPE(WindowMoved)
  EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class MouseMoveEvent : public Event {
 public:
  MouseMoveEvent(double xpos, double ypos) : xpos(xpos), ypos(ypos) {}
  double GetXPos() const;
  double GetYPos() const;

  EVENT_CLASS_TYPE(MouseMoved)
  EVENT_CLASS_CATEGORY(EventCategoryMouse)
 private:
  double xpos, ypos;
};

class MouseButtonEvent : public Event {
 public:
  MouseButtonEvent(int button, int action, int mods, double xpos, double ypos)
      : button(button), action(action), mods(mods), xpos(xpos), ypos(ypos)
  {
  }
  int GetButton() const;
  int GetAction() const;
  int GetMods() const;
  double GetXPos() const;
  double GetYPos() const;

  EVENT_CLASS_TYPE(MouseButtonReleased)
  EVENT_CLASS_CATEGORY(EventCategoryMouse)
 private:
  int button, action, mods;
  double xpos, ypos;
};

class KeyEvent : public Event {
 public:
  KeyEvent(int key, int scancode, int action, int mods)
      : key(key), scancode(scancode), action(action), mods(mods)
  {
  }
  int GetKey() const;
  int GetScancode() const;
  int GetAction() const;
  int GetMods() const;

  EVENT_CLASS_TYPE(KeyReleased)
  EVENT_CLASS_CATEGORY(EventCategoryMouse)
 private:
  int key, scancode, action /*GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT*/ , mods/*修饰键状态*/;
};
}  // namespace mite

#endif
