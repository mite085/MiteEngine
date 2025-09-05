#include "ui_imgui_backend.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace mite {

ImGuiBackend::ImGuiBackend()
    : m_Window(nullptr),
      m_MouseCaptured(false),
      m_MouseCursorVisible(true),
      m_DisplaySize(1280, 720),
      m_FramebufferScale(1.0f, 1.0f),
      m_StyleAdapter(std::make_unique<ImGuiStyleAdapter>()),
      m_InputAdapter(std::make_unique<ImGuiInputAdapter>())
{
}

ImGuiBackend::~ImGuiBackend()
{
  Shutdown();
}

bool ImGuiBackend::Initialize()
{
  // 创建日志系统
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite UI ImGui Backend");
  m_Logger->info("Initializing UI ImGui Backend");

  if (!m_Window) {
    m_Logger->error("ImGuiBackend: GLFW window not set");
    return false;
  }

  // 初始化ImGui上下文
  if (!InitializeImGuiContext()) {
    return false;
  }

  // 初始化平台后端
  if (!InitializePlatformBackend()) {
    return false;
  }

  // 初始化渲染器后端
  if (!InitializeRendererBackend()) {
    return false;
  }

  // 初始化样式适配器
  m_StyleAdapter->Initialize();

  // 初始化输入适配器
  m_InputAdapter->Initialize();

  // 初始化字体
  ImGuiFontManager::LoadFonts();

  // 初始化本地化渲染器
  ImGuiLocalizationRenderer::Initialize();

  // 订阅语言变更事件
  m_SubscriptionGroup.Subscribe<LanguageChangedEvent>(BIND_DISPATCH_FN(OnLanguageChanged));

  // 应用默认样式
  ApplyDarkStyle();

  m_Logger->info("ImGuiBackend initialized successfully");
  return true;
}

void ImGuiBackend::Shutdown()
{
  m_Logger->debug("ImGuiBackend shutdown started");

  // 1. 清理本地化相关资源与适配器
  ImGuiLocalizationRenderer::Shutdown();
  if (m_InputAdapter) {
    m_InputAdapter->Shutdown();
  }
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
  m_DisplaySize = {0, 0};
  m_FramebufferScale = {1.0f, 1.0f};

  m_Logger->info("ImGuiBackend shutdown completed");
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
  const char *glsl_version = "#version 130";
  if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
    m_Logger->error("Failed to initialize ImGui OpenGL3 backend");
    return false;
  }
  return true;
}

void ImGuiBackend::BeginFrame()
{
  // 开始ImGui帧
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // 使用自定义Time模块设置DeltaTime
  ImGui::GetIO().DeltaTime = Time::DeltaTime();

  // 更新显示尺寸（委托给输入适配器）
  m_InputAdapter->UpdateDisplaySize(GetWindow());
  m_DisplaySize = m_InputAdapter->GetDisplaySize();

  // 更新帧缓冲缩放
  m_InputAdapter->UpdateFramebufferScale(GetWindow());
  m_FramebufferScale = m_InputAdapter->GetFramebufferScale();
}

void ImGuiBackend::EndFrame()
{
  ImGui::Render();
}

void ImGuiBackend::ProcessInputEvent(Event &event)
{
  // 委托给输入适配器处理
  m_InputAdapter->ProcessEvent(event);
}

void ImGuiBackend::OnLanguageChanged(LanguageChangedEvent &event) {
  // 切换字体
  ImGuiFontManager::SetLanguageFont(event.GetNewLanguage());

  m_Logger->info("Language changed to: {}", event.GetNewLanguage());
}

void ImGuiBackend::SetDisplaySize(int width, int height)
{
  m_DisplaySize = {width, height};
  if (m_Window) {
    glfwSetWindowSize(m_Window, width, height);
  }
}

glm::ivec2 ImGuiBackend::GetDisplaySize() const
{
  return m_DisplaySize;
}

void ImGuiBackend::SetFramebufferScale(float scaleX, float scaleY)
{
  m_FramebufferScale = {scaleX, scaleY};
  ImGui::GetIO().DisplayFramebufferScale = ImVec2(scaleX, scaleY);
}

glm::vec2 ImGuiBackend::GetFramebufferScale() const
{
  return m_FramebufferScale;
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

void ImGuiBackend::CreateDeviceObjects()
{
  ImGui_ImplOpenGL3_CreateDeviceObjects();
}

void ImGuiBackend::DestroyDeviceObjects()
{
  ImGui_ImplOpenGL3_DestroyDeviceObjects();
}

void ImGuiBackend::Render()
{
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // 多视口支持
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}

const char *ImGuiBackend::GetBackendName() const
{
  return "ImGui (OpenGL3 + GLFW)";
}

void ImGuiBackend::SetWindow(GLFWwindow *window)
{
  m_Window = window;
}

GLFWwindow *ImGuiBackend::GetWindow() const
{
  return m_Window;
}

void ImGuiBackend::ApplyStyle(const std::string &styleName)
{
  m_StyleAdapter->ApplyStyle(styleName);
}
void ImGuiBackend::ApplyDarkStyle()
{
  m_StyleAdapter->ApplyDarkStyle();
}
void ImGuiBackend::ApplyLightStyle()
{
  m_StyleAdapter->ApplyLightStyle();
}
void ImGuiBackend::ApplyCustomStyle(const ImGuiStyle &style)
{
  m_StyleAdapter->ApplyCustomStyle(style);
}

void ImGuiBackend::Shutdown()
{
  ImGuiLocalizationRenderer::Shutdown();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  m_Logger->info("ImGuiBackend shutdown");
}

}  // namespace mite
