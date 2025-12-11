#include "ui_system.h"
#include "ui_imgui_backend/ui_imgui_backend.h"
#include "ui_localization_json.h"
#include "window.h"

namespace mite {
UISystem::UISystem()
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

  // 初始化Menu菜单
  m_Menu = std::make_unique<UIMenu>();

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
  // 更新所有面板
  for (auto &panel : m_Panels) {
    if (panel->IsVisible()) {
      panel->Update(deltaTime);
    }
  }
}

void UISystem::BeginFrame()
{
  if (m_Backend) {
    m_Backend->BeginFrame([this]() { m_Menu->RenderMenuBar(); });
  }
}

void UISystem::Render()
{
  if (m_Backend) {
    // 渲染所有可见面板
    for (auto &panel : m_Panels) {
      if (panel->IsVisible())
        panel->RenderPanel();
    }
  }
}

void UISystem::EndFrame()
{
  if (m_Backend) {
    m_Backend->EndFrame();
  }
}

void UISystem::RegisterPanel(std::shared_ptr<UIPanel> panel)
{
  // 需要检查UI的ID
  if (m_Panels.find(panel) != m_Panels.end()) {
    m_Logger->error("Cannot Register Existing Panel: name = {}", panel->GetName());
  }

  // 注册进哈希表
  m_Panels.insert(panel);

  // 发布面板创建事件
  EventBus::Publish<PanelOpenedEvent>(PanelOpenedEvent(panel));
}

void UISystem::DestroyPanel(std::shared_ptr<UIPanel> panel)
{
  auto it = m_Panels.find(panel);
  if (it != m_Panels.end()) {
    // 发布面板关闭事件
    EventBus::Publish<PanelClosedEvent>(PanelClosedEvent(panel));
    m_Panels.erase(it);
  }
}
bool UISystem::InitializeBackend(void *nativeWindow)
{
  // 目前只实现ImGui后端
  m_Backend = std::make_unique<ImGuiBackend>();

  if (!m_Backend->Initialize(nativeWindow)) {
    m_Logger->error("ImGui Backend Initialize FAILED !");
    return false;
  }

  m_Logger->info("ImGui Backend Initialize Successed");
  return true;
}
}  // namespace mite