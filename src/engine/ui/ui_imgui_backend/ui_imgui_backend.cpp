#include "ui_imgui_backend.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace mite {
ImGuiBackend::ImGuiBackend()
    : m_Window(nullptr),
      m_MouseCaptured(false),
      m_MouseCursorVisible(true),
      m_StyleAdapter(std::make_unique<ImGuiStyleAdapter>())
{
}

bool ImGuiBackend::Initialize(void *glfwWindow)
{
  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite UI ImGui Backend");
  m_Logger->info("Initializing UI ImGui Backend");

  // 获取窗口句柄
  SetWindow(reinterpret_cast<GLFWwindow *>(glfwWindow));
  if (!m_Window) {
    m_Logger->error("ImGuiBackend: GLFW window not set");
    return false;
  }

  // 初始化ImGui上下文
  if (!InitializeImGuiContext()) {
    return false;
  }

  // 初始化平台后端（使用窗口句柄）
  if (!InitializePlatformBackend()) {
    return false;
  }

  // 初始化渲染器后端
  if (!InitializeRendererBackend()) {
    return false;
  }

  // 初始化样式适配器
  m_StyleAdapter->Initialize();

  // 初始化字体为中文
  ImGuiFontManager::LoadFonts();
  ImGuiFontManager::SetLanguageFont("zh-CN");

  m_Logger->info("ImGuiBackend initialized successfully");
  return true;
}

void ImGuiBackend::Shutdown()
{
  m_Logger->debug("ImGuiBackend shutdown started");

  // 1. 清理样式适配器
  if (m_StyleAdapter) {
    m_StyleAdapter->Shutdown();
  }

  // 2. 销毁渲染器后端（OpenGL3）
  if (ImGui::GetCurrentContext()) {
    ImGui_ImplOpenGL3_Shutdown();
  }

  // 3. 销毁平台后端（GLFW）
  if (ImGui::GetCurrentContext()) {
    ImGui_ImplGlfw_Shutdown();
  }

  // 4. 最后销毁ImGui上下文
  if (ImGui::GetCurrentContext()) {
    ImGui::DestroyContext();
  }

  // 5. 重置状态变量
  m_Window = nullptr;
  m_MouseCaptured = false;
  m_MouseCursorVisible = true;

  m_Logger->info("ImGuiBackend shutdown completed");
}

void ImGuiBackend::BeginFrame()
{
  // 开始ImGui帧
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // 创建一个覆盖整个视口的停靠空间
  //ImGui::DockSpaceOverViewport();

  // 使用Time模块设置DeltaTime
  ImGui::GetIO().DeltaTime = Time::DeltaTime();

  static bool opt_fullscreen = true;
  static bool opt_padding = false;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
  // 设置全屏停靠窗口
  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Mite DockSpace", nullptr, window_flags);
  ImGui::PopStyleVar();
  // 菜单栏
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Options")) {
      ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
      ImGui::MenuItem("Padding", NULL, &opt_padding);
      ImGui::Separator();
      if (ImGui::MenuItem(
              "Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))
      {
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  // 停靠空间
  ImGuiID dockspace_id = ImGui::DockSpace(
      ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f), dockspace_flags);
  ImGui::End();

}

void ImGuiBackend::EndFrame()
{
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // 多视口支持
  // （Copy from imgui/examples/example_glfw_opengl3）
  //
  // Update and Render additional Platform Windows
  // (Platform functions may change the current OpenGL context, so we save/restore it to make it
  // easier to paste this code elsewhere.
  //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}

void ImGuiBackend::SetMouseCaptured(bool captured)
{
  m_MouseCaptured = captured;
  if (m_Window) {
    glfwSetInputMode(m_Window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
  }
}

bool ImGuiBackend::IsMouseCaptured() const
{
  return m_MouseCaptured;
}

void ImGuiBackend::SetMouseCursorVisible(bool visible)
{
  m_MouseCursorVisible = visible;
  if (m_Window) {
    glfwSetInputMode(m_Window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
  }
}

bool ImGuiBackend::IsMouseCursorVisible() const
{
  return m_MouseCursorVisible;
}

void ImGuiBackend::ApplyUIStyle(std::shared_ptr<UIStyle> newStyle)
{
  m_StyleAdapter->ApplyUIStyle(newStyle);
}

void ImGuiBackend::ApplyLanguaged(const std::string &oldLanguage, const std::string &newLanguage)
{
  // 切换字体
  if (ImGuiFontManager::SetLanguageFont(newLanguage))
    m_Logger->info("Language changed from: {} to: {}", oldLanguage, newLanguage);
  else
    m_Logger->error("Language change FALIED from: {} to: {}", oldLanguage, newLanguage);
}

void ImGuiBackend::SetWindow(GLFWwindow *window)
{
  m_Window = window;
}

GLFWwindow *ImGuiBackend::GetWindow() const
{
  return m_Window;
}

bool ImGuiBackend::InitializeImGuiContext()
{
  // 创建ImGui上下文
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // 启用键盘导航
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // 启用游戏手柄导航
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // 启用停靠
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // 启用多视口

  // 设置ini文件位置
  // io.IniFilename = "imgui.ini";

  return true;
}

bool ImGuiBackend::InitializePlatformBackend()
{
  if (!ImGui_ImplGlfw_InitForOpenGL(m_Window, true)) {
    m_Logger->error("Failed to initialize ImGui GLFW backend");
    return false;
  }
  return true;
}

bool ImGuiBackend::InitializeRendererBackend()
{
  // 版本设定应当与GLFWWindow中设定的glfwWindowHint一致
  // （原则上应当将版本号作为参数传入）
  const char *glsl_version = "#version 430";
  if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
    m_Logger->error("Failed to initialize ImGui OpenGL3 backend");
    return false;
  }
  return true;
}
}  // namespace mite