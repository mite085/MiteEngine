#include "ui_system.h"
#include "ui_imgui_backend/ui_imgui_backend.h"
#include "ui_localization_json.h"
#include "window.h"
#include "renderer.h"

namespace mite {

UISystem::UISystem(Renderer &renderer, Window &window)
    : m_Visible(true), m_Renderer(renderer), m_Window(window)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite UI ImGui Backend");
  m_Logger->info("Initializing UI ImGui Backend");

  // 初始化后端
  if (!InitializeBackend()) {
    m_Logger->error("UI Backend Initialize FAILED!");
  }

  // 初始化Style Manager
  m_StyleManager = std::make_unique<UIStyleManager>();
  m_StyleManager->Initialize();

  // 初始化翻译系统
  UILocalization::Get();

  // 发布初始化完成事件
  EventBus::Publish<UIInitializedEvent>(UIInitializedEvent());
}

UISystem::~UISystem()
{
  // 发布关闭事件
  EventBus::Publish<UIShutdownEvent>(UIShutdownEvent());

  // 清理面板
  m_Panels.clear();

  // 关闭后端
  if (m_Backend) {
    m_Backend->Shutdown();
    m_Backend.reset();
  }

  m_Logger->info("UI System Shut Down");
}

void UISystem::Update(float deltaTime)
{
  if (!m_Visible) {
    return;
  }

  // 更新所有面板
  for (auto &[id, panel] : m_Panels) {
    if (panel->IsVisible()) {
      panel->Update(deltaTime);
    }
  }
}

void UISystem::BeginFrame()
{
  if (!m_Visible) {
    return;
  }

  if (m_Backend) {
    m_Backend->BeginFrame();
  }
}

void UISystem::Render()
{
  if (!m_Visible) {
    return;
  }

  if (m_Backend) {
    m_Backend->Render();
  }

  // 渲染所有可见面板
  for (auto &[id, panel] : m_Panels) {
    if (panel->IsVisible()) {
      panel->Render();
    }
  }
}

void UISystem::EndFrame()
{
  if (!m_Visible) {
    return;
  }

  if (m_Backend) {
    m_Backend->EndFrame();
  }
}

void UISystem::ProcessInputEvent(Event &event)
{
  if (!m_Visible) {
    return;
  }

  if (m_Backend) {
    m_Backend->ProcessInputEvent(event);
  }
}

std::shared_ptr<UIPanel> UISystem::CreatePanel(const std::string &name)
{
  auto panel = std::make_shared<UIPanel>(name);
  m_Panels[panel->GetID()] = panel;

  // 发布面板创建事件
  EventBus::Publish<PanelOpenedEvent>(PanelOpenedEvent(panel->GetID(), name));

  return panel;
}

void UISystem::DestroyPanel(UUID panelId)
{
  auto it = m_Panels.find(panelId);
  if (it != m_Panels.end()) {
    // 发布面板关闭事件
    EventBus::Publish<PanelClosedEvent>(PanelClosedEvent(panelId, it->second->GetName()));
    m_Panels.erase(it);
  }
}

std::shared_ptr<UIPanel> UISystem::GetPanel(UUID panelId) const
{
  auto it = m_Panels.find(panelId);
  return it != m_Panels.end() ? it->second : nullptr;
}

void UISystem::SetPanelVisible(UUID panelId, bool visible)
{
  if (auto panel = GetPanel(panelId)) {
    panel->SetVisible(visible);
    // 发布可见性变更事件
    EventBus::Publish<UIVisibilityChangedEvent>(
        UIVisibilityChangedEvent(panelId, "Panel", visible));
  }
}

bool UISystem::IsVisible() const
{
  return m_Visible;
}

void UISystem::SetVisible(bool visible)
{
  m_Visible = visible;
}

bool UISystem::InitializeBackend()
{
  // 目前只实现ImGui后端
  m_Backend = std::make_unique<ImGuiBackend>();

  if (!m_Backend->Initialize(m_Window.GetNativeWindow())) {
    m_Logger->error("ImGui Backend Initialize FAILED !");
    return false;
  }

  // 设置显示尺寸
  m_Backend->SetDisplaySize(m_Window.GetWidth(), m_Window.GetHeight());

  m_Logger->info("ImGui Backend Initialize Successed: {}", m_Backend->GetBackendName());
  return true;
}


}  // namespace mite
