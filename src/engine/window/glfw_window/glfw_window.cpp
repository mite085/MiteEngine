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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);  // 启用调试上下文

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

    // 启用错误检查
    initializeOpenGLDebugging();

    // 初始化SPIRV支持（基于Shaderc编译着色器需要这一步）
    InitializeSPIRVSupport();
    CheckSPIRVSupportDetailed();

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
      cleanupOpenGLDebugging();
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

void OpenGLWindow::InitWindowData(const WindowConfig &config)
{
  m_WindowData.title = config.title;
  m_WindowData.width = config.width;
  m_WindowData.height = config.height;
  m_WindowData.vsync = config.vsync;
  m_WindowData.fullscreen = config.fullscreen;
  m_WindowData.resizable = config.resizable;
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
void OpenGLWindow::LoadSPIRVExtensions()
{
  // 获取函数指针
  auto glSpecializeShaderPtr = (PFNGLSPECIALIZESHADERPROC)glfwGetProcAddress(
      "glSpecializeShaderARB");
  if (!glSpecializeShaderPtr) {
    glSpecializeShaderPtr = (PFNGLSPECIALIZESHADERPROC)glfwGetProcAddress("glSpecializeShader");
  }

  if (glSpecializeShaderPtr) {
    // 替换GLAD的函数指针
    glSpecializeShader = glSpecializeShaderPtr;
    LOG_INFO("Successfully loaded glSpecializeShader");
  }
  else {
    LOG_WARN("Failed to load glSpecializeShader function");
  }
}
bool OpenGLWindow::InitializeSPIRVSupport()
{
  LoadSPIRVExtensions();

  // 重新检查支持状态
  if (!glSpecializeShader) {
    LOG_WARN("SPIR-V support initialization failed");
    return false;
  }

  LOG_INFO("SPIR-V support initialized successfully");
  return true;
}
void OpenGLWindow::CheckSPIRVSupportDetailed()
{
  LOG_INFO("=== SPIR-V Toolchain Diagnostics ===");
  // 1. 基础OpenGL信息
  LOG_INFO("--- OpenGL Context Info ---");
  const char *version = (const char *)glGetString(GL_VERSION);
  const char *renderer = (const char *)glGetString(GL_RENDERER);
  const char *vendor = (const char *)glGetString(GL_VENDOR);
  const char *glsl_version = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
  LOG_INFO("OpenGL Version: {}", version ? version : "Unknown");
  LOG_INFO("Renderer: {}", renderer ? renderer : "Unknown");
  LOG_INFO("Vendor: {}", vendor ? vendor : "Unknown");
  LOG_INFO("GLSL Version: {}", glsl_version ? glsl_version : "Unknown");
  // 2. 检查GLAD加载状态
  LOG_INFO("--- GLAD Function Loading ---");
  CheckGLADFunctions();
  // 3. 检查扩展支持
  LOG_INFO("--- Extension Support ---");
  CheckSPIRVExtensions();
  // 4. 检查二进制格式支持
  LOG_INFO("--- Binary Format Support ---");
  CheckBinaryFormats();
  // 5. 检查工具链可用性
  LOG_INFO("--- Toolchain Availability ---");
  CheckToolchainSupport();
  LOG_INFO("=== Diagnostics Complete ===");
}
void OpenGLWindow::CheckGLADFunctions()
{
  // 检查GLAD是否正确加载了SPIR-V相关函数
  struct FunctionCheck {
    const char *name;
    void *pointer;
  } functions[] = {
      {"glSpecializeShader", (void *)glSpecializeShader},
      {"glShaderBinary", (void *)glShaderBinary},
      {"glGetProgramBinary", (void *)glGetProgramBinary},
      {"glProgramBinary", (void *)glProgramBinary},
  };
  for (const auto &func : functions) {
    if (func.pointer) {
      LOG_INFO("{}: Loaded successfully", func.name);
    }
    else {
      LOG_WARN("{}: NOT loaded (GLAD issue)", func.name);
    }
  }
// 检查GLAD版本
#ifdef GLAD_VERSION
  LOG_INFO("GLAD version: {}", GLAD_VERSION);
#else
  LOG_INFO("GLAD version: Unknown");
#endif
}
void OpenGLWindow::CheckSPIRVExtensions()
{
  // 检查所有相关的SPIR-V扩展
  const char *extensions[] = {
      "GL_ARB_gl_spirv",
      "GL_ARB_spirv_extensions",
      "GL_ARB_ES2_compatibility",
      "GL_ARB_program_interface_query",
      "GL_ARB_separate_shader_objects",
  };
  for (const char *ext : extensions) {
    if (CheckExtension(ext)) {
      LOG_INFO("{}: Supported", ext);
    }
    else {
      LOG_INFO("{}: Not supported", ext);
    }
  }
}
bool OpenGLWindow::CheckExtension(const char *extensionName)
{
  if (glGetStringi) {
    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for (GLint i = 0; i < numExtensions; i++) {
      const char *ext = (const char *)glGetStringi(GL_EXTENSIONS, i);
      if (ext && strcmp(ext, extensionName) == 0) {
        return true;
      }
    }
  }
  return false;
}
void OpenGLWindow::CheckBinaryFormats()
{
  // 检查程序二进制格式
  GLint numProgramFormats = 0;
  glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &numProgramFormats);
  LOG_INFO("Total program binary formats supported: {}", numProgramFormats);

  if (numProgramFormats > 0) {
    std::vector<GLint> programFormats(numProgramFormats);
    glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, programFormats.data());
    LOG_INFO("All supported program binary formats:");
    for (GLint format : programFormats) {
      const char *name = "Unknown";
      switch (format) {
        case 0x8F60:
          name = "GLSL";
          break;
        case 0x96BA:
          name = "SPIR-V Program";
          break;
        default:
          name = "Other";
      }
      LOG_INFO("  - 0x{:04X}: {}", format, name);
    }
  }

  // 如果需要检查着色器二进制格式
  GLint numShaderFormats = 0;
  glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &numShaderFormats);
  LOG_INFO("Total shader binary formats supported: {}", numShaderFormats);

  if (numShaderFormats > 0) {
    std::vector<GLint> shaderFormats(numShaderFormats);
    glGetIntegerv(GL_SHADER_BINARY_FORMATS, shaderFormats.data());
    LOG_INFO("All supported shader binary formats:");
    for (GLint format : shaderFormats) {
      const char *name = "Unknown";
      switch (format) {
        case 0x9551:  // GL_SHADER_BINARY_FORMAT_SPIR_V
          name = "SPIR-V Shader";
          break;
        case 0x8DF4:  // GL_SHADER_BINARY_FORMAT_SPIR_V_ARB
          name = "SPIR-V ARB";
          break;
        default:
          name = "Other";
      }
      LOG_INFO("  - 0x{:04X}: {}", format, name);
    }
  }
}
void OpenGLWindow::CheckToolchainSupport()
{
  LOG_INFO("Checking build configuration...");
// 检查编译时定义
#ifdef GL_ARB_gl_spirv
  LOG_INFO("GL_ARB_gl_spirv defined in headers");
#else
  LOG_WARN("GL_ARB_gl_spirv NOT defined in headers");
#endif
#ifdef GL_SHADER_BINARY_FORMAT_SPIR_V
  LOG_INFO("GL_SHADER_BINARY_FORMAT_SPIR_V defined: 0x{:X}", GL_SHADER_BINARY_FORMAT_SPIR_V);
#else
  LOG_WARN("GL_SHADER_BINARY_FORMAT_SPIR_V NOT defined");
#endif
  // 检查GLFW配置
  LOG_INFO("GLFW context hints:");
  int context_version_major, context_version_minor;
  context_version_major = glfwGetWindowAttrib(m_Window, GLFW_CONTEXT_VERSION_MAJOR);
  context_version_minor = glfwGetWindowAttrib(m_Window, GLFW_CONTEXT_VERSION_MINOR);
  LOG_INFO("  - Context version: {}.{}", context_version_major, context_version_minor);
  int profile = glfwGetWindowAttrib(m_Window, GLFW_OPENGL_PROFILE);
  LOG_INFO("  - Profile: {}",
           profile == GLFW_OPENGL_CORE_PROFILE   ? "Core" :
           profile == GLFW_OPENGL_COMPAT_PROFILE ? "Compatibility" :
                                                   "Unknown");
  int debug = glfwGetWindowAttrib(m_Window, GLFW_OPENGL_DEBUG_CONTEXT);
  LOG_INFO("  - Debug context: {}", debug ? "Yes" : "No");
  // 检查可能的头文件冲突
  LOG_INFO("Header file analysis:");
#if defined(__GL_H_) && defined(__glew_h__)
  LOG_WARN("Possible GLEW/GL header conflict detected");
#endif
#if defined(GLAD_GL) && defined(__GLEW_H__)
  LOG_WARN("GLAD and GLEW both included - this will cause issues");
#endif
}
// 辅助函数：获取详细的驱动信息
void OpenGLWindow::LogDriverInfo()
{
#ifdef _WIN32
  LOG_INFO("Platform: Windows");
#elif defined(__linux__)
  LOG_INFO("Platform: Linux");
// 可以添加更多平台特定的驱动检查
#elif defined(__APPLE__)
  LOG_INFO("Platform: macOS");
#endif
  // NVIDIA特定信息
  const char *vendor = (const char *)glGetString(GL_VENDOR);
  if (vendor && strstr(vendor, "NVIDIA")) {
    LOG_INFO("NVIDIA driver detected");
  }
}
// 全局调试回调函数
void OpenGLWindow::openGLErrorCallback(GLenum source,
                                       GLenum type,
                                       GLuint id,
                                       GLenum severity,
                                       GLsizei length,
                                       const GLchar *message,
                                       const void *userParam)
{
  // 忽略通知级别的信息，专注于错误和警告
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
    return;
  }

  // 格式化输出错误信息
  const char *sourceStr = "Unknown";
  const char *typeStr = "Unknown";
  const char *severityStr = "Unknown";

  // 解析错误来源
  switch (source) {
    case GL_DEBUG_SOURCE_API:
      sourceStr = "API";
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      sourceStr = "Window System";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      sourceStr = "Shader Compiler";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      sourceStr = "Third Party";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      sourceStr = "Application";
      break;
    case GL_DEBUG_SOURCE_OTHER:
      sourceStr = "Other";
      break;
  }

  // 解析错误类型
  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      typeStr = "Error";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      typeStr = "Deprecated Behavior";
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      typeStr = "Undefined Behavior";
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      typeStr = "Portability";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      typeStr = "Performance";
      break;
    case GL_DEBUG_TYPE_MARKER:
      typeStr = "Marker";
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      typeStr = "Push Group";
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      typeStr = "Pop Group";
      break;
    case GL_DEBUG_TYPE_OTHER:
      typeStr = "Other";
      break;
  }

  // 解析严重程度
  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      severityStr = "High";
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      severityStr = "Medium";
      break;
    case GL_DEBUG_SEVERITY_LOW:
      severityStr = "Low";
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      severityStr = "Notification";
      break;
  }

  // 输出格式化错误信息
  LOG_ERROR("OpenGL Debug - Source: {}, Type: {}, ID: {}, Severity: {} - {}",
            sourceStr,
            typeStr,
            id,
            severityStr,
            message);

  return;
}

void OpenGLWindow::initializeOpenGLDebugging()
{
  // 检查是否支持调试输出
  GLint flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

  if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
    // 启用调试输出
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);  // 确保回调与错误发生同步
    glDebugMessageCallback(OpenGLWindow::openGLErrorCallback, nullptr);

    // 控制输出级别：启用所有错误和警告，禁用通知
    glDebugMessageControl(
        GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_TRUE);
    glDebugMessageControl(
        GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);

    LOG_INFO("OpenGL debug output enabled");
  }
  else {
    LOG_WARN("OpenGL debug context not available, falling back to basic error checking");
  }
}

void OpenGLWindow::cleanupOpenGLDebugging()
{
  glDebugMessageCallback(nullptr, nullptr);
  glDisable(GL_DEBUG_OUTPUT);
}
}  // namespace mite