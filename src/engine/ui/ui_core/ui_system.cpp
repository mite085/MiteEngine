#include "ui_system.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"
#include "GLFW/glfw3.h"

namespace mite {
UISystem::UISystem()
{
  // 初始化LOGGER
  m_logger = mite::LoggerSystem::CreateModuleLogger("Mite Engine UI");
  m_logger->info("Create logger for user interface");

  // 订阅EventBus中的输入事件，按照EventCategory大类订阅，由ProcessEvent分发
  m_EventHandlerID = EventBus::Get().SubscribeByCategory(EventCategory::EVENT_CATEGORY_INPUT,
                                                         [this](Event &e) { ProcessEvent(e); });
}
UISystem::~UISystem()
{  
  // 取消订阅EventBus
  EventBus::Get().Unsubscribe(m_EventHandlerID);
}

void UISystem::Init(GLFWwindow *window)
{

  // 初始化ImGui上下文
  IMGUI_CHECKVERSION();
  m_imguiContext = ImGui::CreateContext();
  ImGui::SetCurrentContext(m_imguiContext);

  // 配置ImGui
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // 键盘导航
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // 停靠功能
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // 多视口支持

  // 设置默认样式
  ImGui::StyleColorsDark();

  // 调整多视口样式
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // 初始化平台/渲染后端
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 410");

  m_logger->info("UI System initialized");
}

void UISystem::Shutdown()
{
  // 按顺序销毁面板
  for (auto &[name, panel] : m_panels) {
    panel->onDetach();
  }
  m_panels.clear();
  m_panelOrder.clear();

  // 关闭ImGui后端
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  // 销毁上下文
  if (m_imguiContext) {
    ImGui::DestroyContext(m_imguiContext);
    m_imguiContext = nullptr;
  }

  m_logger->info("UI System shutdown");
}

void UISystem::BeginFrame()
{
  // 开始新一帧的ImGui
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  m_frameStarted = true;

  // 设置主停靠空间
  ImGui::DockSpaceOverViewport(ImGui::GetID("MainDockspace"),  // 自定义dockspace ID
                               ImGui::GetMainViewport(),
                               ImGuiDockNodeFlags_PassthruCentralNode  // 常用标志
  );

  // 启用Gizmo的BeginFrame
  ImGuizmo::BeginFrame();
}

void UISystem::Update(float deltaTime)
{
  if (!m_frameStarted)
    BeginFrame();

  // 更新所有可见面板
  for (const auto &name : m_panelOrder) {
    if (auto &panel = m_panels[name]; panel->isVisible()) {
      panel->onUpdate(deltaTime);
    }
  }
}

void UISystem::EndFrame()
{
  if (!m_frameStarted)
    return;

  // 渲染所有可见面板
  for (const auto &name : m_panelOrder) {
    if (auto &panel = m_panels[name]; panel->isVisible()) {
      panel->onRender();
    }
  }

  // 完成ImGui渲染
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // 多视口支持
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup);
  }

  m_frameStarted = false;
}

void UISystem::RegisterPanel(const std::string &name, std::shared_ptr<UIPanel> panel)
{
  if (m_panels.find(name) != m_panels.end()) {
    LOG_WARN("UI Panel '{}' already registered, replacing", name);
    UnregisterPanel(name);
  }

  m_panels[name] = panel;
  m_panelOrder.push_back(name);
  panel->onAttach();

  LOG_DEBUG("Registered UI Panel: {}", name);
}

void UISystem::UnregisterPanel(const std::string &name)
{
  if (auto it = m_panels.find(name); it != m_panels.end()) {
    it->second->onDetach();
    m_panels.erase(it);

    // 从渲染顺序中移除
    m_panelOrder.erase(std::remove(m_panelOrder.begin(), m_panelOrder.end(), name),
                       m_panelOrder.end());

    LOG_DEBUG("Unregistered UI Panel: {}", name);
  }
}

std::shared_ptr<UIPanel> UISystem::GetPanel(const std::string &name)
{
  if (auto it = m_panels.find(name); it != m_panels.end()) {
    return it->second;
  }
  return nullptr;
}

bool UISystem::ProcessEvent(Event &event)
{
  // 从后往前处理面板事件（保证顶层面板优先）
  for (auto it = m_panelOrder.rbegin(); it != m_panelOrder.rend(); ++it) {
    if (auto &panel = m_panels[*it]; panel->isVisible() && panel->onEvent(event)) {
      return true;
    }
  }
  return false;
}

void UISystem::SetPanelVisible(const std::string &name, bool visible)
{
  if (auto panel = GetPanel(name)) {
    panel->setVisible(visible);
  }
}

void UISystem::TogglePanelVisible(const std::string &name)
{
  if (auto panel = GetPanel(name)) {
    panel->setVisible(!panel->isVisible());
  }
}
};  // namespace mite