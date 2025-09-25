#include "ui_system.h"
#include "ui_imgui_backend/ui_imgui_backend.h"
#include "ui_localization_json.h"
#include "window.h"
#include "renderer.h"

namespace mite {

UISystem::UISystem()
    : m_Visible(true)
{
  m_Logger = mite::LoggerSystem::CreateModuleLogger("Mite UI System");
  m_Logger->info("Initializing UI System");
}

void UISystem::Initialize(void *nativeWindow)
{  
    // 初始化后端
  if (!InitializeBackend(nativeWindow)) {
    m_Logger->error("UI Backend Initialize FAILED!");
  }

  // 初始化Style Manager，并将CurrentStyle作用于后端
  m_StyleManager = std::make_unique<UIStyleManager>();
  m_StyleManager->Initialize();

  // 发布初始化完成事件
  EventBus::Publish<UISystemInitializedEvent>(UISystemInitializedEvent());
}

void UISystem::Shutdown()
{ 
  // 发布关闭事件
  EventBus::Publish<UISystemShutdownEvent>(UISystemShutdownEvent());

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
    // 绘制菜单栏

    // 设定停靠空间（Editor专用）

    // 渲染所有可见面板
    for (auto &[id, panel] : m_Panels) {
      if (panel->IsVisible())
        panel->RenderPanel();
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

void UISystem::RegisterPanel(std::shared_ptr<UIPanel> panel)
{
  // 需要检查UI的ID
  if (m_Panels.find(panel->GetPanelProps().elementId) != m_Panels.end()) {
    m_Logger->error("Cannot Register Existing Panel: name = {}, UUID = {}",
                    panel->GetName(),
                    UUIDGenerator::UUIDToString(panel->GetPanelProps().elementId));
  }

  // 注册进哈希表
  m_Panels[panel->GetPanelProps().elementId] = panel;

  // 发布面板创建事件
  EventBus::Publish<PanelOpenedEvent>(
      PanelOpenedEvent(panel->GetPanelProps().elementId, panel->GetName()));
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

bool UISystem::InitializeBackend(void *nativeWindow)
{
  // 目前只实现ImGui后端
  m_Backend = std::make_unique<ImGuiBackend>();

  if (!m_Backend->Initialize(nativeWindow)) {
    m_Logger->error("ImGui Backend Initialize FAILED !");
    return false;
  }

  m_Logger->info("ImGui Backend Initialize Successed: {}", m_Backend->GetBackendName());
  return true;
}


}  // namespace mite
