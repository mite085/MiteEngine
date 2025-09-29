#include "glfw_window.h"

namespace mite {
// 静态成员初始化
uint32_t OpenGLWindow::s_GLFWWindowCount = 0;

OpenGLWindow::OpenGLWindow() : m_CallbackAdapter()
{
  // 初始化日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite GLFW Window");
  m_Logger->trace("GLFW Window constructor called");
}
OpenGLWindow::~OpenGLWindow()
{
  m_Logger->trace("GLFW Window destructor called");
}
const bool OpenGLWindow::WindowShouldClose()
{
  return glfwWindowShouldClose(m_Window);
}
void OpenGLWindow::Initialize(const WindowConfig &config)
{
  try {
    // 如果是第一个窗口，初始化GLFW库
    if (s_GLFWWindowCount == 0) {
      InitGLFW();
    }

    m_Logger->info("Creating GLFW window: {} ({}x{}), VSync: {}",
                   config.title,
                   config.width,
                   config.height,
                   config.vsync);

    // 设置窗口提示
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    // 创建窗口
    m_Window = glfwCreateWindow(static_cast<int>(config.width),
                                static_cast<int>(config.height),
                                config.title.c_str(),
                                config.fullscreen ? glfwGetPrimaryMonitor() : nullptr,
                                nullptr);
    if (!m_Window) {
      m_Logger->critical("Failed to create GLFW window");
      throw;
    }

    // GLFW回调适配器注册回调函数
    // 
    // 注意：
    // GLFW内部没有"重复注册"的概念，
    // 设置新的回调会覆盖旧的，
    // 所以无需为防止重复注册，
    // 在每次注册之前执行一次UnregisterCallbacks
    m_CallbackAdapter.RegisterCallbacks(m_Window);

    // 设置当前上下文
    MakeContextCurrent();

    // 加载 Glad 的函数指针（必须在上下文激活后调用）
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      m_Logger->critical("Failed to initialize Glad");
      glfwTerminate();
      throw;
    }

    // 设置VSync
    SetVSync(m_WindowData.vsync);

    // 增加窗口计数
    s_GLFWWindowCount++;
    m_Initialized = true;

    m_Logger->info("GLFW window created successfully");
  }
  catch (const std::exception &e) {
    m_Logger->critical("Window initialization failed: {}", e.what());
    throw;
  }
}
void OpenGLWindow::Shutdown()
{
  m_Logger->info("Shutting down GLFW window: {}", m_WindowData.title);
  if (m_Window) {
    // 注销回调函数
    m_CallbackAdapter.UnregisterCallbacks();
    // 关闭当前窗口
    glfwDestroyWindow(m_Window);
    m_Window = nullptr;
    s_GLFWWindowCount--;
    m_Initialized = false;
    m_ShouldClose = true;

    // 如果这是最后一个窗口，关闭GLFW
    if (s_GLFWWindowCount == 0) {
      ShutdownGLFW();
    }
  }
}
uint32_t OpenGLWindow::GetWidth() const
{
  return m_WindowData.width;
}
uint32_t OpenGLWindow::GetHeight() const
{
  return m_WindowData.height;
}
void *OpenGLWindow::GetNativeWindow() const
{
  return m_Window;
}
bool OpenGLWindow::IsVSync() const
{
  return m_WindowData.vsync;
}
void OpenGLWindow::SetVSync(bool enabled)
{
  if (m_Window) {
    m_WindowData.vsync = enabled;
    // GLFW内置设置垂直同步的函数
    glfwSwapInterval(enabled ? 1 : 0);
    m_Logger->debug("VSync {}", enabled ? "enabled" : "disabled");
  }
  else {
    m_Logger->warn("Attempt to set VSYNC on uninitialized window");
  }
}
void OpenGLWindow::SetTitle(const std::string &title)
{
  if (m_Window) {
    m_WindowData.title = title;
    // 更换窗口标题
    glfwSetWindowTitle(m_Window, title.c_str());
    m_Logger->debug("Window title set to: {}", title);
  }
  else {
    m_Logger->warn("Attempt to set TITLE on uninitialized window");
  }
}
void OpenGLWindow::Resize(uint32_t width, uint32_t height)
{
  if (m_Window) {
    m_WindowData.width = width;
    m_WindowData.height = height;
    // 重置窗口尺寸
    glfwSetWindowSize(m_Window, static_cast<int>(width), static_cast<int>(height));
    m_Logger->debug("Window resized to: {}x{}", width, height);
  }
  else {
    m_Logger->warn("Attempt to RESIZE uninitialized window");
  }
}
void OpenGLWindow::Maximize()
{
  if (m_Window) {
    glfwMaximizeWindow(m_Window);
    // 更新窗口尺寸
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    m_WindowData.width = static_cast<uint32_t>(width);
    m_WindowData.height = static_cast<uint32_t>(height);
    m_Logger->debug("Window maximized, new size: {}x{}", width, height);
  }
  else {
    m_Logger->warn("Attempt to MAXIMIZED uninitialized window");
  }
}
void OpenGLWindow::Minimize()
{
  if (m_Window) {
    glfwIconifyWindow(m_Window);
    m_Logger->debug("Window minimized");
  }
  else {
    m_Logger->warn("Attempt to MINIMIZED uninitialized window");
  }
}
void OpenGLWindow::Restore()
{
  if (m_Window) {
    glfwRestoreWindow(m_Window);
    // 更新窗口尺寸
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    m_WindowData.width = static_cast<uint32_t>(width);
    m_WindowData.height = static_cast<uint32_t>(height);
    m_Logger->debug("Window restored, size: {}x{}", width, height);
  }
  else {
    m_Logger->warn("Attempt to restore uninitialized window");
  }
}
void OpenGLWindow::Close()
{
  if (m_Window) {
    m_ShouldClose = true;
    // 关闭窗口
    glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
    m_Logger->debug("Window close requested");
  }
  else {
    m_Logger->warn("Attempt to close uninitialized window");
  }
}

void OpenGLWindow::PollEvents()
{  // 检查窗口是否有效
  if (!m_Window) {
    m_Logger->warn("Attempted to poll events on null window");
    return;
  }

  // 可记录性能调试信息
  // m_Logger->debug("Polling GLFW events for window: {}", m_WindowData.title);

  // 处理所有挂起的事件
  glfwPollEvents();

  // 检查窗口关闭请求
  m_ShouldClose = glfwWindowShouldClose(m_Window);
  if (m_ShouldClose) {
    m_Logger->info("Window close requested: {}", m_WindowData.title);
  }
}
void OpenGLWindow::WaitEvents()
{  // 检查窗口是否有效
  if (!m_Window) {
    m_Logger->warn("Attempted to wait for events on null window");
    return;
  }

  // 可记录性能调试信息
  // m_Logger->debug("Waiting for GLFW events for window: {}", m_WindowData.title);

  // 阻塞直到有新事件到达
  glfwWaitEvents();

  // 检查窗口关闭请求
  m_ShouldClose = glfwWindowShouldClose(m_Window);
  if (m_ShouldClose) {
    m_Logger->info("Window close requested during wait: {}", m_WindowData.title);
  }
}
bool OpenGLWindow::IsKeyPressed(int keycode) const
{
  // 参数检查
  if (keycode < 0 || keycode >= GLFW_KEY_LAST) {
    m_Logger->warn("Invalid keycode queried: {}", keycode);
    return false;
  }

  // 查询并返回按键状态
  int state = glfwGetKey(m_Window, keycode);
  if (state == GLFW_PRESS)
    return true;
  else
    return false;
}
bool OpenGLWindow::IsMouseButtonPressed(int button) const
{
  // 参数检查
  if (button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) {
    m_Logger->warn("Invalid mouse button queried: {}", button);
    return false;
  }

  // 查询并返回鼠标按钮状态
  int state = glfwGetKey(m_Window, button);
  if (state == GLFW_PRESS)
    return true;
  else
    return false;
}
std::pair<double, double> OpenGLWindow::GetMousePosition() const
{
  double xpos, ypos;
  // 查询并返回鼠标位置
  glfwGetCursorPos(m_Window, &xpos, &ypos);
  return {xpos, ypos};
}
void OpenGLWindow::MakeContextCurrent()
{
  if (!m_Window) {
    m_Logger->error("Attempted to make context current on null window");
    throw;
  }

  m_Logger->debug("Making context current for window: {}", m_WindowData.title);
  glfwMakeContextCurrent(m_Window);

  // 检查上下文是否成功设置
  if (glfwGetCurrentContext() != m_Window) {
    m_Logger->error("Failed to make context current for window: {}", m_WindowData.title);
    throw;
  }
}
void OpenGLWindow::SwapBuffers()
{
  if (!m_Window) {
    m_Logger->error("Attempted to swap buffers on null window");
    return;  // 这里选择不抛出异常，因为可能在关闭过程中调用
  }

  // 可记录性能调试信息
  // m_Logger->debug("Swapping buffers for window: {}", m_WindowData.title);
  glfwSwapBuffers(m_Window);

  // 检查OpenGL错误
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    m_Logger->warn("OpenGL error after buffer swap: {}", err);
  }
}
const uint32_t OpenGLWindow::GLFWWindowCount()
{
  return s_GLFWWindowCount;
}

void OpenGLWindow::InitWindowData(const WindowConfig &config) {
  m_WindowData.title      = config.title;
  m_WindowData.width      = config.width;
  m_WindowData.height     = config.height;
  m_WindowData.vsync      = config.vsync;
  m_WindowData.fullscreen = config.fullscreen;
  m_WindowData.resizable  = config.resizable;
}
void OpenGLWindow::InitGLFW()
{
  if (!glfwInit()) {
    LOG_CRITICAL("Failed to initialize GLFW");
    throw std::runtime_error("Failed to initialize GLFW");
  }

  glfwSetErrorCallback(GLFWWindowCallbackAdapter::ErrorCallback);

  // 配置GLFW
  glfwWindowHint(GLFW_SAMPLES, 4);  // 4x MSAA
  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

  LOG_INFO("GLFW initialized successfully");
}
void OpenGLWindow::ShutdownGLFW()
{
  LOG_INFO("Terminating GLFW");
  glfwTerminate();
}

}  // namespace mite